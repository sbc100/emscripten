#!/usr/bin/env python3
# Copyright 2016 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Wrapper scripte around `llvm-ar`.
"""

import sys
from tools import shared

args = shared.init(sys.argv)
cmd = [shared.LLVM_AR] + args[1:]
sys.exit(shared.run_process(cmd, stdin=sys.stdin, check=False).returncode)
