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

#define PI acos(-1)

#define TIME (iTime * .5)

// Only guarenteed to work on opengl 4.1
const float INF = 1 / 0.;

mat2 rotate(float a) {
    return mat2(cos(a), sin(a), -sin(a), cos(a));
}

vec2 radial(vec2 p, int cells, out int id) {
	// Polar coordinates
	float r = length(p);
	float a = atan(p.y, p.x);

    float rep = PI * 2. / float(cells);
    // Rotate the cell location
    a = a - TIME * .5 + p.x;
    id = int(mod(floor(a / rep), cells));
    a = mod(a, rep);
    r = mod(r, .8);

    return vec2(r * cos(a), r * sin(a));
}


vec2 to_texcoord(vec2 uv) {
    return ((uv * iResolution.y) + iResolution * .5) / iResolution;
}

void main() {
    vec2 uv = (gl_FragCoord.xy - iResolution * .5) / iResolution.y;

    uv = uv * uv;

    /*
    const int cells = 3;
    vec3 C[cells];
    C[0] = vec3(83, 52, 28) / 255.;
    C[1] = vec3(207, 93, 12) / 255.;
    C[2] = vec3(100, 30, 70) / 255.;
    */

    const int cells = 5;
    vec3 C[cells];
    C[0] = vec3(242,34,255) / 255.;
    C[1] = vec3(255,211,25) / 255.;
    C[2] = vec3(255,41,117) / 255.;
    C[3] = vec3(255,144,31) / 255.;
    C[4] = vec3(140,30,255) / 255.;

    int id;
    uv = radial(uv, cells, id);
    uv *= rotate(-TIME);
    vec3 col = texture(img0, to_texcoord(uv)).rgb;

    float mono = col.r;
    col = mix(col, C[id], mono);

    FragColor.rgb =  col;
    FragColor.a = 1;
}
