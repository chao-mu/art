#version 410

layout (location = 0) out vec4 o;

#pragma channel vec3 a
#pragma channel vec3 b
#pragma channel float swap 0

bool near(float x, float y) {
    return abs(x - y) < 0.05;
}

bool is_bg(vec3 col) {
    return near(length(col), 0) ||
        (
         near(col.r, 0.572549) &&
         near(col.g, 0.56470) &&
         near(col.b, 1.0)
        );
}

vec3 calc(vec3 a, vec3 b) {
    vec3 c;
    if (is_bg(a)) {
        c = b;
    } else {
        c = a;
    }
 
    return c;
}

void main() {
    vec3 a = channel_a();
    vec3 b = channel_b();
    
    o.rgb = mix(calc(a, b), calc(b, a), channel_swap());
    o.a = 1;
}
