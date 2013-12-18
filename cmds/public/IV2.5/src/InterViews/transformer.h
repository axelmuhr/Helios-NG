/*
 * Interface to transformation matrices.
 */

#ifndef transformer_h
#define transformer_h

#include <InterViews/defs.h>
#include <InterViews/resource.h>

class Transformer : public Resource {
    float mat00, mat01, mat10, mat11, mat20, mat21;

    float Det(Transformer* t);
public:
    Transformer(Transformer* t =nil);	    // returns identity if t == nil
    Transformer(
	float a00, float a01, float a10, float a11, float a20, float a21
    );

    void GetEntries(
	float& a00, float& a01, float& a10, float& a11, float& a20, float& a21
    );
    void Premultiply(Transformer* t);
    void Postmultiply(Transformer* t);
    void Invert();

    void Translate(float dx, float dy);
    void Scale(float sx, float sy);
    void Rotate(float angle);
    boolean Translated (float = 1e-6);
    boolean Scaled (float = 1e-6);
    boolean Stretched (float = 1e-6);
    boolean Rotated (float = 1e-6);
    boolean Rotated90 (float = 1e-6);

    void Transform(Coord& x, Coord& y);
    void Transform(Coord x, Coord y, Coord& tx, Coord& ty);
    void Transform(float x, float y, float& tx, float& ty);
    void TransformList(Coord x[], Coord y[], int n);
    void TransformList(Coord x[], Coord y[], int n, Coord tx[], Coord ty[]);
    void InvTransform(Coord& tx, Coord& ty);
    void InvTransform(Coord tx, Coord ty, Coord& x, Coord& y);
    void InvTransform(float tx, float ty, float& x, float& y);
    void InvTransformList(Coord tx[], Coord ty[], int n);
    void InvTransformList(Coord tx[], Coord ty[], int n, Coord x[], Coord y[]);

    boolean operator == (Transformer&);
    boolean operator != (Transformer&);
    Transformer& operator = (Transformer&);
};

inline float Transformer::Det (Transformer *t) {
    return t->mat00*t->mat11 - t->mat01*t->mat10;
}
inline boolean Transformer::Translated (float tol) {
    return -tol > mat20 || mat20 > tol || -tol > mat21 || mat21 > tol;
}
inline boolean Transformer::Scaled (float tol) {
    float l = 1 - tol, u = 1 + tol;
    return l > mat00 || mat00 > u || l > mat11 || mat11 > u;
}
inline boolean Transformer::Stretched (float tol) {
    float diff = mat00 - mat11;
    return -tol > diff || diff > tol;
}
inline boolean Transformer::Rotated (float tol) {
    return -tol > mat01 || mat01 > tol || -tol > mat10 || mat10 > tol;
}
inline boolean Transformer::Rotated90 (float tol) {
    return Rotated(tol) && -tol <= mat00 && mat00 <= tol &&
        -tol <= mat11 && mat11 <= tol;
}
inline void Transformer::Transform (Coord x, Coord y, Coord& tx, Coord& ty) {
    tx = round(x*mat00 + y*mat10 + mat20);
    ty = round(x*mat01 + y*mat11 + mat21);
}
inline void Transformer::Transform (float x, float y, float& tx, float& ty) {
    tx = x*mat00 + y*mat10 + mat20;
    ty = x*mat01 + y*mat11 + mat21;
}

#endif
