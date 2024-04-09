var b = Module, c = b.$ww, e = "undefined" != typeof document ? document.currentScript?.src : void 0, k, q, r;

"function" == typeof importScripts && (e = self.location.href);

c && (onmessage = a => {
    onmessage = null;
    a = a.data;
    self.Module = a;
});

var f = b.mem || new WebAssembly.Memory({
    initial: 256,
    maximum: 256,
    shared: !0
}), g = f.buffer, h = [], l = a => {
    a = a.data;
    let d = a._wsc;
    d && k.get(d)(...a.x);
}, m = a => {
    h.push(a);
}, n = {}, p = 1;

c && (n[0] = this, addEventListener("message", m));

WebAssembly.instantiate(b.wasm, {
    a: {
        b: (a, d) => {
            let t = n[p] = new Worker(e, {
                workerData: "em-ww"
            });
            t.postMessage({
                workerID: p,
                wasm: b.wasm,
                wasmMemory: f,
                sb: a,
                sz: d
            });
            t.onmessage = l;
            return p++;
        },
        c: () => !1,
        d: (a, d) => {
            n[a].postMessage({
                _wsc: d,
                x: []
            });
        },
        e: function() {
            console.log("Hello from wasm worker!");
        },
        a: f
    }
}).then((a => {
    a = a.instance.exports;
    q = a.g;
    r = a.i;
    k = a.h;
    c ? (a = b, r(a.sb, a.sz), removeEventListener("message", m), h = h.forEach(l), 
    addEventListener("message", l)) : a.f();
    c || q();
}));