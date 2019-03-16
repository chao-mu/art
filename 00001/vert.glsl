#version 410

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 texcoord;

out vec2 texcoordL;
out vec2 texcoordR;
out vec2 texcoordT;
out vec2 texcoordTL;
out vec2 texcoordTR;
out vec2 texcoordB;
out vec2 texcoordBL;
out vec2 texcoordBR;

uniform vec2 iResolutionImg0;
uniform vec2 iResolution;

void main() {
    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);

	float widthStep = 1. / iResolution.x;
	float heightStep = 1. / iResolution.y;
	vec2 widthHeightStep = vec2(widthStep, heightStep);
	vec2 widthNegativeHeightStep = vec2(widthStep, -heightStep);

	vec2 uv = aTexCoord;

    texcoord = uv;
	texcoordL = uv.xy - widthStep;
	texcoordR = uv.xy + widthStep;

	texcoordT = uv.xy - heightStep;
	texcoordTL = uv.xy - widthHeightStep;
	texcoordTR = uv.xy + widthNegativeHeightStep;

	texcoordB = uv.xy + heightStep;
	texcoordBL = uv.xy - widthNegativeHeightStep;
	texcoordBR = uv.xy + widthHeightStep;
}
