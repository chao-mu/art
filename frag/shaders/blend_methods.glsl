#version 410

layout (location = 0) out vec4 o;

#pragma channel vec3 a
#pragma channel vec3 b
#pragma channel float mix_phoenix 0
#pragma channel float mix_hard_mix 0
#pragma channel float mix_reflecty 0

#pragma include include/ColorSpaces.inc.glsl

#define blendEach(f, base, blend) vec3(f(base.r, blend.r), f(base.g, blend.g), f(base.b, blend.b)) 

in vec2 tc;

uniform float iTime;

vec3 blendAdd(vec3 a, vec3 b) {
    return min(a + b, vec3(1));
}

vec3 blendAvg(vec3 a, vec3 b) {
    return (a + b) * .5;
}

float blendColorBurn(float a, float b) {
    if (b == 0.) {
        // Avoid divison by zero
        return a;
    } else {
        return max(
            1. - ((1. - a) / b),
            0.
        );
    }
}

vec3 blendColorBurn(vec3 a, vec3 b) {
    return blendEach(blendColorBurn, a, b);
}

float blendColorDodge(float a, float b){
	return (b==1.0)?b:min(a/(1.0-b),1.0);
}

vec3 blendColorDodge(vec3 a, vec3 b) {
    return blendEach(blendColorDodge, a, b);
}

vec3 blendDarken(vec3 a, vec3 b) {
	return min(a, b);
}

vec3 blendDiff(vec3 a, vec3 b) {
	return abs(a - b);
}

vec3 blendExclusion(vec3 base, vec3 blend) {
	return base+blend-2.0*base*blend;
}

float blendOverlay(float base, float blend) {
	return base<0.5?(2.0*base*blend):(1.0-2.0*(1.0-base)*(1.0-blend));
}

vec3 blendOverlay(vec3 base, vec3 blend) {
	return blendEach(blendOverlay, base, blend);
}

vec3 blendHardLight(vec3 base, vec3 blend) {
	return blendOverlay(blend,base);
}

float blendLighten(float base, float blend) {
	return max(blend,base);
}

vec3 blendLighten(vec3 a, vec3 b) {
    return blendEach(blendLighten, a, b);
}

float blendLinearBurn(float base, float blend) {
	return max(base+blend-1.0,0.0);
}

vec3 blendLinearBurn(vec3 a, vec3 b) {
    return blendEach(blendLinearBurn, a, b);
}

float blendLinearDodge(float base, float blend) {
	// Note : Same implementation as BlendAddf
	return min(base+blend,1.0);
}

vec3 blendLinearDodge(vec3 base, vec3 blend) {
	// Note : Same implementation as BlendAdd
	return min(base+blend,vec3(1.0));
}

float blendLinearLight(float base, float blend) {
	return blend<0.5?blendLinearBurn(base,(2.0*blend)):blendLinearDodge(base,(2.0*(blend-0.5)));
}

vec3 blendLinearLight(vec3 a, vec3 b) {
    return blendEach(blendLinearLight, a, b);
}


vec3 blendMultiply(vec3 base, vec3 blend) {
        return base*blend;
}

vec3 blend_phoenix(vec3 base, vec3 blend) {
	return min(base,blend)-max(base,blend)+vec3(1.0);
}

float blendPinLight(float base, float blend) {
	return (blend<0.5)?min(base,(2.0*blend)):max(base,(2.0*(blend-0.5)));
}

vec3 blendPinLight(vec3 a, vec3 b) {
    return blendEach(blendPinLight, a, b);
}

float blend_reflecty(float base, float blend) {
	return (blend==1.0)?blend:min(base*base/(1.0-blend),1.0);
}

vec3 blend_reflecty(vec3 a, vec3 b) {
    return blendEach(blend_reflecty, a, b);
}

float blendScreen(float base, float blend) {
        return 1.0-((1.0-base)*(1.0-blend));
}

vec3 blendScreen(vec3 a, vec3 b) {
    return blendEach(blendScreen, a, b);
}


vec3 blendSubtract(vec3 base, vec3 blend) {
	return max(base+blend-vec3(1.0),vec3(0.0));
}

float blendVividLight(float base, float blend) {
	return (blend<0.5)?blendColorBurn(base,(2.0*blend)):blendColorDodge(base,(2.0*(blend-0.5)));
}

vec3 blendVividLight(vec3 a, vec3 b) {
    return blendEach(blendVividLight, a, b);
}

float blend_hard_mix(float base, float blend) {
	return (blendVividLight(base,blend)<0.5)?0.0:1.0;
}

vec3 blend_hard_mix(vec3 a, vec3 b) {
    return blendEach(blend_hard_mix, a, b);
}

vec3 blendGlow(vec3 base, vec3 blend) {
	return blend_reflecty(blend,base);
}

#define blend_it(method) o.rgb = mix(o.rgb, blend_ ## method(o.rgb, b), channel_mix_ ## method())

void main() {
    vec3 a = channel_a();
    vec3 b = channel_b();
    
    //o.rgb = blend_hard_mix(a, b);
    // Subtle
    // o.rgb = blend_reflecty(a, b);
    // o.rgb = blend_phoenix(a, b);
    o.rgb = a;
    
    blend_it(hard_mix);
    blend_it(reflecty);
    blend_it(phoenix);

    o.a = 1;
}

/* Parts of this code are based on https://github.com/jamieowen/glsl-blend which has the following licnese:

The MIT License (MIT) Copyright (c) 2015 Jamie Owen

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
