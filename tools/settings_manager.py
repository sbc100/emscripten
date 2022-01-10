# Copyright 2021 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import difflib
import os
import importlib.util

from .utils import path_from_root, exit_with_error
from . import diagnostics
from . import settings
from . import settings_internal

# Subset of settings that take a memory size (i.e. 1Gb, 64kb etc)
MEM_SIZE_SETTINGS = (
    'TOTAL_STACK',
    'INITIAL_MEMORY',
    'MEMORY_GROWTH_LINEAR_STEP',
    'MEMORY_GROWTH_GEOMETRIC_CAP',
    'GL_MAX_TEMP_BUFFER_SIZE',
    'MAXIMUM_MEMORY',
    'DEFAULT_PTHREAD_STACK_SIZE'
)

PORTS_SETTINGS = (
    # All port-related settings are valid at compile time
    'USE_SDL',
    'USE_LIBPNG',
    'USE_BULLET',
    'USE_ZLIB',
    'USE_BZIP2',
    'USE_VORBIS',
    'USE_COCOS2D',
    'USE_ICU',
    'USE_MODPLUG',
    'USE_SDL_MIXER',
    'USE_SDL_IMAGE',
    'USE_SDL_TTF',
    'USE_SDL_NET',
    'USE_SDL_GFX',
    'USE_LIBJPEG',
    'USE_OGG',
    'USE_REGAL',
    'USE_BOOST_HEADERS',
    'USE_HARFBUZZ',
    'USE_MPG123',
    'USE_GIFLIB',
    'USE_FREETYPE',
    'SDL2_MIXER_FORMATS',
    'SDL2_IMAGE_FORMATS',
)

# Subset of settings that apply at compile time.
# (Keep in sync with [compile] comments in settings.js)
COMPILE_TIME_SETTINGS = (
    'MEMORY64',
    'INLINING_LIMIT',
    'DISABLE_EXCEPTION_CATCHING',
    'DISABLE_EXCEPTION_THROWING',
    'MAIN_MODULE',
    'SIDE_MODULE',
    'RELOCATABLE',
    'STRICT',
    'EMSCRIPTEN_TRACING',
    'USE_PTHREADS',
    'SUPPORT_LONGJMP',
    'DEFAULT_TO_CXX',
    'WASM_OBJECT_FILES',

    # Internal settings used during compilation
    'EXCEPTION_CATCHING_ALLOWED',
    'EXCEPTION_HANDLING',
    'LTO',
    'OPT_LEVEL',
    'DEBUG_LEVEL',

    # This is legacy setting that we happen to handle very early on
    'RUNTIME_LINKED_LIBS',
    # TODO: should not be here
    'AUTO_ARCHIVE_INDEXES',
    'DEFAULT_LIBRARY_FUNCS_TO_INCLUDE',
) + PORTS_SETTINGS


alt_names = {}
all_settings = {}
internal_settings = set()
allowed_settings = []
legacy_settings = {}

def dict():
  return all_settings


def keys(self):
  return all_settings.keys()


def limit_settings(allowed):
  allowed_settings.clear()
  if allowed:
    allowed_settings.extend(allowed)


def get_item(self, key):
  return all_settings[key]


def set_item(self, key, value):
  all_settings[key] = value


def set_setting(self, name, value):
  if allowed_settings:
    assert name in allowed_settings, f"internal error: attempt to write setting '{name}' while in limited settings mode"

  if name == 'STRICT' and value:
    for a in legacy_settings:
      all_settings.pop(a, None)

  if name in legacy_settings:
    # TODO(sbc): Rather then special case this we should have STRICT turn on the
    # legacy-settings warning below
    if all_setting['STRICT']:
      exit_with_error('legacy setting used in strict mode: %s', name)
    fixed_values, error_message = legacy_settings[name]
    if fixed_values and value not in fixed_values:
      exit_with_error('Invalid command line option -s ' + name + '=' + str(value) + ': ' + error_message)
    diagnostics.warning('legacy-settings', 'use of legacy setting: %s (%s)', name, error_message)

  if name in self.alt_names:
    alt_name = self.alt_names[name]
    all_settings[alt_name] = value

  if name not in all_settings:
    msg = "Attempt to set a non-existent setting: '%s'\n" % name
    suggestions = difflib.get_close_matches(name, list(all_settings.keys()))
    suggestions = [s for s in suggestions if s not in legacy_settings]
    suggestions = ', '.join(suggestions)
    if suggestions:
      msg += ' - did you mean one of %s?\n' % suggestions
    msg += " - perhaps a typo in emcc's  -s X=Y  notation?\n"
    msg += ' - (see src/settings.py for valid values)'
    exit_with_error(msg)

  self.all_settings[name] = value


def get_setting(attr):
  if allowed_settings:
    assert attr in allowed_settings, f"internal error: attempt to read setting '{attr}' while in limited settings mode"

  if attr in all_settings:
    return all_settings[attr]
  else:
    raise AttributeError(f"no such setting: '{attr}'")


def init():
  # Load the settings defaults.
  def read_settings(mod):
    for key in dir(mod):
      if not key.startswith('__'):
        all_settings[key] = getattr(mod, key)
  read_settings(settings)
  read_settings(settings_internal)

  if 'EMCC_STRICT' in os.environ:
    all_settings['STRICT'] = int(os.environ.get('EMCC_STRICT'))

  # Special handling for LEGACY_SETTINGS.  See src/setting.js for more
  # details
  for name, new_name in all_settings['LEGACY_SETTINGS'].items():
    if len(new_name) == 1:
      new_name = new_name[0]
      legacy_settings[name] = (None, 'setting renamed to ' + new_name)
      alt_names[name] = new_name
      alt_names[new_name] = name
      default_value = all_settings[new_name]
    else:
      fixed_values, err = new_name
      legacy_settings[name] = (fixed_values, err)
      default_value = fixed_values[0]
    assert name not in all_settings, 'legacy setting (%s) cannot also be a regular setting' % name
    if not all_settings['STRICT']:
      all_settings[name] = default_value

  settings.__setattr__ = set_setting
  settings.__getattr__ = get_setting
  settings.__setitem__ = set_item
  settings.__getitem__ = get_item

init()
