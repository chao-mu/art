#version 410

layout (location = 0) out vec4 o;

#pragma channel vec3 rgb0

#pragma channel float high 1
#pragma channel float low 0

#pragma channel float key .5
#pragma channel bool keyEnabled false
#pragma channel bool smoothEnabled false

in vec2 texcoord;

void main() {
    vec3 img = channel_rgb0(texcoord);
    float h = channel_high(texcoord);
    float l = channel_low(texcoord);
    
    if (channel_smoothEnabled(texcoord)) {
        o.rgb = smoothstep(l, h, img);
    }  else {
        for (int i = 0; i < 3; i++) {
            o[i] = img[i] <= h && img[i] >= l ? img[i] : 0;
        }
    }

    if (channel_keyEnabled(texcoord)) {
        o.rgb = step(channel_key(texcoord), o.rgb);
    }

    o.a = 1;
}
