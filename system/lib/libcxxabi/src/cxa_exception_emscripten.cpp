#include <cassert>
#include <cstring>
#include <cstdio>
#include <vector>

#include "cxxabi.h"
#include "private_typeinfo.h"
#include "cxa_exception.h"
#include "include/atomic_support.h"

extern "C" void setThrew(uintptr_t threw, int value);
extern "C" void setTempRet0(uint32_t value);

namespace __cxxabiv1 {

extern "C" {

//  Utility routines
static inline __cxa_exception* cxa_exception_from_thrown_object(void* thrown_object) {
    return static_cast<__cxa_exception*>(thrown_object) - 1;
}

void *__cxa_allocate_exception(size_t thrown_size) _NOEXCEPT {
  size_t actual_size = sizeof(__cxa_exception) + thrown_size;
  char* buffer = (char*)::malloc(actual_size);
  ::memset(buffer, 0, actual_size);
  //printf("__cxa_allocate_exception obj=%p header=%p\n", buffer + sizeof(__cxa_exception), buffer);
  return buffer + sizeof(__cxa_exception);
}

void __cxa_free_exception(void *thrown_object) _NOEXCEPT {
  __cxa_exception* exception = cxa_exception_from_thrown_object(thrown_object);
  //printf("__cxa_free_exception obj=%p header=%p\n", thrown_object, exception);
  ::free(exception);
}

#ifdef __USING_EMSCRIPTEN_EXCEPTIONS__
void __cxa_increment_exception_refcount(void *thrown_object) _NOEXCEPT {
  if (thrown_object != NULL) {
    __cxa_exception* exception_header = cxa_exception_from_thrown_object(thrown_object);
    //printf("__cxa_increment_exception_refcount obj=%p header=%p\n", thrown_object, exception_header);
    std::__libcpp_atomic_add(&exception_header->referenceCount, size_t(1));
  }
}

_LIBCXXABI_NO_CFI void __cxa_decrement_exception_refcount(void *thrown_object) _NOEXCEPT {
  if (thrown_object != NULL) {
    __cxa_exception* exception_header = cxa_exception_from_thrown_object(thrown_object);
    //printf("__cxa_decrement_exception_refcount obj=%p header=%p rethrown=%d\n", thrown_object, exception_header, exception_header->rethrown);
    // A rethrown exception can reach refcount 0; it must not be discarded
    // Its next handler will clear the rethrown flag and addRef it, prior to
    // final decRef and destruction here
    if (std::__libcpp_atomic_add(&exception_header->referenceCount, size_t(-1)) == 0 && !exception_header->rethrown) {
      //printf("__cxa_decrement_exception_refcount destructor=%p\n", exception_header->exceptionDestructor);
      if (NULL != exception_header->exceptionDestructor)
          exception_header->exceptionDestructor(thrown_object);
      __cxa_free_exception(thrown_object);
      //err('decref freeing exception ' + [info.excPtr, exceptionLast, 'stack', exceptionCaught]);
    }
  }
}

// This native structure is returned from __cxa_find_matching_catch, and serves as catching
// context, i.e. stores information required to proceed with a specific selected catch. It stores
// base and adjusted pointers of a thrown object. It is allocated dynamically and should be freed
// when it is done with a specific catch (i.e. either in __cxa_end_catch when caught or in
// __resumeException when no catch clause matched). The class itself is just a native pointer
// wrapper, and contains all the necessary accessors for the fields in the native structure.
// ptr - Native structure pointer to wrap, the structure is allocated when not specified.
struct __catch_info {
  void* basePtr;
  void* adjustedPtr;
};

static __cxa_exception* get_cxa_exception(__catch_info* info) {
  return cxa_exception_from_thrown_object(info->basePtr);
}

static int is_pointer_type(std::type_info* type) {
  return !!dynamic_cast<__pointer_type_info*>(type);
}

// Get pointer which is expected to be received by catch clause in C++ code. It may be adjusted
// when the pointer is casted to some of the exception object base classes (e.g. when virtual
// inheritance is used). When a pointer is thrown this method should return the thrown pointer
// itself.
static void* get_exception_ptr(__catch_info* info) {
  bool isPointer = is_pointer_type(get_cxa_exception(info)->exceptionType);
  if (isPointer) {
    return info->basePtr;
  }
  if (info->adjustedPtr) {
    return &info->adjustedPtr;
  }
  return &info->basePtr;
}

static int uncaughtExceptionCount;
static std::vector<__catch_info*> exceptionCaught;

void* __cxa_begin_catch(void* unwind_arg) _NOEXCEPT {
  __catch_info* info = (__catch_info*)unwind_arg;
  __cxa_exception* ex = get_cxa_exception(info);
  if (!ex->caught) {
    ex->caught = true;
    uncaughtExceptionCount--;
  }
  ex->rethrown = false;
  exceptionCaught.push_back(info);
#if 0
  err('cxa_begin_catch ' + [ptr, 'stack', exceptionCaught]);
#endif
  __cxa_increment_exception_refcount(info->basePtr);
  return get_exception_ptr(info);
}

static void* exceptionLast;

// We're done with a catch. Now, we can run the destructor if there is one
// and free the exception. Note that if the dynCall on the destructor fails
// due to calling apply on undefined, that means that the destructor is
// an invalid index into the FUNCTION_TABLE, so something has gone wrong.
void __cxa_end_catch() {
  // Clear state flag.
  setThrew(0, 0);
  assert(exceptionCaught.size() > 0);
  // Call destructor if one is registered then clear it.
  __catch_info* catchInfo = exceptionCaught.back();
  exceptionCaught.pop_back();

  //err('cxa_end_catch popped ' + [catchInfo, exceptionLast, 'stack', exceptionCaught]);
  __cxa_decrement_exception_refcount(catchInfo->basePtr);
  ::free(catchInfo);
  exceptionLast = 0; // XXX in decRef?
}

int __cxa_can_catch(std::type_info* catchType, std::type_info* excpType, void **thrown);

// Finds a suitable catch clause for when an exception is thrown.
// In normal compilers, this functionality is handled by the C++
// 'personality' routine. This is passed a fairly complex structure
// relating to the context of the exception and makes judgements
// about how to handle it. Some of it is about matching a suitable
// catch clause, and some of it is about unwinding. We already handle
// unwinding using 'if' blocks around each function, so the remaining
// functionality boils down to picking a suitable 'catch' block.
// We'll do that here, instead, to keep things simpler.
void* __cxa_find_matching_catch_v(int count, ...) {
  void* thrown = exceptionLast;
  if (!thrown) {
    // just pass through the null ptr
    setTempRet0(0);
    return 0;
  }
  __cxa_exception* info = cxa_exception_from_thrown_object(thrown);
  std::type_info* thrownType = info->exceptionType;
  __catch_info CatchInfo{thrown, 0}
  if (!thrownType) {
    // just pass through the thrown ptr
    setTempRet0(0);
    return catchInfo.basePtr;
  }

  // can_catch receives a **, add indirection
  //out("can_catch on " + [thrown]);
  void* exceptionThrown = 0;
  // The different catch blocks are denoted by different types.
  // Due to inheritance, those types may not precisely match the
  // type of the thrown object. Find one which matches, and
  // return the type of the catch block which should be called.
  va_list ap;
  va_start(ap, count);
  for (int i = 0; i < count; i++) {
    std::type_info* caughtType = va_arg(va_list ap, std::type_info*);
    if (caughtType === 0 || caughtType === thrownType) {
      // Catch all clause matched or exactly the same type is caught
      break;
    }
    if (__cxa_can_catch(caughtType, thrownType, &exceptionThrown) {
      if (thrown !== exceptionThrown) {
        catchInfo.set_adjusted_ptr(exceptionThrown);
      }
      //out("  can_catch found " + [adjusted, caughtType]);
      setTempRet0(caughtType);
      return makeStructuralReturn(catchInfo.basePtr);
    }
  }
  va_end(ap);
  setTempRet0(thrownType);
  return catchInfo.basePtr;
},

#endif // __USING_EMSCRIPTEN_EXCEPTIONS__

} // extern "C"

}  // abi
