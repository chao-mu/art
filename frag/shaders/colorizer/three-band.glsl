#version 410

layout (location = 0) out vec4 o;

#pragma channel float lum 0

#pragma channel float low .333333
#pragma channel float mid .666666

#pragma channel vec3 col0 vec3(1)
#pragma channel vec3 col1 vec3(1)
#pragma channel vec3 col2 vec3(1)

in vec2 texcoord;

void main() {
    float lum = channel_lum(texcoord);
    
    float low = channel_low(texcoord);
    float mid = channel_mid(texcoord);
    
    if (lum <= low) {
        o.rgb = (lum + 1 - low)  * channel_col0(texcoord);
    } else if (lum <= mid) {
        o.rgb = (lum + 1 - mid)  * channel_col1(texcoord);
    } else {
        o.rgb = lum * channel_col2(texcoord);
    }
    
    o.a = 1;
}
