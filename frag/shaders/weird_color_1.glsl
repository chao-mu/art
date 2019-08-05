#version 410

out vec4 o;

#pragma channel vec3 img0
#pragma channel float mix 0

#pragma include include/ColorSpaces.inc.glsl

void main() {
    vec3 rgb = channel_img0();
    vec3 hsv = rgb_to_hsv(rgb);
    o.rgb = mix(rgb, mix(rgb, hsv_to_rgb(abs(vec3(0., 1, 1) - hsv)), -2), channel_mix());
}
