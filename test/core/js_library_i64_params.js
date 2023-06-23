TestLibrary = {
  jscall__deps: i53ConversionDeps,
  jscall__sig: 'jj',
  jscall: function(foo) {
    if (isNaN(foo)) {
      err('overflow');
      return 42;
    }
    err('js:got:       ' + foo);

    _called_from_js({{{ splitI64("foo") }}});

    if (foo < 0)
      var rtn = Math.ceil(foo / 2);
    else
      rtn = Math.floor(foo / 2);
    err('js:returning: ' + rtn);
    return {{{ makeReturn64('rtn') }}};
  },
}

mergeInto(LibraryManager.library, TestLibrary);
