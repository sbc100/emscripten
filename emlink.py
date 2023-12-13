#!/usr/bin/env python3
# Copyright 2032 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

from tools.toolchain_profiler import ToolchainProfiler
from tools import link
from tools.settings import settings
import emcc

import sys


class LinkState:
  def __init__(self, args):
    # Using tuple here to prevent accidental mutation
    self.orig_args = tuple(args)
    self.link_flags = []
    self.lib_dirs = []
    self.forced_stdlibs = []
    self.mode = 0

  def add_link_flag(self, i, f):
    if f.startswith('-L'):
      self.lib_dirs.append(f[2:])

    self.link_flags.append((i, f))


def parse_linker_args(args):
  for i, a in enumerate(args):
    if a == '-m':
      arch = args[i+1]
      args[i] = None
      args[i+1] = None

  return [a for a in args if a]


@ToolchainProfiler.profile()
def main(args):
  settings.EMLINK = True
  args = parse_linker_args(args)
  state = LinkState(args)
  options, args = emcc.phase_parse_arguments(state)
  _, input_files = emcc.phase_setup(options, state, args)
  #input_files = [(i, a) for i, a in enumerate(args) if not a.startswith('-')]
  return link.run(input_files, options, state, args)


if __name__ == '__main__':
  try:
    sys.exit(main(sys.argv[1:]))
  except KeyboardInterrupt:
    logger.debug('KeyboardInterrupt')
    sys.exit(1)
