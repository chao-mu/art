#version 410

out vec4 o;

#pragma include include/ColorSpaces.inc.glsl

#pragma channel vec3 img0 
#pragma channel float gamma 1
#pragma channel float gamma_inc 0
#pragma channel float gamma_dec 0
#pragma channel float contrast 1
#pragma channel float contrast_inc 0
#pragma channel float contrast_dec 0
#pragma channel float brightness 0
#pragma channel bool split false

in vec2 uv;

// From https://alaingalvan.tumblr.com/post/79864187609/glsl-color-correction-shaders
vec3 Gamma(vec3 value, float param)
{
    return vec3(pow(abs(value.r), param),pow(abs(value.g), param),pow(abs(value.b), param));
}

// From https://alaingalvan.tumblr.com/post/79864187609/glsl-color-correction-shaders
vec3 brightnessContrast(vec3 value, float brightness, float contrast)
{
    return (value - 0.5) * contrast + 0.5 + brightness;
}

void main() {
    vec3 col = channel_img0();
    col = brightnessContrast(col, channel_brightness(), channel_contrast() + channel_contrast_inc() - channel_contrast_dec());
    col = Gamma(col, channel_gamma() + channel_gamma_inc() - channel_gamma_dec());
    o.rgb = channel_split() && uv.x < 0 ? channel_img0() : col;
    o.a = 1;
}
