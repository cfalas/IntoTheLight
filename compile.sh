#!/bin/sh
mkdir -p dist
rm dist/game.wasm
em++ -s USE_GLFW=3 -DPLATFORM_WEB -o dist/index.js main.cpp -O3 -Wall libraylib.a -s ASYNCIFY -lwebsocket.js -sASSERTIONS --preload-file resources -sALLOW_MEMORY_GROWTH