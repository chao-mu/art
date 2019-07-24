#version 410

out vec4 o;

#pragma channel vec3 img0
#pragma channel float knob 0
#pragma channel float mix 0

uniform bool firstPass;
uniform sampler2D lastOut;
uniform float iTime;

in vec2 tc;

vec3 coefs(float knob) {
    vec3 c = vec3(0);

    if (knob < 0.333) {
        c.x = 1;
    } else if (knob < 0.666) {
        c.y = 1;
    } else {
        c.z = 1;
    }
    
    return c;
}

void main() {
    vec3 cur = channel_img0();
    vec3 last = firstPass ? cur : texture(lastOut, tc).rgb;
    float knob = channel_knob();
    
    if (knob < 0.01 || length(cur - vec3(1, 0, 0)) < 0.7) {
        o.rgb = cur;
    } else {
        o.rgb = last;
    }
    
    o.a = 1;
}
