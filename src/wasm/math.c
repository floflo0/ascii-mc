#include <math.h>

float fminf(const float x, const float y) {
    if (x < y) return x;
    return y;
}

float fmaxf(const float x, const float y) {
    if (x < y) return y;
    return x;
}
