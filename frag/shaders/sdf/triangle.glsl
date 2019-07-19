#version 410

#pragma include ../util.inc

layout (location = 0) out vec4 o;

#pragma channel float size 1
#pragma channel vec2 coord

in vec2 texcoord;

void main() {
    vec2 p = to_1to1(channel_coord(texcoord));
    
    p *= 6 * channel_size(texcoord);

    // iq - https://www.iquilezles.org/www/articles/distfunctions2d/distfunctions2d.htm
    const float k = sqrt(3.0);
    p.x = abs(p.x) - 1.0;
    p.y = p.y + 1.0/k;
    if( p.x + k*p.y > 0.0 ) p = vec2(p.x-k*p.y,-k*p.x-p.y)/2.0;
    p.x -= clamp( p.x, -2.0, 0.0 );
    float d = -length(p)*sign(p.y);
    
    o.rgba = vec4(vec3(d), 1);
}
