#version 410

out vec3 o;

#pragma channel vec3 img0 vec3(0)
#pragma channel vec3 a vec3(0)
#pragma channel vec3 b vec3(1)

in vec2 uv;
in vec2 tc;

#pragma include ../shaders/util.inc

uniform float iTime;

void main() {
    vec3 orig = channel_img0();
    vec3 orig_hsv = rgb_to_hsv(orig);
    
    vec3 a = channel_a();
    vec3 b = channel_b();
    
    vec3 m = mix(a, b, orig_hsv.z);
 
    vec3 m_hsv = rgb_to_hsv(m);
    m_hsv.z = orig_hsv.z;
    m = hsv_to_rgb(m_hsv);
    
    o.rgb = m;
}
