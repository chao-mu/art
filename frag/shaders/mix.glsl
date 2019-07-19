#version 410

layout (location = 0) out vec4 o;

#pragma channel vec3 a
#pragma channel vec3 b
#pragma channel float mix 0

in vec2 tc;

void main() {
    o.rgb = mix(channel_a(tc), channel_b(tc), channel_mix(tc));
    o.a = 1;
}
