#version 410

layout (location = 0) out vec4 FragColor;

#pragma channel vec3 a
#pragma channel vec3 b

in vec2 texcoord;

void main() {
    FragColor.rgb = channel_a(texcoord) * channel_b(texcoord);
    FragColor.a = 1;
}
