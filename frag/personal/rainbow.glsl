#version 410

layout (location = 0) out vec4 FragColor;

in vec2 uv;

uniform float iTime;

uniform bool rotate = false;

#define TWO_PI (2. * 3.1415926)

//  Function from Iñigo Quiles
//  https://www.shadertoy.com/view/MsS3Wc
vec3 hsb2rgb( in vec3 c ){
    vec3 rgb = clamp(abs(mod(c.x*6.0+vec3(0.0,4.0,2.0),
                             6.0)-3.0)-1.0,
                     0.0,
                     1.0 );
    rgb = rgb*rgb*(3.0-2.0*rgb);
    return c.z * mix( vec3(1.0), rgb, c.y);
}


mat2 rot(float a) {
    return mat2(cos(a), sin(a), -sin(a), cos(a));
}


void main() {
    vec2 st = uv;
    if (rotate) {
        st *= rot(iTime);
    }

    float a = atan(st.y, st.x);
    float r = length(st) * 2.;
    vec3 hsb = vec3((a/TWO_PI)+0.5,r,1.0);

	FragColor.rgb = hsb2rgb(hsb);
	FragColor.a = 1;
}
