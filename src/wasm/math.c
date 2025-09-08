#include <assert.h>
#include <math.h>

#include "js.h"

float fminf(const float x, const float y) {
    if (x < y) return x;
    return y;
}

float fmaxf(const float x, const float y) {
    if (x < y) return y;
    return x;
}

double pow(const double x, const double y) {
    return JS_Math_pow(x, y);
}

float powf(const float x, const float y) {
    return JS_Math_pow(x, y);
}

double exp2(const double x) {
    return JS_Math_pow(x, 2.0);
}

float cosf(const float x) {
    return JS_Math_cos(x);
}

float sinf(const float x) {
    return JS_Math_sin(x);
}

float tanf(const float x) {
    return JS_Math_tan(x);
}

float atanf(const float x) {
    return JS_Math_atan(x);
}

float roundf(const float x) {
    return JS_Math_round(x);
}

double fmod(const double x, const double y) {
    return JS_fmod(x, y);
}

float fmodf(const float x, const float y) {
    return JS_fmod(x, y);
}

int isnanf(const float x) {
    return JS_isNaN(x);
}
