#version 410

out vec3 o;

#define PI 3.1415926

#pragma channel vec3 img0 vec3(0)
#pragma channel vec3 a vec3(0)
#pragma channel vec3 b vec3(0)
#pragma channel vec3 c vec3(1)
#pragma channel vec3 d vec3(1)

in vec2 uv;
in vec2 tc;

#pragma include ../shaders/util.inc

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

float applyKernel(in mat3 m, int i) {
    return m[0][2] * rgb_to_hsv(channel_img0(texcoordBL))[i] +
        m[1][2] * rgb_to_hsv(channel_img0(texcoordB))[i] +
        m[2][2] * rgb_to_hsv(channel_img0(texcoordBR))[i] +
        m[0][1] * rgb_to_hsv(channel_img0(texcoordL))[i] +
        m[1][1] * rgb_to_hsv(channel_img0(texcoord))[i] +
        m[2][1] * rgb_to_hsv(channel_img0(texcoordR))[i] +
        m[0][0] * rgb_to_hsv(channel_img0(texcoordTL))[i] +
        m[1][0] * rgb_to_hsv(channel_img0(texcoordT))[i] +
        m[2][0] * rgb_to_hsv(channel_img0(texcoordTR))[i];
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

bool eq(vec3 a, vec3 b) {
    return length(a - b) < 0.1;
}

float edge(int i) {
    float a = applyKernel(KERNEL_SCHARR_X, i);
    float b = applyKernel(KERNEL_SCHARR_Y, i);

    return sqrt((a * a) + (b * b));
}

/*
void main() {
    o.rgb = vec3(smoothstep(0.4, 1., edge(2)));
}
*/

void main() {
    vec3 orig = channel_img0();
    vec3 hsv = rgb_to_hsv(orig);

    //hsv.x = 1 - hsv.x;
    hsv.y = abs(sin(hsv.y * PI + iTime) * edge(2));

    o.rgb = hsv_to_rgb(hsv);
}
