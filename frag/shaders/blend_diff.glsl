#version 410

layout (location = 0) out vec4 o;

#pragma channel vec3 a
#pragma channel vec3 b

in vec2 tc;

void main() {
    o.rgb = abs(channel_a(tc) - channel_b(tc));
    o.a = 1;
}
