if (ENVIRONMENT_IS_WASM_WORKER) {

#if ENVIRONMENT_MAY_BE_NODE
  // Node.js support
  if (ENVIRONMENT_IS_NODE) {
    // Create as web-worker-like an environment as we can.

    var parentPort = worker_threads['parentPort'];
    parentPort.on('message', (data) => typeof onmessage === "function" && onmessage({ data: data }));

    Object.assign(globalThis, {
      self: global,
      postMessage: (msg) => parentPort.postMessage(msg),
      performance: global.performance || { now: Date.now },
      addEventListener: (name, handler) => parentPort.on(name, handler),
      removeEventListener: (name, handler) => parentPort.off(name, handler),
    });
  }
#endif

  {{{ implicitSelf() }}}onmessage = (d) => {
    // The first message sent to the Worker is always the bootstrap message.
    // Drop this message listener, it served its purpose of bootstrapping
    // the Wasm Module load, and is no longer needed. Let user code register
    // any desired message handlers from now on.
    {{{ implicitSelf() }}}onmessage = null;
    d = d.data;
#if !MODULARIZE
    self.{{{ EXPORT_NAME }}} = d;
#endif
#if !MINIMAL_RUNTIME
    _wasmWorkerID = d['workerID'];
    console.error("running worker");
    d['instantiateWasm'] = (info, receiveInstance) => {
      var instance = new WebAssembly.Instance(d['wasm'], info);
      return receiveInstance(instance, d['wasm']);
    }
#endif
  }
}
