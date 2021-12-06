#!/usr/bin/env python3
# Copyright 2021 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""emld - linker emulation script

emld is a drop-in replacement for a link such as ld or wasm-ld.

See emld --help  for details.
"""

from tools.toolchain_profiler import ToolchainProfiler  # noqa

import logging
import os
import sys
import time

from tools.settings import settings
from tools import shared
import emcc

logger = logging.getLogger('emld')


def run(args):
  if shared.DEBUG:
    logger.warning(f'invocation: {shared.shlex_join(args)} (in {os.getcwd()})')

  ## Process argument and setup the compiler
  state = emcc.EmccState(args)
  options, newargs, settings_map = emcc.phase_parse_arguments(state)
  #linker_args = [(i, a) for i, a in enumerate(args[1:])]
  linker_args = args[1:]
  target, wasm_target = emcc.phase_linker_setup(options, state, newargs, settings_map)
  return emcc.do_link(options, state, target, wasm_target, linker_args)


def main(args):
  start_time = time.time()
  ret = run(args)
  logger.debug('total time: %.2f seconds', (time.time() - start_time))
  return ret


if __name__ == '__main__':
  try:
    sys.exit(main(sys.argv))
  except KeyboardInterrupt:
    logger.warning('KeyboardInterrupt')
    sys.exit(1)
