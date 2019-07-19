#version 410

layout (location = 0) out vec4 o;

#pragma channel vec3 img0

in vec2 tc;

void main() {
    vec3 col = channel_img0(tc);
    
    if (length(col) < 0.0001) {
        o.rgb = vec3(1);
    } else {
        o.rgb = vec3(0);
    }
    
    o.a = 1;
}
