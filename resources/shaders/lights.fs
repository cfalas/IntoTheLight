#version 100

precision mediump float;

#define MAX_SPOTS  3

struct Spot {
    vec2 pos;        // window coords of spot
    float inner;    // inner fully transparent centre radius
    float radius;    // alpha fades out to this radius
};

uniform Spot spots[MAX_SPOTS];  // Spotlight positions array
uniform float screenWidth;      // Width of the screen

void main()
{
    gl_FragColor = vec4(0, 0, 0, alpha);
}