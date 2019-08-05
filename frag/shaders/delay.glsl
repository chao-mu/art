#version 410

out vec4 o;

#pragma channel vec3 img0
#pragma channel float mix 0

uniform bool firstPass;
uniform sampler2D lastOut;
uniform float iTime;

in vec2 tc;

void main() {
    vec3 cur = channel_img0();
    vec3 last = firstPass ? cur : texture(lastOut, tc).rgb;
    
    o.rgb = mix(cur, last, channel_mix());
    o.a = 1;
}
