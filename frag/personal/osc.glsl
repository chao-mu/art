#version 410

#define PI 3.1415926

layout (location = 0) out vec4 o;

uniform float iTime;

in vec2 uv;
in vec2 tc;

#pragma channel float drift 0.
#pragma channel float freq 1.
#pragma channel float phase 0.
#pragma channel float key .5
#pragma channel bool keyEnabled false
#pragma channel bool flip false
#pragma channel float shape 0.
#pragma channel float orient 0.

#define SINE_THRESH 0.5

float osc(float shape, float x, float freq, float drift, float phase) {
    float sine = sin(2 * PI * x * freq + (drift * iTime) + phase + PI) * .5 + .5;
    float tri = abs(mod(x * freq + (drift * iTime) + phase, 2.0) - 1.0);
    float saw = fract(x * freq + (drift * iTime) + phase);
    float sqr = tri > 0.5 ? 1 : 0;

    if (shape <= 0.5) {
        return mix(sine, tri, shape * 2.);
    } else {
        return mix(tri, saw, (shape - .5) * 2);
    }
}

void main() {
    float freq = channel_freq(tc);
    float drift = channel_drift(tc);
    float phase = channel_phase(tc);
    bool flip = channel_flip(tc);

    float shape = channel_shape(tc);
	o.r = osc(shape, flip ? uv.y : uv.x, freq, drift, phase);
    o.g = osc(shape, flip ? uv.x + uv.y : uv.x - uv.y, freq, drift, phase);
    o.b = osc(shape, length(uv), freq, drift, phase);

    if (channel_keyEnabled(tc)) {
        o.rgb = step(vec3(channel_key(tc)), o.rgb);
    }
    
    o.a = 1;
}

/*
  float orient = channel_orient(tc);
  float res;
    if (orient < .5) {
        res = mix(
            osc(shape, flip ? uv.y : uv.x, freq, drift, phase),
            osc(shape, flip ? uv.x + uv.y : uv.x - uv.y, freq, drift, phase),
            orient * 2.
        );
    } else {
        res = mix(
            osc(shape, flip ? uv.x + uv.y : uv.x - uv.y, freq, drift, phase),
            osc(shape, length(uv), freq, drift, phase),
            (orient - .5) * 2
        );
    }

*/
