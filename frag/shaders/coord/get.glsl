#version 410

layout (location = 0) out vec4 o;

in vec2 texcoord;

void main() {
    o.rg = texcoord;
    o.ba = vec2(1);
}
