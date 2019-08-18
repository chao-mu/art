#version 410

out vec4 o;

#pragma channel vec3 master
#pragma channel vec3 img0
#pragma channel float mix_1 0
#pragma channel float mix_2 0

#pragma include include/ColorSpaces.inc.glsl

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


vec3 effect_1(vec3 rgb) {
    vec3 hsv = rgb_to_hsv(rgb);
    return mix(rgb, hsv_to_rgb(abs(vec3(0., 1, 1) - hsv)), -2);
}

vec3 effect_2(vec3 rgb) {
    vec3 hsv = rgb_to_hsv(rgb);
    hsv.z = abs(sin(hsv.z * 10));
    hsv.x = abs(sin(hsv.x * 10 + iTime));

    return hsv_to_rgb(hsv);
}

vec3 effect_3(vec3 rgb) {
    rgb += sin(rgb * 19);
    return rgb;
}

void main() {
    vec3 rgb = channel_img0();
    vec3 blur = applyKernel(KERNEL_BOX_BLUR).rgb / .9;
    o.rgb = mix(channel_master(), effect_1(rgb), channel_mix_1());
    o.rgb = mix(o.rgb, effect_2(o.rgb), channel_mix_2());
    o.rgb = blur;
}
