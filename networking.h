#include <emscripten/emscripten.h>
#include <emscripten/websocket.h>
#include <stdio.h>
#include <string>
#include "raylib.h"
#include "entities.h"
#include<algorithm>
using namespace std;

#pragma once

EM_BOOL onopen(int eventType, const EmscriptenWebSocketOpenEvent *websocketEvent, void *userData);

EM_BOOL onerror(int eventType, const EmscriptenWebSocketErrorEvent *websocketEvent, void *userData) {
    puts("onerror");

    return EM_TRUE;
}
EM_BOOL onclose(int eventType, const EmscriptenWebSocketCloseEvent *websocketEvent, void *userData) {
    puts("onclose");

    return EM_TRUE;
}
EM_BOOL onmessage(int eventType, const EmscriptenWebSocketMessageEvent *websocketEvent, void *userData) {
    Environment opp_env;
    memcpy(&opp_env, (websocketEvent->data), websocketEvent->numBytes);
    ((Environment*)(userData))->merge(opp_env);
    return EM_TRUE;
}

class Websocket{
    EMSCRIPTEN_WEBSOCKET_T ws;
    Environment *env;

    public:
    Websocket(){}
    Websocket(Environment *e){
        env = e;
    }
    void ConnectServer(){
	    EmscriptenWebSocketCreateAttributes ws_attrs = {
			"ws://localhost:8765/",
			NULL,
			EM_TRUE
		};
        ws = emscripten_websocket_new(&ws_attrs);
		if (!emscripten_websocket_is_supported()) {
			printf("Emscripten Socket not supported\n");
			return;
		}
		else{
			printf("Emscripten Socket supported!\n");
		}

		emscripten_websocket_set_onopen_callback(ws, this, onopen);
		emscripten_websocket_set_onerror_callback(ws, NULL, onerror);
		emscripten_websocket_set_onclose_callback(ws, NULL, onclose);
		emscripten_websocket_set_onmessage_callback(ws, env, onmessage);

    }
    bool active = false;

    void SendPosition(){
        if(active){
            emscripten_websocket_send_binary(ws, env, sizeof(*env));
        }
    }

};


EM_BOOL onopen(int eventType, const EmscriptenWebSocketOpenEvent *websocketEvent, void *userData) {
    puts("onopen");

    EMSCRIPTEN_RESULT result;
    result = emscripten_websocket_send_utf8_text(websocketEvent->socket, "hoge");
    if (result) {
        printf("Failed to emscripten_websocket_send_utf8_text(): %d\n", result);
    }
    ((Websocket*)(userData))->active = true;
    return EM_TRUE;
}