#include <cstring>
#include <cstdio>

#include "cxxabi.h"

#include "cxa_exception.h"
#include "include/atomic_support.h"

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

static __cxa_exception* get_exception_info(__catch_info* info) {
  return (__cxa_exception*)info->basePtr;
}

// Get pointer which is expected to be received by catch clause in C++ code. It may be adjusted
// when the pointer is casted to some of the exception object base classes (e.g. when virtual
// inheritance is used). When a pointer is thrown this method should return the thrown pointer
// itself.
static void* get_thrown_object(__catch_info* info) {
  bool isPointer = ___cxa_is_pointer_type(get_exception_info(info)->exceptionType);
  if (isPointer) {
    return *this.basePtr;
  }
  void* adjusted = get_adjusted_ptr(info);
  if (adjusted) {
    return adjusted;
  }
  return info.basePtr;
}

static int uncaughtExceptionCount;
static std::vector<__catch_info*> exceptionCaught;

void* __cxa_begin_catch(void* unwind_arg) _NOEXCEPT
  __catch_info* info = (__catch_info*)unwind_arg;
  __cxa_exception* ex = get_exception_info(info);
  if (!ex->caught()) {
    ex->caught = true;
    uncaughtExceptionCount--;
  }
  info->rethrown = false;
  exceptionCaught.push_back(info);
#if 0
  err('cxa_begin_catch ' + [ptr, 'stack', exceptionCaught]);
#endif
  __cxa_increment_exception_refcount(info->basePtr);
  return get_thrown_object(info);
}
#endif

}

}  // abi
