uniform vec2 iResolution;

vec2 to_1to1(in vec2 uv) {
    uv -= .5;
    uv.x *= iResolution.x / iResolution.y;
    
    return uv;
}

vec2 to_tc(vec2 uv, in vec2 res) {
    uv.x /= res.x / res.y;
    uv += .5;
    
    return uv;
}

vec2 to_0to1(in vec2 uv) {
    return to_tc(uv, iResolution);
}

