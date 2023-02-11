#ifndef _VECMATH_H
#define _VECMATH_H

struct float3 {
    float x;
    float y;
    float z;
};


struct float3 add_f3(struct float3 a, struct float3 b) {
    struct float3 res = {a.x + b.x, a.y + b.y, a.z + b.z};
    return res;
}

#endif // _VECMATH_H
