#version 410

out vec4 o;

#define PI 3.1415926

#pragma channel vec3 img0
#pragma channel float amount

#pragma include ../shaders/util.inc

in vec2 uv;

void main() {
    float s = sin(2 * PI * length(uv) + PI) * .5 + .5;
    s = mix(1, s + .2, channel_amount());
        
    o.rgb = channel_img0(to_tc(uv / s, img0_res));
    o.a = 1;
}
