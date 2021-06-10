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
#endif

}

}  // abi
