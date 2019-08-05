#version 410

layout (location = 0) out vec4 o;

#pragma channel vec3 a
#pragma channel vec3 b
#pragma channel float mix 0

in vec2 tc;

uniform float iTime;

float rand(float x) {
    return fract(sin(x * 1234.5434) * 3446.7453);
}

float noise(float x) {
    float i = floor(x);
    float f = fract(x);

    return mix(rand(i), rand(i + 1), f);
}

void main() {
    float x = tc.x * noise(tc.x * 10 + iTime);
    float n = noise(x * 10 - iTime);
    
    o.rgb = n > channel_mix() ? channel_a() : channel_b();
    o.a = 1;
}
