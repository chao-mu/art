#version 410

out vec4 o;

#pragma channel vec3 img0
#pragma channel float mix 0

#pragma include include/ColorSpaces.inc.glsl

uniform float iTime;

void main() {
    vec3 rgb = channel_img0();
    vec3 hsv = rgb_to_hsv(rgb);
    hsv.z = abs(sin(hsv.z * 10));
    hsv.x = abs(sin(hsv.x * 10 + iTime));
    o.rgb = mix(rgb, hsv_to_rgb(hsv), channel_mix());
}
