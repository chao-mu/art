#version 410

layout (location = 0) out vec4 o;

#pragma channel float hue

in vec2 texcoord;

void main() {
    float h = channel_hue(texcoord);
    
    o.r = abs(h * 6 - 3) - 1;
    o.g = 2 - abs(h * 6 - 2);
    o.b = 2 - abs(h * 6 - 4);
}
