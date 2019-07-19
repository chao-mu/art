#version 410

layout (location = 0) out vec4 o;

#pragma channel vec3 a
#pragma channel vec3 b
#pragma channel float x 0
#pragma channel float y 0

in vec2 tc;

void main() {
    vec3 a = channel_a();
    vec2 off = vec2(channel_x(), channel_y());
    o.rgb = mix(a, channel_b(b_tc - off), 1 - length(a));
    o.a = 1;
}
