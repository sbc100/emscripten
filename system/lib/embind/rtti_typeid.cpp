#include <emscripten/wire.h>

namespace emscripten::internal {
  int embind_uses_rtti_typeid() { return 0;}
}

int mixing_embind_rtti_with_light_typeid_does_not_work = 1;
