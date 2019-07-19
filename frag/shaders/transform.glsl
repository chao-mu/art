#version 410

layout (location = 0) out vec4 o;

#pragma include util.inc

#pragma channel vec3 img0
#pragma channel float rot 0
#pragma channel float scale 1
#pragma channel float offsetX 0 
#pragma channel float offsetY 0

#define TAU (3.1415926 * 2.)
#define tc texcoord

in vec2 uv;
in vec2 texcoord;

mat2 rot(float a) {
    return mat2(cos(a), sin(a), -sin(a), cos(a));
}

void main() {
    vec2 st = uv * rot(channel_rot(tc) * TAU);
    st += vec2(channel_offsetX(tc), channel_offsetY(tc));
    st /= channel_scale(tc);
    
    o.rgb = channel_img0(to_0to1(st));
    o.a = 1;
}
