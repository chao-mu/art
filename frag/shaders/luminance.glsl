#version 410

layout (location = 0) out vec4 o;

#pragma channel vec3 rgb0

in vec2 texcoord;

void main() {
    o.rgb = vec3(dot(channel_rgb0(texcoord), vec3(0.299, 0.587, 0.114)));
    o.a = 1;
}
