#version 410

layout (location = 0) out vec4 FragColor;

// Time in seconds
uniform float iTime;

// width and height of image
uniform vec2 iResolution;

// The image texture
uniform sampler2D img0;

// The last output of this shader
uniform sampler2D lastOut;

// Whether or not this is the first time this shader is being run
uniform bool firstPass;

in vec2 texcoord;

void main() {
    vec2 texcoord = gl_FragCoord.xy / iResolution;
    if (firstPass) {
        FragColor = texture(img0, texcoord);
        return;
    }

    vec4 col = texture(img0, texcoord);
    vec2 uv = (texcoord - .5) * 2.;

    uv *= 1.2;

    vec4 col2 = texture(lastOut, uv * .5 + .5);

    FragColor = mix(col, col2, -1);
}
