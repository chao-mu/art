#version 410

layout (location = 0) out vec4 FragColor;

#pragma include ../shaders/util.inc

// Image
#pragma channel vec3 img0
#pragma channel vec2 noise vec2(0, 0)
#pragma channel float a 0.5
#pragma channel float b 0.51

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

vec3 applyKernel(in mat3 m) {
    vec2 noise = channel_noise(texcoord);
    return m[0][2] * channel_img0(texcoordBL + noise) +
        m[1][2] * channel_img0(texcoordB) +
        m[2][2] * channel_img0(texcoordBR) +
        m[0][1] * channel_img0(texcoordL) +
        m[1][1] * channel_img0(texcoord) +
        m[2][1] * channel_img0(texcoordR) +
        m[0][0] * channel_img0(texcoordTL) +
        m[1][0] * channel_img0(texcoordT) +
        m[2][0] * channel_img0(texcoordTR);
}

#define KERNEL_SCHARR_Y \
    mat3( \
        3, 10, 3, \
        0, 0, 0, \
        -3, -10, -3 \
    )

#define KERNEL_SCHARR_X \
    mat3( \
        3, 0, -3, \
        10, 0, -10, \
        3, 0, -3 \
    )

void main() {
    float edge_y = dot(applyKernel(KERNEL_SCHARR_X), vec3(1));
    float edge_x = dot(applyKernel(KERNEL_SCHARR_Y), vec3(1));
    if (edge_x < channel_a(texcoord)) {
        FragColor.rgb = channel_img0(texcoord + vec2(0, 1 / iResolution.y));
    } else if (edge_y < channel_b(texcoord)) {
        FragColor.rgb = channel_img0(texcoord + vec2(1 / iResolution.x, 0));
    } else {
        FragColor.rgb = vec3(1);//channel_img0(texcoord);
    }
    

    FragColor.a = 1;
}
