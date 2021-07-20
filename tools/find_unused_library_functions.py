#!/usr/bin/env python3

import os
import subprocess
import sys
from subprocess import Popen, PIPE

VERBOSE = False


def check_native_symbol(sym):
  native = sym[1:]
  proc = Popen(['git', 'grep', '-w', native, 'system', 'tests'], stdout=PIPE)
  proc.communicate()
  found_results = proc.returncode == 0
  if found_results:
    if VERBOSE:
      print('found in system or tests')
    return 0

  proc = Popen(['git', 'grep', '-w', sym, 'src'], stdout=PIPE)
  proc.communicate()
  found_results = proc.returncode == 0
  if found_results:
    if VERBOSE:
      print('found in JS library')
    return 0

  return 1


def check_js_symbol(sym):
  return


def check_symbol(sym):
  if sym.startswith('___sys_'):
    return 0
  if sym.startswith('_emscripten_autodebug_'):
    return 0
  if sym.endswith('_before_on_calling_thread'):
    return 0

  if sym[0] == '_':
    return check_native_symbol(sym)
  else:
    return check_js_symbol(sym)


def get_all_syms():
  emcc = os.path.join(os.path.dirname(os.path.dirname(__file__)), 'emcc')
  output = subprocess.check_output([emcc, '-sUSE_PTHREADS', '--dump-js-symbols'], text=True)
  return output.splitlines()


def main():
  missing = []
  syms = get_all_syms()

  first = None
  if len(sys.argv) > 1:
    first = sys.argv[1]
    if first not in syms:
      print("symbol not found: " + first)
      return 1

  counter = 0
  should_clear = False
  for sym in syms:
    if first:
      if sym == first:
        first = None
      else:
        counter += 1
        continue
    if should_clear:
      sys.stdout.write("\033[F") # back to previous line
      sys.stdout.write("\033[K") # clear line
      should_clear = False
    sys.stdout.write('checking [%4d/%d] %d%%: %s\n' % (counter, len(syms), counter / len(syms) * 100, sym))
    if check_symbol(sym):
      print('no references found for: ' + sym)
      missing.append(sym)
    elif not VERBOSE:
      should_clear = True

    counter += 1

  if not missing:
    return 0

  print('The following symbols are not referenced:')
  for sym in missing:
    print(' ' + sym)
  return 1


if __name__ == '__main__':
  sys.exit(main())
