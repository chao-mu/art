#version 410

layout (location = 0) out vec4 o;

#pragma channel float r 0
#pragma channel float g 0
#pragma channel float b 0

in vec2 texcoord;

void main() {
    o.r = channel_r(texcoord);
    o.g = channel_g(texcoord);
    o.b = channel_b(texcoord);
    o.a = 1;
}

