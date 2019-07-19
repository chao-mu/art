#version 410

layout (location = 0) out vec4 o;

#pragma channel vec3 img0

in vec2 uv;

void main() {
    vec2 tc = uv;
    tc.x /= img0_res.x / img0_res.y;
    tc += .5;

    o = vec4(channel_img0(tc), 1);
}
