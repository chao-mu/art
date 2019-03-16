#version 410

layout (location = 0) out vec4 FragColor;

uniform float iTime;
uniform vec2 iResolution;

uniform sampler2D img0;
uniform sampler2D lastOut;
uniform bool firstPass;

in vec2 texcoord;

#define MAX_STEPS 100

mat2 rotate(float a) {
    return mat2(cos(a), -sin(a), sin(a), cos(a));
}

void main() {
    vec2 texcoord = gl_FragCoord.xy / iResolution;
    if (firstPass) {
        FragColor = texture(img0, texcoord);
    } else {
        vec3 col = texture(lastOut, texcoord).rgb;
        FragColor.rgb = col;
    }

    FragColor.a = 1;
}
