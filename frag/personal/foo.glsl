#version 410

layout (location = 0) out vec4 FragColor;

#pragma channel float offset
#pragma channel vec3 img0

uniform float iTime;

in vec2 texcoord;

void main() {
    vec3 img = channel_img0(texcoord);
    FragColor.rgb = sin(img + channel_offset(texcoord) + iTime) * .5 + .5;
    FragColor.a = 1;
}
