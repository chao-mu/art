#version 410

layout (location = 0) out vec4 FragColor;

// Image
#pragma channel vec3 img0

uniform bool negate = false;

// width and height of image
uniform vec2 iResolution;

uniform float iTime;

in vec2 texcoord;
in vec2 texcoordL;
in vec2 texcoordR;
in vec2 texcoordT;
in vec2 texcoordTL;
in vec2 texcoordTR;
in vec2 texcoordB;
in vec2 texcoordBL;
in vec2 texcoordBR;

vec3 applyKernel(mat3 m) {
    return m[0][2] * channel_img0(texcoordBL) +
        m[1][2] * channel_img0(texcoordB) +
        m[2][2] * channel_img0(texcoordBR) +
        m[0][1] * channel_img0(texcoordL) +
        m[1][1] * channel_img0(texcoord) +
        m[2][1] * channel_img0(texcoordR) +
        m[0][0] * channel_img0(texcoordTL) +
        m[1][0] * channel_img0(texcoordT) +
        m[2][0] * channel_img0(texcoordTR);
}

#define KERNEL_BOX_BLUR \
    mat3( \
        1, 1, 1, \
        1, 1, 1, \
        1, 1, 1 \
    )

void main() {
    vec3 blur = applyKernel(KERNEL_BOX_BLUR).rgb / .9;
    FragColor.rgba = vec4(blur, 1);
}
