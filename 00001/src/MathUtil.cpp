#include "MathUtil.h"

#include <cmath>
#include <algorithm>

#define EPSILON 0.0000001

float remap(float value, float in_min, float in_max, float out_min, float out_max) {
    if (fabs(in_min - in_max) < EPSILON){
        return out_min;
    } else {
        return ((value - in_min) / (in_max - in_min) * (out_max - out_min) + out_min);
    }
}

DrawInfo DrawInfo::scaleCenter(float src_width, float src_height, float dest_width, float dest_height) {
    float scale = std::min(dest_width/src_width, dest_height/src_height);

    float new_width = src_width * scale;
    float new_height = src_height * scale;

    DrawInfo info;
    // Bottom left
    info.x0 = static_cast<int>((dest_width - new_width) / 2.0);
    info.y0 = static_cast<int>((dest_height - new_height) / 2.0);
    info.x1 = info.x0 + static_cast<int>(new_width);
    info.y1 = info.y0 + static_cast<int>(new_height);

    return info;
}

#undef EPSILON
