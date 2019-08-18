#version 410

layout (location = 0) out vec4 o;

#pragma channel vec3 a
#pragma channel vec3 b
#pragma channel float mix_fade 0
#pragma channel float mix_bars 0
#pragma channel float mix_lumin_a 0
#pragma channel float mix_lumin_b 0
#pragma channel float mix_diff 0

#pragma include include/ColorSpaces.inc.glsl

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

vec3 mix_diff(vec3 a, vec3 b, float m) {
    return mix(a, abs(a - b), m);
}

vec3 mix_bars(vec3 a, vec3 b, float m) {
    float x = tc.x * noise(tc.x * 10 + iTime);
    float n = noise(x * 10 - iTime);
    
    return n > m ? a : b;
}


vec3 mix_lumin(vec3 a, vec3 b, float m) {
    return get_luminance(a) > m ? a : b;
}

void main() {
    vec3 a = channel_a();
    vec3 b = channel_b();
    o.rgb = mix_lumin(a, b, channel_mix_lumin_a());
    o.rgb = mix_lumin(b, o.rgb, 1 - channel_mix_lumin_b());
    o.rgb = mix_bars(o.rgb, b, channel_mix_bars());
    o.rgb = mix_diff(o.rgb, b, channel_mix_diff());
    o.rgb = mix(o.rgb, b, channel_mix_fade());
    
    o.a = 1;
}
