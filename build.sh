set -x -e
./em++ -Wno-experimental side.cpp -sSIDE_MODULE -pthread -fexceptions --profiling-funcs -c -o side.o --profiling-funcs -g
./em++ -Wno-experimental side.o -sSIDE_MODULE -pthread -fexceptions --profiling-funcs -o side.wasm -sSAFE_HEAP -sSAFE_HEAP_LOG
./em++ -Wno-experimental main.cpp -pthread -fexceptions -sPTHREADS_DEBUG -sEXIT_RUNTIME -sPROXY_TO_PTHREAD -sMAIN_MODULE=2 side.wasm --profiling-funcs -sSAFE_HEAP -sSAFE_HEAP_LOG
node --experimental-wasm-threads ./a.out.js

