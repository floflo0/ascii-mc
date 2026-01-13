#pragma once

typedef struct {
    int x, y;
} v2i;

typedef struct {
    float x, y;
} v2f;

typedef struct {
    int x, y, z;
} v3i;

typedef struct {
    float x, y, z;
} v3f;

typedef union {
    struct {
        float x, y, z, w;
    };
    v3f xyz;
} v4f;

typedef float m4f[16];
