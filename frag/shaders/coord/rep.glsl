#version 410

layout (location = 0) out vec4 o;

#pragma include ../util.inc

#pragma channel vec2 coord
#pragma channel float c 0.5;

in vec2 texcoord;

void main() {
    float c_ = channel_c(texcoord);
    vec2 uv = to_1to1(channel_coord(texcoord));

    o.rg = to_0to1(mod(uv, c_) - .5 * c);
    o.ba = vec2(1);
}
