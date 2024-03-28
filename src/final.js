/**
 * @license
 * Copyright 2010 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// This file gets included after all the other generated code
// as well as after any --post-js files.

#if MODULARIZE_INSTANCE && !MINIMAL_RUNTIME
return Module;
})();
#endif
