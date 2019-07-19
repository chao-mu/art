#version 410

layout (location = 0) out vec4 o;

#pragma channel vec3 a
#pragma channel vec3 b

uniform bool firstFrame;

in vec2 tc;

void main() {
    vec3 a = channel_a(tc);
    vec3 b = channel_b(tc);
    
    if (length(b) < 0.001) {
        o.rgb = a;
    } else {
        o.rgb = b;
    }

    o.a = 1;
}
