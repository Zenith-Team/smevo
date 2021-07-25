#pragma once

#include "types.h"

class Mtx34 {
public:
    static Mtx34* sIdentity;

    union {
        f32 rows[3][4];
        f32 cells[12];
    };

    void rotateAndTranslate(const Vec3u& rRotation, const Vec3f& rTranslation);
    static void makeST(Mtx34& rMtx, const Vec3f& rScale, const Vec3f& rTranslation);
    static void makeSRT(Mtx34& rMtx, const Vec3f& rScale, const Vec3f& rRotation, const Vec3f& rTranslation);
};

class Mtx44 {
public:
    union {
        f32 rows [4][4];
        f32 cells[16];
    };
};

// TODO: Do this properly(?)
extern "C" void ASM_MTXConcat(const Mtx34* p1, const Mtx34* p2, Mtx34* pOut);

inline Mtx34* MTX34MULT(Mtx34* pOut, const Mtx34* p1, const Mtx34* p2) {
    ASM_MTXConcat(p1, p2, pOut);
    return pOut;
}