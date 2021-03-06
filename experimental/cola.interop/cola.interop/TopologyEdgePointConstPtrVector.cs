/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.36
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

namespace org.adaptagrams.cola {

using System;
using System.Runtime.InteropServices;

public class TopologyEdgePointConstPtrVector : IDisposable, System.Collections.IEnumerable {
  private HandleRef swigCPtr;
  protected bool swigCMemOwn;

  internal TopologyEdgePointConstPtrVector(IntPtr cPtr, bool cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = new HandleRef(this, cPtr);
  }

  internal static HandleRef getCPtr(TopologyEdgePointConstPtrVector obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~TopologyEdgePointConstPtrVector() {
    Dispose();
  }

  public virtual void Dispose() {
    lock(this) {
      if(swigCPtr.Handle != IntPtr.Zero && swigCMemOwn) {
        swigCMemOwn = false;
        colaPINVOKE.delete_TopologyEdgePointConstPtrVector(swigCPtr);
      }
      swigCPtr = new HandleRef(null, IntPtr.Zero);
      GC.SuppressFinalize(this);
    }
  }

  public TopologyEdgePointConstPtrVector(System.Collections.ICollection c) : this() {
    if (c == null)
      throw new ArgumentNullException("c");
    foreach (EdgePoint element in c) {
      this.Add(element);
    }
  }

  public bool IsFixedSize {
    get {
      return false;
    }
  }

  public bool IsReadOnly {
    get {
      return false;
    }
  }

  public EdgePoint this[int index]  {
    get {
      return getitem(index);
    }
    set {
      setitem(index, value);
    }
  }

  public int Capacity {
    get {
      return (int)capacity();
    }
    set {
      if (value < size())
        throw new ArgumentOutOfRangeException("Capacity");
      reserve((uint)value);
    }
  }

  public int Count {
    get {
      return (int)size();
    }
  }

  public bool IsSynchronized {
    get {
      return false;
    }
  }

  public void CopyTo(System.Array array) {
    CopyTo(0, array, 0, this.Count);
  }

  public void CopyTo(System.Array array, int arrayIndex) {
    CopyTo(0, array, arrayIndex, this.Count);
  }

  public void CopyTo(int index, System.Array array, int arrayIndex, int count) {
    if (array == null)
      throw new ArgumentNullException("array");
    if (index < 0)
      throw new ArgumentOutOfRangeException("index", "Value is less than zero");
    if (arrayIndex < 0)
      throw new ArgumentOutOfRangeException("arrayIndex", "Value is less than zero");
    if (count < 0)
      throw new ArgumentOutOfRangeException("count", "Value is less than zero");
    if (array.Rank > 1)
      throw new ArgumentException("Multi dimensional array.");
    if (index+count > this.Count || arrayIndex+count > array.Length)
      throw new ArgumentException("Number of elements to copy is too large.");
    for (int i=0; i<count; i++)
      array.SetValue(getitemcopy(index+i), arrayIndex+i);
  }

  // Type-safe version of IEnumerable.GetEnumerator
  System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator() {
    return new TopologyEdgePointConstPtrVectorEnumerator(this);
  }

  public TopologyEdgePointConstPtrVectorEnumerator GetEnumerator() {
    return new TopologyEdgePointConstPtrVectorEnumerator(this);
  }

  // Type-safe enumerator
  /// Note that the IEnumerator documentation requires an InvalidOperationException to be thrown
  /// whenever the collection is modified. This has been done for changes in the size of the
  /// collection but not when one of the elements of the collection is modified as it is a bit
  /// tricky to detect unmanaged code that modifies the collection under our feet.
  public sealed class TopologyEdgePointConstPtrVectorEnumerator : System.Collections.IEnumerator {
    private TopologyEdgePointConstPtrVector collectionRef;
    private int currentIndex;
    private object currentObject;
    private int currentSize;

    public TopologyEdgePointConstPtrVectorEnumerator(TopologyEdgePointConstPtrVector collection) {
      collectionRef = collection;
      currentIndex = -1;
      currentObject = null;
      currentSize = collectionRef.Count;
    }

    // Type-safe iterator Current
    public EdgePoint Current {
      get {
        if (currentIndex == -1)
          throw new InvalidOperationException("Enumeration not started.");
        if (currentIndex > currentSize - 1)
          throw new InvalidOperationException("Enumeration finished.");
        if (currentObject == null)
          throw new InvalidOperationException("Collection modified.");
        return (EdgePoint)currentObject;
      }
    }

    // Type-unsafe IEnumerator.Current
    object System.Collections.IEnumerator.Current {
      get {
        return Current;
      }
    }

    public bool MoveNext() {
      int size = collectionRef.Count;
      bool moveOkay = (currentIndex+1 < size) && (size == currentSize);
      if (moveOkay) {
        currentIndex++;
        currentObject = collectionRef[currentIndex];
      } else {
        currentObject = null;
      }
      return moveOkay;
    }

    public void Reset() {
      currentIndex = -1;
      currentObject = null;
      if (collectionRef.Count != currentSize) {
        throw new InvalidOperationException("Collection modified.");
      }
    }
  }

  public void Clear() {
    colaPINVOKE.TopologyEdgePointConstPtrVector_Clear(swigCPtr);
  }

  public void Add(EdgePoint x) {
    colaPINVOKE.TopologyEdgePointConstPtrVector_Add(swigCPtr, EdgePoint.getCPtr(x));
  }

  private uint size() {
    uint ret = colaPINVOKE.TopologyEdgePointConstPtrVector_size(swigCPtr);
    return ret;
  }

  private uint capacity() {
    uint ret = colaPINVOKE.TopologyEdgePointConstPtrVector_capacity(swigCPtr);
    return ret;
  }

  private void reserve(uint n) {
    colaPINVOKE.TopologyEdgePointConstPtrVector_reserve(swigCPtr, n);
  }

  public TopologyEdgePointConstPtrVector() : this(colaPINVOKE.new_TopologyEdgePointConstPtrVector__SWIG_0(), true) {
  }

  public TopologyEdgePointConstPtrVector(int capacity) : this(colaPINVOKE.new_TopologyEdgePointConstPtrVector__SWIG_1(capacity), true) {
    if (colaPINVOKE.SWIGPendingException.Pending) throw colaPINVOKE.SWIGPendingException.Retrieve();
  }

  private EdgePoint getitemcopy(int index) {
    IntPtr cPtr = colaPINVOKE.TopologyEdgePointConstPtrVector_getitemcopy(swigCPtr, index);
    EdgePoint ret = (cPtr == IntPtr.Zero) ? null : new EdgePoint(cPtr, false);
    if (colaPINVOKE.SWIGPendingException.Pending) throw colaPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  private EdgePoint getitem(int index) {
    IntPtr cPtr = colaPINVOKE.TopologyEdgePointConstPtrVector_getitem(swigCPtr, index);
    EdgePoint ret = (cPtr == IntPtr.Zero) ? null : new EdgePoint(cPtr, false);
    if (colaPINVOKE.SWIGPendingException.Pending) throw colaPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  private void setitem(int index, EdgePoint val) {
    colaPINVOKE.TopologyEdgePointConstPtrVector_setitem(swigCPtr, index, EdgePoint.getCPtr(val));
    if (colaPINVOKE.SWIGPendingException.Pending) throw colaPINVOKE.SWIGPendingException.Retrieve();
  }

  public void AddRange(TopologyEdgePointConstPtrVector values) {
    colaPINVOKE.TopologyEdgePointConstPtrVector_AddRange(swigCPtr, TopologyEdgePointConstPtrVector.getCPtr(values));
    if (colaPINVOKE.SWIGPendingException.Pending) throw colaPINVOKE.SWIGPendingException.Retrieve();
  }

  public TopologyEdgePointConstPtrVector GetRange(int index, int count) {
    IntPtr cPtr = colaPINVOKE.TopologyEdgePointConstPtrVector_GetRange(swigCPtr, index, count);
    TopologyEdgePointConstPtrVector ret = (cPtr == IntPtr.Zero) ? null : new TopologyEdgePointConstPtrVector(cPtr, true);
    if (colaPINVOKE.SWIGPendingException.Pending) throw colaPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void Insert(int index, EdgePoint x) {
    colaPINVOKE.TopologyEdgePointConstPtrVector_Insert(swigCPtr, index, EdgePoint.getCPtr(x));
    if (colaPINVOKE.SWIGPendingException.Pending) throw colaPINVOKE.SWIGPendingException.Retrieve();
  }

  public void InsertRange(int index, TopologyEdgePointConstPtrVector values) {
    colaPINVOKE.TopologyEdgePointConstPtrVector_InsertRange(swigCPtr, index, TopologyEdgePointConstPtrVector.getCPtr(values));
    if (colaPINVOKE.SWIGPendingException.Pending) throw colaPINVOKE.SWIGPendingException.Retrieve();
  }

  public void RemoveAt(int index) {
    colaPINVOKE.TopologyEdgePointConstPtrVector_RemoveAt(swigCPtr, index);
    if (colaPINVOKE.SWIGPendingException.Pending) throw colaPINVOKE.SWIGPendingException.Retrieve();
  }

  public void RemoveRange(int index, int count) {
    colaPINVOKE.TopologyEdgePointConstPtrVector_RemoveRange(swigCPtr, index, count);
    if (colaPINVOKE.SWIGPendingException.Pending) throw colaPINVOKE.SWIGPendingException.Retrieve();
  }

  public static TopologyEdgePointConstPtrVector Repeat(EdgePoint value, int count) {
    IntPtr cPtr = colaPINVOKE.TopologyEdgePointConstPtrVector_Repeat(EdgePoint.getCPtr(value), count);
    TopologyEdgePointConstPtrVector ret = (cPtr == IntPtr.Zero) ? null : new TopologyEdgePointConstPtrVector(cPtr, true);
    if (colaPINVOKE.SWIGPendingException.Pending) throw colaPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void Reverse() {
    colaPINVOKE.TopologyEdgePointConstPtrVector_Reverse__SWIG_0(swigCPtr);
  }

  public void Reverse(int index, int count) {
    colaPINVOKE.TopologyEdgePointConstPtrVector_Reverse__SWIG_1(swigCPtr, index, count);
    if (colaPINVOKE.SWIGPendingException.Pending) throw colaPINVOKE.SWIGPendingException.Retrieve();
  }

  public void SetRange(int index, TopologyEdgePointConstPtrVector values) {
    colaPINVOKE.TopologyEdgePointConstPtrVector_SetRange(swigCPtr, index, TopologyEdgePointConstPtrVector.getCPtr(values));
    if (colaPINVOKE.SWIGPendingException.Pending) throw colaPINVOKE.SWIGPendingException.Retrieve();
  }

}

}
