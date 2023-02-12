#include <emscripten/emscripten.h>
#include <emscripten/websocket.h>
#include <sstream>
#include <stdio.h>
#include <string>
#include <stdexcept>
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
    if(websocketEvent->numBytes < 10){
        return EM_TRUE;
    }
    std::istringstream s((char*)(websocketEvent->data));
    s.exceptions(ios::eofbit | ios::failbit | ios::badbit);
    try{
        Environment opp_env = Environment::deserialize(s);
        ((Environment*)(userData))->merge(opp_env);
    }
    catch (exception&){
        printf("Caught exception\n");
    }
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
			"ws://213.133.90.205:8765/",
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
            string env_str = env->serialize();
            emscripten_websocket_send_utf8_text(ws, env_str.c_str());
        }
    }

};


EM_BOOL onopen(int eventType, const EmscriptenWebSocketOpenEvent *websocketEvent, void *userData) {
    puts("onopen");

    ((Websocket*)(userData))->active = true;
    return EM_TRUE;
}