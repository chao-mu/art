#version 410

layout (location = 0) out vec4 FragColor;

// Image
uniform sampler2D img0;

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

vec4 applyKernel(sampler2D tex, in mat3 m) {
    return m[0][2] * texture(tex, texcoordBL) +
        m[1][2] * texture(tex, texcoordB) +
        m[2][2] * texture(tex, texcoordBR) +
        m[0][1] * texture(tex, texcoordL) +
        m[1][1] * texture(tex, texcoord) +
        m[2][1] * texture(tex, texcoordR) +
        m[0][0] * texture(tex, texcoordTL) +
        m[1][0] * texture(tex, texcoordT) +
        m[2][0] * texture(tex, texcoordTR);
}

#define KERNEL_BOX_BLUR \
    mat3( \
        1, 1, 1, \
        1, 1, 1, \
        1, 1, 1 \
    )

void main() {
    vec3 blur = applyKernel(img0, KERNEL_BOX_BLUR).rgb / .9;
    FragColor.rgba = vec4(blur, 1);
}
