#version 410

out vec4 o;

#pragma channel bool x false
#pragma channel bool y false
#pragma channel bool z false

uniform sampler2D lastOut;

in vec2 tc;

/*
 * pressed - .6
 * pressed, consecutive on - .7
 * not pressed 
 * 
 * an off brings a press to armed state
 */


#define ARMED_ON 0.8
#define ARMED_OFF 0.2
#define OFF 0
#define ON 1

bool eq(float a, float b) {
    return abs(a - b) < 0.001;
}

void set(bool t, int i, vec3 last) {
    float l = last[i];
    if (t) {
        if (eq(l, ARMED_ON)) {
            o[i] = OFF;
        } else if (eq(l, ARMED_OFF)) {
            o[i] = ON;
        }
    } else {
        if (eq(l, ON)) {
            o[i] = ARMED_ON;
        } else if (eq(l, OFF)) {
            o[i] = ARMED_OFF;
        }
    }
}

void main() {
    vec3 last = texture(lastOut, tc).rgb;
    
    set(channel_x(), 0, last);
    set(channel_y(), 1, last);
    set(channel_z(), 2, last);
    
    o.a = 1;
}
