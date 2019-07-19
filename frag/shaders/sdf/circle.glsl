#version 410

layout (location = 0) out vec4 o;

#pragma include ../util.inc

#pragma channel float radius .2
#pragma channel vec2 coord

in vec2 texcoord;

void main() {
    vec2 uv = to_1to1(channel_coord(texcoord));

    float d = length(uv) - channel_radius(texcoord);
    
    o.rgba = vec4(vec3(d), 1);
}
