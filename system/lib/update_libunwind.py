#!/usr/bin/env python3
# Copyright 2019 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os
import sys
import shutil

script_dir = os.path.abspath(os.path.dirname(__file__))
emscripten_root = os.path.dirname(os.path.dirname(script_dir))
default_llvm_dir = os.path.join(os.path.dirname(emscripten_root), 'llvm-project')
local_root = os.path.join(script_dir, 'libunwind')

preserve_files = ('readme.txt','Unwind-wasm.c')
excludes = ('test',)


def clean_dir(dirname):
  for f in os.listdir(dirname):
    if f in preserve_files:
      continue
    full = os.path.join(dirname, f)
    if os.path.isdir(full):
      clean_dir(full)
      if len(os.listdir(full)) == 0:
        os.rmdir(full)
    else:
      os.remove(full)


def copy_tree(upstream_dir, local_dir):
  for f in os.listdir(upstream_dir):
    full = os.path.join(upstream_dir, f)
    if f not in excludes:
      if os.path.isdir(full):
        os.makedirs(os.path.join(local_dir, f), exist_ok=True)
        copy_tree(full, os.path.join(local_dir, f))
      else:
        shutil.copy2(full, os.path.join(local_dir, f))


def main():
  if len(sys.argv) > 1:
    llvm_dir = os.path.abspath(sys.argv[1])
  else:
    llvm_dir = default_llvm_dir
  libunwind_dir = os.path.join(llvm_dir, 'libunwind')
  assert os.path.exists(libunwind_dir)

  # Remove old version
  clean_dir(local_root)

  copy_tree(libunwind_dir, local_root)


if __name__ == '__main__':
  main()
