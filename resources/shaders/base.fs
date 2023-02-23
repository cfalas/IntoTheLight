#version 100
#define MAX_FOCUSPOINTS 100

precision mediump float;

// Input vertex attributes (from vertex shader)
varying vec3 fragPosition;
varying vec2 fragTexCoord;
varying vec4 fragColor;

// Input uniform values
uniform vec2 focuspoints[MAX_FOCUSPOINTS];

// NOTE: Add here your custom variables

vec2 getFocus(int id) {
    for (int i=0; i<MAX_FOCUSPOINTS; i++) {
        if (i == id) return focuspoints[i];
    }
}

void main()
{
    float alph = 1.0/((distance(getFocus(int(fragColor.x*255.0)),fragPosition.xy) - 200.0)*0.005);
    alph = alph*0.3;
    if(fragColor.y*255.0>0.5){
        gl_FragColor = vec4(1,0,0,alph);
    } else {
        gl_FragColor = vec4(0,0,1,alph);
    }
}