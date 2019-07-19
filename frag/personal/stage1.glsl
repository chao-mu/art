#version 410

layout (location = 0) out vec4 o;

#pragma channel vec3 rgb0

#pragma channe float high 1
#pragma channe float low 0

in vec2 texcoord;

void main() {
    o = smoothstep(1, 0, channel_rgb0(texcoord));
}
