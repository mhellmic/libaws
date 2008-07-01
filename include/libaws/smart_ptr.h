#ifndef LIBAWS_SMARTPTR_API_H
#define LIBAWS_SMARTPTR_API_H

#include <libaws/common.h>

namespace aws {

class SmartObject
{
protected:
  mutable unsigned int  theRefCount;

public:
  SmartObject() : theRefCount(0) { }

  SmartObject(const SmartObject&) : theRefCount(0) { }

  virtual ~SmartObject() { }

  virtual void free() { delete this; }

  long getRefCount() const { return theRefCount; }

  void addReference() const { ++theRefCount; }

  void removeReference () {
    if (--theRefCount == 0) 
      free();
  }

	SmartObject& operator=(const SmartObject&) { return *this; }
}; /* class SmartObject */

template<class T>
class SmartPtr 
{
protected:
  T  *p;

  void init() {
    if (p != 0) p->addReference();
  }

  template <class otherT> SmartPtr& 
  assign (const SmartPtr<otherT>& rhs) {
		if (p != rhs.get())
    {
			if (p) p->removeReference();
			p = static_cast<T*>(rhs.get());
			init();
		}
		return *this;
  }

public:
  SmartPtr(T* realPtr = 0) : p(realPtr) {
    init(); 
  }

  SmartPtr(SmartPtr const& rhs) : p(rhs.get()) {
    init();
  }

  ~SmartPtr() throw () {
    if (p)
      p->removeReference();
    p = 0;
  }

  bool isNull () const        { return p == 0; }

  T* get() const             { return p; }

  operator T* ()              { return get(); }
  operator const T * () const { return get(); }

  T* operator->() const       { return p; } 
  T& operator*() const        { return *p; } 

	bool operator==(SmartPtr const& h) const  { return p == h.p; }
	bool operator!=(SmartPtr const& h) const  { return p != h.p; }
	bool operator==(T const* pp) const        { return p == pp; } 
	bool operator!=(T const* pp) const        { return p != pp; }
  bool operator<(const SmartPtr& h) const   { return p < h.p; }



  SmartPtr& operator=(SmartPtr const& rhs) {
    return assign (rhs);
  }

	template <class otherT> SmartPtr& operator=(SmartPtr<otherT> const& rhs) {
    return assign (rhs);
	}

};  /* SmartPtr */

} /* namespace aws */
#endif
