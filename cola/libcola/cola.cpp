/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libcola - A library providing force-directed network layout using the 
 *           stress-majorization method subject to separation constraints.
 *
 * Copyright (C) 2006-2008  Monash University
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library in the file LICENSE; if not, 
 * write to the Free Software Foundation, Inc., 59 Temple Place, 
 * Suite 330, Boston, MA  02111-1307  USA
 *
*/

#include "commondefs.h"
#include <cmath>
#include "cola.h"
#include "conjugate_gradient.h"
#include "straightener.h"
#include "shortest_paths.h"
#include "cluster.h"

using namespace std;
using namespace vpsc;
using straightener::generateClusterBoundaries;

namespace cola {

TestConvergence defaultTest(0.0001,100);

ConstrainedMajorizationLayout
::ConstrainedMajorizationLayout(
        vector<Rectangle*>& rs,
        vector<Edge> const & es,
        RootCluster *clusterHierarchy,
        double const idealLength,
        std::valarray<double> const * eweights,
        TestConvergence& done,
        PreIteration* preIteration)
    : n(rs.size()),
      lap2(valarray<double>(n*n)), 
      Dij(valarray<double>(n*n)),
      tol(1e-7), done(done), preIteration(preIteration),
      X(valarray<double>(n)), Y(valarray<double>(n)),
      stickyNodes(false), 
      startX(valarray<double>(n)), startY(valarray<double>(n)),
      constrainedLayout(false),
      nonOverlappingClusters(false),
      clusterHierarchy(clusterHierarchy), linearConstraints(NULL),
      gpX(NULL), gpY(NULL),
      ccsx(NULL), ccsy(NULL),
      unsatisfiableX(NULL), unsatisfiableY(NULL),
      avoidOverlaps(None),
      straightenEdges(NULL),
      bendWeight(0.1), potBendWeight(0.1),
      xSkipping(true),
      scaling(true),
      externalSolver(false),
      majorization(true)
{
    boundingBoxes.resize(rs.size());
    copy(rs.begin(),rs.end(),boundingBoxes.begin());

    done.reset();

    assert(!eweights||eweights->size()==es.size()); 
    assert(!straightenEdges||straightenEdges->size()==es.size());

    double** D=new double*[n];
    for(unsigned i=0;i<n;i++) {
        D[i]=new double[n];
    }
    shortest_paths::johnsons(n,D,es,eweights);
    //shortest_paths::neighbours(n,D,es,eweights);
    edge_length = idealLength;
    if(clusterHierarchy) {
        for(Clusters::const_iterator i=clusterHierarchy->clusters.begin();
                i!=clusterHierarchy->clusters.end();i++) {
            Cluster *c=*i;
            for(vector<unsigned>::iterator j=c->nodes.begin();j!=c->nodes.end();j++) {
                for(vector<unsigned>::iterator k=c->nodes.begin();k!=c->nodes.end();k++) {
                    unsigned a=*j, b=*k;
                    if(a==b) continue;
                    D[a][b]/=c->internalEdgeWeightFactor;
                }
            }
        }
    }
    // Lij_{i!=j}=1/(Dij^2)
    //
    for(unsigned i = 0; i<n; i++) {
        X[i]=rs[i]->getCentreX();
        Y[i]=rs[i]->getCentreY();
        double degree = 0;
        for(unsigned j=0;j<n;j++) {
            double d = edge_length * D[i][j];
            Dij[i*n + j] = d;
            if(i==j) continue;
            double lij=0;
            if(d!=0 && !isinf(d)) {
                lij=1./(d*d);
            }
            degree += lap2[i*n + j] = lij;
        }
        lap2[i*n + i]=-degree;
        delete [] D[i];
    }
    //GradientProjection::dumpSquareMatrix(Dij);
    delete [] D;
}
// stickyNodes adds a small force attracting nodes 
// back to their starting positions
void ConstrainedMajorizationLayout::setStickyNodes(
        const double stickyWeight, 
        valarray<double> const & startX,
        valarray<double> const & startY) {
    assert( startX.size()==n && startY.size()==n);
    stickyNodes = true;
    // not really constrained but we want to use GP solver rather than 
    // ConjugateGradient
    constrainedLayout = true; 
    this->stickyWeight=stickyWeight;
    this->startX = startX;
    this->startY = startY;
    for(unsigned i = 0; i<n; i++) {
        lap2[i*n+i]-=stickyWeight;
    }
}

void ConstrainedMajorizationLayout::majorize(
        valarray<double> const & Dij, GradientProjection* gp, 
        valarray<double>& coords,
        valarray<double> const & startCoords)
{
    double L_ij,dist_ij,degree;
    /* compute the vector b */
    /* multiply on-the-fly with distance-based laplacian */
    valarray<double> b(n);
    for (unsigned i = 0; i < n; i++) {
        b[i] = degree = 0;
        for (unsigned j = 0; j < n; j++) {
            if (j == i) continue;
            dist_ij = euclidean_distance(i, j);
            /* skip zero distances */
            if (dist_ij > 1e-30 && Dij[i*n+j] > 1e-30 && Dij[i*n+j] < 1e10) {
                /* calculate L_ij := w_{ij}*d_{ij}/dist_{ij} */
                L_ij = 1.0 / (dist_ij * Dij[i*n+j]);
                degree -= L_ij;
                b[i] += L_ij * coords[j];
            }
        }
        if(stickyNodes) {
            //double l = startCoords[i]-coords[i];
            //l/=10.;
            //b[i]-=stickyWeight*(coords[i]+l);
            b[i] -= stickyWeight*startCoords[i];
        }
        b[i] += degree * coords[i];
        assert(!std::isnan(b[i]));
    }
    if(constrainedLayout) {
        //printf("GP iteration...\n");
        gp->solve(b,coords);
    } else {
        //printf("CG iteration...\n");
        conjugate_gradient(lap2, coords, b, n, tol, n);
    }
    moveBoundingBoxes();
}
void ConstrainedMajorizationLayout::newton(
        valarray<double> const & Dij, GradientProjection* gp, 
        valarray<double>& coords,
        valarray<double> const & startCoords)
{
    /* compute the vector b */
    /* multiply on-the-fly with distance-based laplacian */
    valarray<double> b(n);
    valarray<double> A(n*n);
    for (unsigned i = 0; i < n; i++) {
        b[i] = 0;
        double Aii = 0;
        for (unsigned j = 0; j < n; j++) {
            if (j == i) continue;
            double d = Dij[i*n+j];
            double l = euclidean_distance(i,j);
            double dx = coords[i]-coords[j];
            double dy2 = l*l - dx*dx;
            /* skip zero distances */
            if (l > 1e-30 
                    && d > 1e-30 && d < 1e10) {
                if(d>80 && l > d) continue;
                b[i]+=dx*(l-d)/(l*d*d);
                Aii-=A[i*n+j]=(d*dy2/(l*l*l)-1)/(d*d);
            }
        }
        A[i*n+i]=Aii;
    }
    if(constrainedLayout) {
        //printf("GP iteration...\n");
        gp->solve(b,coords);
    } else {
        //printf("CG iteration...\n");
        /*
        unsigned N=n-1;
        valarray<double> b2(N);
        valarray<double> A2(N*N);
        valarray<double> x(N);
        for(unsigned i=0;i<N;i++) {
            b2=b[i];
            x=coords[i];
            for(unsigned j=0;j<N;j++) {
                A2[i*N+j]=A[i*n+j];
            }
        }
        conjugate_gradient(A2, x, b2, N, tol, N);
        */
        //valarray<double> x=coords;
        //x-=x.sum()/n;
        //conjugate_gradient(A, x, b, n, tol, n);
        //double stepsize=0.5;
        valarray<double> x=b;
        // stepsize = g.g / (g A g)
        double numerator = 0;
        for(unsigned i=0;i<n;i++) {
            numerator+=x[i]*x[i];
        }
        double denominator = 0;
        for(unsigned i=0;i<n;i++) {
            double r=0;
            for(unsigned j=0;j<n;j++) {
                r+=A[i*n+j]*x[j];
            }
            denominator+=r*x[i];
        }
        double stepsize=numerator/(2*denominator);
        double oldstress=compute_stress(Dij);
        valarray<double> oldcoords=coords;
        while(stepsize>0.00001) {
            coords=oldcoords-stepsize*x;
            double stress=compute_stress(Dij);
                printf("  stress=%f, stepsize=%f\n",stress,stepsize);
            if(oldstress>=stress) {
                break;
            }
            coords=oldcoords;
            stepsize*=0.5;
        }
    }
    moveBoundingBoxes();
}
inline double ConstrainedMajorizationLayout
::compute_stress(valarray<double> const &Dij) {
    double sum = 0, d, diff;
    for (unsigned i = 1; i < n; i++) {
        for (unsigned j = 0; j < i; j++) {
            d = Dij[i*n+j];
            if(!isinf(d)&&d!=numeric_limits<double>::max()) {
                diff = d - euclidean_distance(i,j);
                if(d>80&&diff<0) continue;
                sum += diff*diff / (d*d);
            }
        }
        if(stickyNodes) {
            double l = startX[i]-X[i];
            sum += stickyWeight*l*l;
            l = startY[i]-Y[i];
            sum += stickyWeight*l*l;
        }
    }
    //printf("stress=%f\n",sum);
    return sum;
}

void ConstrainedMajorizationLayout::run(bool x, bool y) {
    if(constrainedLayout) {
        vector<vpsc::Rectangle*>* pbb = boundingBoxes.empty()?NULL:&boundingBoxes;
        SolveWithMosek mosek = Off;
        if(externalSolver) mosek=Outer;
        // scaling doesn't currently work with straighten edges because sparse
        // matrix used with dummy nodes is not properly scaled at the moment.
        if(straightenEdges) setScaling(false);
        gpX=new GradientProjection(
            HORIZONTAL,&lap2,tol,100,ccsx,unsatisfiableX,
            avoidOverlaps,clusterHierarchy,pbb,scaling,mosek);
        gpY=new GradientProjection(
            VERTICAL,&lap2,tol,100,ccsy,unsatisfiableY,
            avoidOverlaps,clusterHierarchy,pbb,scaling,mosek);
    }
    if(n>0) do {
        // to enforce clusters with non-intersecting, convex boundaries we
        // could create cluster boundaries here with chains of dummy nodes (a
        // dummy node for each vertex of the convex hull) connected by dummy
        // straightenEdges and we'd then continue on to straightenEdges below.
        // This should work assuming we already have a feasible (i.e. non
        // overlapping cluster) state.  The former could be enforced by an
        // earlier stage involving simple rectangular cluster boundaries.
        vector<straightener::Edge*> cedges;
        if(!straightenEdges && nonOverlappingClusters) {
            straightenEdges = &cedges;
        }
        if(preIteration) {
            if ((*preIteration)()) {
                for(vector<Lock>::iterator l=preIteration->locks.begin();
                        l!=preIteration->locks.end();l++) {
                    unsigned id=l->getID();
                    double x=l->pos(HORIZONTAL), y=l->pos(VERTICAL);
                    X[id]=x;
                    Y[id]=y;
                    if(stickyNodes) {
                        startX[id]=x;
                        startY[id]=y;
                    }
                    boundingBoxes[id]->moveCentre(x,y);
                    if(constrainedLayout) {
                        gpX->fixPos(id,X[id]); 
                        gpY->fixPos(id,Y[id]);
                    }
                }
            } else { break; }
        }
        /* Axis-by-axis optimization: */
        if(straightenEdges) {
            if(x) straighten(*straightenEdges,HORIZONTAL);
            if(y) straighten(*straightenEdges,VERTICAL);
        } else {
            if(majorization) {
                if(x) majorize(Dij,gpX,X,startX);
                if(y) majorize(Dij,gpY,Y,startY);
            } else {
                if(x) newton(Dij,gpX,X,startX);
                if(y) newton(Dij,gpY,Y,startY);
            }
        }
        if(clusterHierarchy) {
            for(Clusters::iterator c=clusterHierarchy->clusters.begin();
                    c!=clusterHierarchy->clusters.end();c++) {
                (*c)->computeBoundary(boundingBoxes);
            }
        }
        if(preIteration && constrainedLayout) {
            for(vector<Lock>::iterator l=preIteration->locks.begin();
                    l!=preIteration->locks.end();l++) {
                gpX->unfixPos(l->getID());
                gpY->unfixPos(l->getID());
            }
        }
    } while(!done(compute_stress(Dij),X,Y));
}
void ConstrainedMajorizationLayout::straighten(vector<straightener::Edge*>& sedges, Dim dim) {
    GradientProjection * gp;
    valarray<double>* coords;
    valarray<double>* startCoords;
    if(dim==HORIZONTAL) {
        gp=gpX;
        coords=&X;
        startCoords=&startX;
    } else {
        gp=gpY;
        coords=&Y;
        startCoords=&startY;
    }
	vector<straightener::Node*> snodes;
    if(dim==HORIZONTAL) {
        Rectangle::setXBorder(0.0001);
    }
	for (unsigned i=0;i<n;i++) {
		snodes.push_back(new straightener::Node(i,boundingBoxes[i]));
	}
    if(dim==HORIZONTAL) {
        Rectangle::setXBorder(0);
    }
    for (unsigned i=n;i<gp->getNumStaticVars();i++) {
        // insert some dummy nodes
        snodes.push_back(new straightener::Node(i,-100,-100));
    }
    vector<straightener::Cluster*> sclusters;
    
    if(nonOverlappingClusters && clusterHierarchy) {
       generateClusterBoundaries(dim,snodes,sedges,boundingBoxes,
                *clusterHierarchy,sclusters);
    }
    vector<SeparationConstraint*> cs;
    straightener::generateConstraints(dim,snodes,sedges,cs,xSkipping);
    straightener::LinearConstraints linearConstraints;
    for(unsigned i=0;i<sedges.size();i++) {
        sedges[i]->nodePath(snodes,!nonOverlappingClusters);
        vector<unsigned>& path=sedges[i]->path;
        vector<unsigned>& activePath=sedges[i]->activePath;
        // take u and v as the ends of the line
        // unsigned u=path[0];
        // unsigned v=path[path.size()-1];
        double total_length=0;
        for(unsigned j=1;j<path.size();j++) {
            unsigned u=path[j-1], v=path[j];
            total_length+=snodes[u]->euclidean_distance(snodes[v]);
        }
        // keep potential bends straight
        for(unsigned j=1;j<activePath.size();j++) {
            unsigned uj=activePath[j-1], vj=activePath[j];
            unsigned u=path[uj], v=path[vj];
            for(unsigned k=uj+1;k<vj;k++) {
                unsigned b=path[k];
		// might be useful to have greater weight for potential bends than actual bends
                linearConstraints.push_back(new straightener::LinearConstraint(
                            *snodes[u],*snodes[v],*snodes[b],-potBendWeight));
            }
        }
        // straighten actual bends
        for(unsigned j=1;j<activePath.size()-1;j++) {
            unsigned u=path[activePath[j-1]], 
                     b=path[activePath[j]],
                     v=path[activePath[j+1]];
            linearConstraints.push_back(new straightener::LinearConstraint(
                            *snodes[u],*snodes[v],*snodes[b],-bendWeight));
        }
    }
    //std::cout << (dim==HORIZONTAL?"X":"Y") << " snodes.size "<<snodes.size()<< " n="<<n<<std::endl;
    //std::cout << "Generated "<<linearConstraints.size()<< " linear constraints"<<std::endl;
    SparseMap Q(snodes.size());
    for(straightener::LinearConstraints::iterator i=linearConstraints.begin();
           i!= linearConstraints.end();i++) {
        straightener::LinearConstraint* c=*i;
        Q(c->u,c->u)+=c->w*c->duu;
        Q(c->u,c->v)+=c->w*c->duv;
        Q(c->u,c->b)+=c->w*c->dub;
        Q(c->v,c->u)+=c->w*c->duv;
        Q(c->v,c->v)+=c->w*c->dvv;
        Q(c->v,c->b)+=c->w*c->dvb;
        Q(c->b,c->b)+=c->w*c->dbb;
        Q(c->b,c->u)+=c->w*c->dub;
        Q(c->b,c->v)+=c->w*c->dvb;
    }
    double boundaryWeight = 0.0001;
    for(unsigned i=0;i<sclusters.size();i++) {
        // for each cluster boundary chain create an attractive force between
        // each pair of adjacent nodes
        straightener::Cluster* c = sclusters[i];
        for(unsigned j=0;j<c->boundary.size();j++) {
            straightener::Edge* e = c->boundary[j];
            Q(e->startNode,e->endNode)+=boundaryWeight;
            Q(e->endNode,e->startNode)+=boundaryWeight;
            Q(e->startNode,e->startNode)-=boundaryWeight;
            Q(e->endNode,e->endNode)-=boundaryWeight;
        }
    }
    constrainedLayout = true;
    SparseMatrix sparseQ(Q);
    gp->straighten(&sparseQ,cs,snodes);
    //return;
    majorize(Dij,gp,*coords,*startCoords);
    valarray<double> const & r=gp->getFullResult();
    for(unsigned i=0;i<snodes.size();i++) {
        snodes[i]->pos[dim] = r[i];
    }
    for(unsigned i=0;i<sedges.size();i++) {
        sedges[i]->createRouteFromPath(snodes);
        sedges[i]->dummyNodes.clear();
        sedges[i]->path.clear();
    }
    for(unsigned i=0;i<sclusters.size();i++) {
        straightener::Cluster *sc = sclusters[i];
        //sc->updateActualBoundary();
        delete sc;
    }
    for(unsigned i=0;i<cs.size();i++) {
        delete cs[i];
    }
    for(unsigned i=0;i<linearConstraints.size();i++) {
        delete linearConstraints[i];
    }
    for(unsigned i=0;i<snodes.size();i++) {
        delete snodes[i];
    }
}

Rectangle bounds(vector<Rectangle*>& rs) {
    assert(!rs.empty());
    
    double left = rs[0]->getMinX(), right = rs[0]->getMaxX(), 
        top = rs[0]->getMinY(), bottom = rs[0]->getMaxY();
    
    for(unsigned i = 1; i < rs.size(); i++) {
        left = min(left,rs[i]->getMinX());
        right = max(right,rs[i]->getMaxX());
        top = min(top,rs[i]->getMinY());
        bottom = max(bottom,rs[i]->getMaxY());
    }
    return Rectangle(left, right, top, bottom);
}

