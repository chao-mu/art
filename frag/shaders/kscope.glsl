#version 410

#pragma include util.inc

#define PI 3.1415926
#define time iTime

out vec4 o;

#pragma channel vec3 img0
#pragma channel float tweak_1 0
#pragma channel float mix 0

uniform float iTime;

in vec2 uv;
in vec2 tc;

void main() {
    vec2 p = uv;
   
    float a = atan(p.y,p.x);
    float r = length(p);
    
    a = sin(a * (channel_tweak_1() * 7 + 1));
    
    p.x = r * cos(a);
    p.y = r * sin(a);
    
    p = mix(uv, p, channel_mix());
    vec3 col = channel_img0(to_tc(p, img0_res));

    o.rgba = vec4(col, 1.0);
}
