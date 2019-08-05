#version 410

out vec4 o;

#pragma channel vec3 img0
#pragma channel float mix 0

#pragma include include/ColorSpaces.inc.glsl

void main() {
    vec3 rgb = channel_img0();
    o.rgb = mix(rgb, vec3(1) - rgb, channel_mix());
}
