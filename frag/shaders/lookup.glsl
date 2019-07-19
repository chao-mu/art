#version 410

layout (location = 0) out vec4 o;

#pragma channel float x
#pragma channel float y
#pragma channel vec3 img0

in vec2 texcoord;

void main() {
    phaseo.rgb = channel_img0(vec2(channel_x(texcoord), channel_y(texcoord)));
    o.a = 1;
}
