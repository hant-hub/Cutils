#ifndef GA_H
#define GA_H

#include <core/cutils.h>

//3D PGA R_{3,0,1}

// ae123 + be032 + ce013 + de021
// (b, c, d)
typedef struct GAPoint {
    f32 a; //e123
    f32 b; //e032
    f32 c; //e013
    f32 d; //e021
} GAPoint;

//(e23, e31, e12)
typedef struct GALine {
    f32 a; //e01
    f32 b; //e02
    f32 c; //e03
    f32 d; //e12
    f32 e; //e31
    f32 f; //e23
} GALine;

typedef struct GAIdealLine {
    f32 a; //e01
    f32 b; //e02
    f32 c; //e03
} GAIdealLine;

typedef struct GABranch {
    f32 a; //e12
    f32 b; //e31
    f32 c; //e23
} GABranch;

//bx + cy + dz + a = 0
//ae0 + be1 + ce2 + de3
typedef struct GAPlane {
    f32 a; //e0
    f32 b; //e1
    f32 c; //e2
    f32 d; //e3
} GAPlane;

typedef struct GAMultiVector {
} GAMultiVector;

//Intuitive Constructors

//(x, y, z)
GAPoint makePoint(f32 x, f32 y, f32 z);

//equivalent to point + direction
//L = (x,y,z) + (dx, dy, dz) * t
GALine makeLine(f32 x, f32 y, f32 z, f32 dx, f32 dy, f32 dz);

//ax + by + cz + d = 0
GAPlane makePlane(f32 a, f32 b, f32 c, f32 d);

//Conversion
GALine idealLineToLine(GAIdealLine l);

//Scalar Product
GAPoint scalePoint(GAPoint p, f32 s);
GALine scaleLine(GALine l, f32 s);
GAPlane scalePlane(GAPlane p, f32 s);

//Rejection
GAPlane rejectPlane(GAPlane p);
GALine rejectLine(GALine l);

//Metrics
f32 normLine(GALine l);
f32 normIdealLine(GALine l);

//Meets
GALine meetPlanes3(GAPlane a, GAPlane b);
GAPoint meetLinesPlane(GAPlane a, GALine b);

//Joins
GALine joinPoints3(GAPoint a, GAPoint b);
GAPlane joinPointLine(GAPoint a, GALine b);

//Renormalize points
GAPoint normPoint3(GAPoint p);

//Duals
GAPoint dualPlane3(GAPlane p);
GAPlane dualPoint3(GAPoint p);

//Reflect things over plane
GAPoint reflectPlanePoint(GAPoint point, GAPlane plane);
GALine reflectPlaneLine(GALine line, GAPlane plane);
GAPlane reflectPlanePlane(GAPlane p, GAPlane plane);

//Reflect things across line
GAPoint reflectLinePoint(GAPoint point, GALine line);
GALine reflectLineLine(GALine l, GALine line);
GAPlane reflectLinePlane(GAPlane plane, GALine line);

//Reflect things through point
GAPoint reflectPointPoint(GAPoint p, GAPoint point);
GALine reflectPointLine(GALine l, GAPoint point);
GAPlane reflectPointPlane(GAPlane plane, GAPoint point);

//Reverse
GAPoint reversePoint(GAPoint p);
GALine reverseLine(GALine l);
GAPlane reversePlane(GAPlane p); //NOTE(ELI): Apparently This does nothing lmao

//Inner Products
f32 innerPointPoint(GAPoint a, GAPoint b);
f32 innerPointLine(GAPoint a, GALine b);
f32 innerPointPlane(GAPoint a, GAPlane b);

f32 innerLineLine(GALine a, GALine b);
f32 innerLinePlane(GALine a, GAPlane p);

f32 innerPlanePlane(GAPlane a, GAPlane b);

//SandwichProduct





#ifdef GA_IMPL

GALine meetPlanes3(GAPlane a, GAPlane b) {
    return (GALine){
        .a = a.b * b.a - a.a * b.b,
        .b = a.c * b.a - a.a * b.c,
        .c = a.d * b.a - a.a * b.d,
        .d = a.c * b.b - a.b * b.c,
        .e = a.b * b.d - a.d * b.b,
        .f = a.d * b.c - a.c * b.d,
    };
}


GAPoint meetLinesPlane(GAPlane a, GALine b) {
    return (GAPoint) {
        .a = a.c * b.e + a.b * b.f + a.d * b.d,
        .b = a.c * b.c - a.a * b.f - a.d * b.b,
        .c = a.d * b.a - a.b * b.c - a.a * b.e,
        .d = a.b * b.b - a.c * b.a - a.a * b.d,
    };
}

GAPoint normPoint3(GAPoint p) {
    return (GAPoint) {
        .a = 1.0f, 
        .b = p.b/p.a, 
        .c = p.c/p.a, 
        .d = p.d/p.a, 
    };
}

union Unify {
    GAPoint point;
    GAPlane plane;
};

GAPoint dualPlane3(GAPlane p) {
    union Unify a = {0};
    a.plane = p;
    return a.point;
}

GAPlane dualPoint3(GAPoint p) {
    union Unify a = {0};
    a.point = p;
    return a.plane;
}

GALine joinPoints3(GAPoint a, GAPoint b) {
    return meetPlanes3(dualPoint3(a), dualPoint3(b));
}

GAPlane joinPointLine(GAPoint a, GALine b) {
    return dualPoint3(meetLinesPlane(dualPoint3(a), b));
}

GALine idealLineToLine(GAIdealLine l) {
    return (GALine) {
        .a = l.a,
        .b = l.b,
        .c = l.c,
    };
}

#endif
#endif
