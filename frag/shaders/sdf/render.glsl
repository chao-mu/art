#version 410

layout (location = 0) out vec4 o;

#pragma channel float sdf0 0
#pragma channel vec3 col0 vec3(0)

in vec2 texcoord;

float render(float dist) {
    return smoothstep(0.1, 0.1 - fwidth(dist), dist);
}

void main() {
    o.rgb = vec3(0);
    o.rgb += render(channel_sdf0(texcoord)) * channel_col0(texcoord);
    o.a = 1;
}
