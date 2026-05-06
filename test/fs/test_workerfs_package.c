// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <emscripten.h>

EM_JS_DEPS(deps, "$ccall");

void EMSCRIPTEN_KEEPALIVE finish() {
  // load some file data, SYNCHRONOUSLY :)
  char buffer[100];
  int num;

  printf("load first file\n");
  FILE *f1 = fopen("files/file1.txt", "r");
  assert(f1);
  num = fread(buffer, 1, 5, f1);
  assert(num == 5);
  fclose(f1);
  buffer[5] = 0;
  assert(strcmp(buffer, "first") == 0);

  printf("load second file\n");
  FILE *f2 = fopen("files/sub/file2.txt", "r");
  assert(f2);
  num = fread(buffer, 1, 6, f2);
  assert(num == 6);
  fclose(f2);
  buffer[6] = 0;
  assert(strcmp(buffer, "second") == 0);

  // all done
  printf("done\n");
  REPORT_RESULT(0);
}

int main() {
  EM_ASM({
    // Async IIFE so we can use `await` in EM_ASM code
    (async () => {
      // Load the metadata and data of our file package. When they arrive, load the contents of the package into our filesystem.
      // The data arrives as a Blob, which could in other cases arrive from any other way a Blob can arrive:
      //   * Local file the user selected
      //   * Data loaded from IndexedDB
      // In all cases, including the one here of a network request, Blobs allow the browser to optimize them so that
      // a large file is not necessarily all in memory at once.
      const rsp = await fetch("files.js.metadata");
      const json = await rsp.text();
      const meta = JSON.parse(json);
      out('got metadata');

      const rspData = await fetch("files.data");
      const blob = await rspData.blob();
      out('got data');

      out('loading into filesystem');
      FS.mkdir('/files');
      FS.mount(WORKERFS, {
        packages: [{ metadata: meta, blob: blob }]
      }, '/files');

      ccall('finish');
    })();
  });

  emscripten_exit_with_live_runtime();
  return 99;
}