	void removeClusterOverlap(RootCluster& clusterHierarchy, vpsc::Rectangles& rs, vpsc::Dim dim) {
		if(clusterHierarchy.nodes.size()>0 || clusterHierarchy.clusters.size()>0) {
			vpsc::Variables vars;
			vpsc::Constraints cs;
			for(unsigned i=0;i<rs.size();i++) {
				vars.push_back(new vpsc::Variable(i, rs[i]->getCentreD(dim)));
			}
			clusterHierarchy.computeBoundingRect(rs);
			clusterHierarchy.createVars(dim,rs,vars);
			clusterHierarchy.generateNonOverlapConstraints(dim, cola::Both, rs, vars, cs);
			vpsc::Solver s(vars,cs);
			try {
				s.satisfy();
			} catch(const char* e) {
				cerr << "ERROR from solver in GraphData::removeOverlap : " << e << endl;
			}
			for(unsigned i=0;i<cs.size();++i) {
				if(cs[i]->unsatisfiable) {
					cout << "Unsatisfiable constraint: " << *cs[i] << endl;
				}
			}
			for(unsigned i=0;i<rs.size();i++) {
				rs[i]->moveCentreD(dim,vars[i]->finalPosition);
			}
			for_each(vars.begin(),vars.end(),delete_object());
			for_each(cs.begin(),cs.end(),delete_object());
		}
	}
	void removeClusterOverlapFast(RootCluster& clusterHierarchy, vpsc::Rectangles& rs) {
		removeClusterOverlap(clusterHierarchy, rs, vpsc::HORIZONTAL);
		removeClusterOverlap(clusterHierarchy, rs, vpsc::VERTICAL);
	}

} // namespace cola

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4 :
