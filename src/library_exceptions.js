/**
 * @license
 * Copyright 2010 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

var LibraryExceptions = {
  $uncaughtExceptionCount: '0',

  // This class is the exception metadata which is prepended to each thrown object (in WASM memory).
  // It is allocated in one block among with a thrown object in __cxa_allocate_exception and freed
  // in ___cxa_free_exception. It roughly corresponds to __cxa_exception structure in libcxxabi. The
  // class itself is just a native pointer wrapper, and contains all the necessary accessors for the
  // fields in the native structure.
  // TODO: Unfortunately this approach still cannot be considered thread-safe because single
  // exception object can be simultaneously thrown in several threads and its state (except
  // reference counter) is not protected from that. Also protection is not enough, separate state
  // should be allocated. libcxxabi has concept of dependent exception which is used for that
  // purpose, it references the primary exception.
  //
  // excPtr - Thrown object pointer to wrap. Metadata pointer is calculated from it.
  $ExceptionInfo: function(excPtr) {
    this.excPtr = excPtr;
    this.ptr = excPtr - {{{ C_STRUCTS.__cxa_exception.__size__ }}};

    this.set_type = function(type) {
      {{{ makeSetValue('this.ptr', C_STRUCTS.__cxa_exception.exceptionType, 'type', '*') }}};
    };

    this.get_type = function() {
      return {{{ makeGetValue('this.ptr', C_STRUCTS.__cxa_exception.exceptionType, '*') }}};
    };

    this.set_destructor = function(destructor) {
      {{{ makeSetValue('this.ptr', C_STRUCTS.__cxa_exception.exceptionDestructor, 'destructor', '*') }}};
    };

    this.set_caught = function (caught) {
      caught = caught ? 1 : 0;
      {{{ makeSetValue('this.ptr', C_STRUCTS.__cxa_exception.caught, 'caught', 'i8') }}};
    };

    this.get_caught = function () {
      return {{{ makeGetValue('this.ptr', C_STRUCTS.__cxa_exception.caught, 'i8') }}} != 0;
    };

    this.set_rethrown = function (rethrown) {
      rethrown = rethrown ? 1 : 0;
      {{{ makeSetValue('this.ptr', C_STRUCTS.__cxa_exception.rethrown, 'rethrown', 'i8') }}};
    };

    this.get_rethrown = function () {
      return {{{ makeGetValue('this.ptr', C_STRUCTS.__cxa_exception.rethrown, 'i8') }}} != 0;
    };

    // Initialize native structure fields. Should be called once after allocated.
    this.init = function(type, destructor) {
      this.set_type(type);
      this.set_destructor(destructor);
    }
  },

  // Here, we throw an exception after recording a couple of values that we need to remember
  // We also remember that it was the last exception thrown as we need to know that later.
  __cxa_throw__sig: 'viii',
  __cxa_throw__deps: ['$ExceptionInfo', '$exceptionLast', '$uncaughtExceptionCount'],
  __cxa_throw: function(ptr, type, destructor) {
#if EXCEPTION_DEBUG
    err('Compiled code throwing an exception, ' + [ptr.toString(16),type,destructor]);
#endif
    var info = new ExceptionInfo(ptr);
    // Initialize ExceptionInfo content after it was allocated in __cxa_allocate_exception.
    info.init(type, destructor);
    exceptionLast = ptr;
    uncaughtExceptionCount++;
    {{{ makeThrow('ptr') }}}
  },

  // This exception will be caught twice, but while begin_catch runs twice,
  // we early-exit from end_catch when the exception has been rethrown, so
  // pop that here from the caught exceptions.
  __cxa_rethrow__deps: ['$exceptionCaught', '$exceptionLast', '$uncaughtExceptionCount'],
  __cxa_rethrow__sig: 'v',
  __cxa_rethrow: function() {
    var catchInfo = exceptionCaught.pop();
    if (!catchInfo) {
      abort('no exception to throw');
    }
    var info = catchInfo.get_exception_info();
    var ptr = catchInfo.get_base_ptr();
    if (!info.get_rethrown()) {
      // Only pop if the corresponding push was through rethrow_primary_exception
      exceptionCaught.push(catchInfo);
      info.set_rethrown(true);
      info.set_caught(false);
      uncaughtExceptionCount++;
    } else {
      catchInfo.free();
    }
#if EXCEPTION_DEBUG
    err('Compiled code RE-throwing an exception, popped ' +
      [ptr, exceptionLast, 'stack', exceptionCaught]);
#endif
    exceptionLast = ptr;
    {{{ makeThrow('ptr') }}}
  },

  llvm_eh_typeid_for: function(type) {
    return type;
  },

  __cxa_get_exception_ptr__deps: ['$CatchInfo'],
  __cxa_get_exception_ptr: function(ptr) {
#if EXCEPTION_DEBUG
    err('cxa_get_exception_ptr ' + ptr);
#endif
    return new CatchInfo(ptr).get_exception_ptr();
  },

  __cxa_uncaught_exceptions__deps: ['$uncaughtExceptionCount'],
  __cxa_uncaught_exceptions: function() {
    return uncaughtExceptionCount;
  },

  __cxa_call_unexpected: function(exception) {
    err('Unexpected exception thrown, this is not properly supported - aborting');
#if !MINIMAL_RUNTIME
    ABORT = true;
#endif
    throw exception;
  },

  __cxa_current_primary_exception__deps: ['$exceptionCaught',
                                          '__cxa_increment_exception_refcount',
                                          '$CatchInfo'],
  __cxa_current_primary_exception: function() {
    if (!exceptionCaught.length) {
      return 0;
    }
    var catchInfo = exceptionCaught[exceptionCaught.length - 1];
    ___cxa_increment_exception_refcount(catchInfo.get_base_ptr());
    return catchInfo.get_base_ptr();
  },

  __cxa_rethrow_primary_exception__deps: ['$CatchInfo', '$exceptionCaught', '__cxa_rethrow'],
  __cxa_rethrow_primary_exception: function(ptr) {
    if (!ptr) return;
    var catchInfo = new CatchInfo();
    catchInfo.set_base_ptr(ptr);
    var info = catchInfo.get_exception_info();
    exceptionCaught.push(catchInfo);
    info.set_rethrown(true);
    ___cxa_rethrow();
  },

  __resumeException__deps: ['$exceptionLast', '$CatchInfo'],
  __resumeException: function(catchInfoPtr) {
    var catchInfo = new CatchInfo(catchInfoPtr);
    var ptr = catchInfo.get_base_ptr();
#if EXCEPTION_DEBUG
    out("Resuming exception " + [ptr, exceptionLast]);
#endif
    if (!exceptionLast) { exceptionLast = ptr; }
    catchInfo.free();
    {{{ makeThrow('ptr') }}}
  },
};

// In LLVM, exceptions generate a set of functions of form __cxa_find_matching_catch_1(), __cxa_find_matching_catch_2(), etc.
// where the number specifies the number of arguments. In Emscripten, route all these to a single function '__cxa_find_matching_catch'
// that variadically processes all of these functions using JS 'arguments' object.
addCxaCatch = function(n) {
  LibraryManager.library['__cxa_find_matching_catch_' + n] = LibraryExceptions['__cxa_find_matching_catch'];
  LibraryManager.library['__cxa_find_matching_catch_' + n + '__sig'] = new Array(n + 2).join('i');
  LibraryManager.library['__cxa_find_matching_catch_' + n + '__deps'] = LibraryExceptions['__cxa_find_matching_catch__deps'];
};

mergeInto(LibraryManager.library, LibraryExceptions);
