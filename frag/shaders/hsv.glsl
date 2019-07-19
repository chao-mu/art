#version 410

layout (location = 0) out vec4 o;

#pragma channel vec4 rgb0

in vec2 texcoord;

void main() {
    o = channel_rgb0(texcoord);
}
