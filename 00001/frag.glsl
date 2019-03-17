#version 410

layout (location = 0) out vec4 FragColor;

// width and height of image
uniform vec2 iResolution;

// The image texture
uniform sampler2D img0;

// The last output of this shader
uniform sampler2D lastOut;

// Whether or not this is the first time this shader is being run
uniform bool firstPass;

in vec2 texcoord;

// Random number in the range 0 >= x < 1
float rand(vec2 v) {
    return fract(sin(dot(v, vec2(23.405, 45.324))) * 3405.563);
}

// Based on code from The Book of Shaders
// Perlin noise.
float noise(vec2 v) {
    vec2 i = floor(v);
    vec2 f = fract(v);

    // Four corners in 2D of a tile
    float a = rand(i);
    float b = rand(i + vec2(1.0, 0.0));
    float c = rand(i + vec2(0.0, 1.0));
    float d = rand(i + vec2(1.0, 1.0));

    vec2 u = smoothstep(0., 1., f);

    // Mix 4 corners percentages
    return mix(a, b, u.x) +
            (c - a)* u.y * (1.0 - u.x) +
            (d - b) * u.x * u.y;
}

// The vector field we use to modify the image;
vec2 vfield() {
    vec2 uv = (gl_FragCoord.xy - .5 * iResolution) / iResolution.y;

    float x = noise(uv * 10);

    return vec2(cos(x), sin(x));
}

void main() {
    vec2 texcoord = gl_FragCoord.xy / iResolution;
    if (firstPass) {
        FragColor = texture(img0, texcoord);
        return;
    }

    vec3 col = texture(lastOut, texcoord).rgb;
    vec2 uv = (texcoord - .5) * 2.;

    vec2 o = vfield();
    vec2 rUv = mix(uv, uv * o, 0.001);
    vec2 gUv = mix(uv, uv * o, 0.002);
    vec2 bUv = mix(uv, uv * o, 0.003);

    FragColor = vec4(
        texture(lastOut, rUv * .5 + .5).r,
        texture(lastOut, gUv * .5 + .5).g,
        texture(lastOut, bUv * .5 + .5).b,
        1
    );
}
