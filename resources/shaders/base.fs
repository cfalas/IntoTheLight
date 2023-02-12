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

vec2 getData(int id) {
    for (int i=0; i<MAX_FOCUSPOINTS; i++) {
        if (i == id) return focuspoints[i];
    }
}

void main()
{
    float alph = 1.0/(distance(getData(int(fragColor.x*255.0)),fragPosition.xy)*0.005);
    alph = alph*0.3;
    gl_FragColor = vec4(1,1,1,alph);
}