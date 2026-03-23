#define SB_IMPL
#include <core/cutils.h>

#define GA_IMPL
#include <ga/ga.h>


int main() {

    {
        printlog("\n--------------- Test Meet of Planes -----------\n");
        GAPlane p1 = {-2, 1, 0, 0};
        GAPlane p2 = {-1, 0, 1, 0};
        GAPlane p3 = {-1, 0, 0, 1};

        GALine l = meetPlanes3(p1, p2);

        debuglog("Line: (%f %f %f) (%f %f %f)", l.a, l.b, l.c, l.d, l.e, l.f);
        //debuglog("Line: %d", l.f);

        GAPoint point = meetLinesPlane(p3, l);

        point = normPoint3(point);
        debuglog("Point: (%f %f %f) %f", point.b, point.c, point.d, point.a);
    }
    {
        printlog("\n--------------- Test Join of Points -----------\n");
        GAPoint p1 = {1, 1, 1, 1};
        GAPoint p2 = {1, 0, 0, 0};
        GAPoint p3 = {1, 0, 1, 0};

        GALine l = joinPoints3(p1, p2);
        GAPlane plane = joinPointLine(p3, l);

        debuglog("%fx + %fy + %fz + %f = 0", plane.b, plane.c, plane.d, plane.a);
    }

    {
        printlog("\n-------------- Test Join of Point and Line -----------\n");
        GAPlane p = {-1, 0, 0, 1};
        GALine l = makeLine(1, 1, 0, 0, 0, 1);
        debuglog("%fx + %fy + %fz + %f = 0", p.b, p.c, p.d, p.a);
        debuglog("Line: (%f %f %f) (%f %f %f)", l.a, l.b, l.c, l.d, l.e, l.f);

        GAPoint point = meetLinesPlane(p, l);
        debuglog("Point: (%f %f %f) %f", point.b, point.c, point.d, point.a);
    }

    {
        printlog("\n------------ Test Make Funcs -----------------\n");
        GALine l = makeLine(1, 1, 0, 0, 0, 1);
        debuglog("Line: (%f %f %f) (%f %f %f)", l.a, l.b, l.c, l.d, l.e, l.f);

        GAPoint p = makePoint(1, 2, 3);
        debuglog("Point: (%f %f %f) %f", p.b, p.c, p.d, p.a);

        GAPlane plane = makePlane(1, 1, 1, 0);
        debuglog("Plane: %fx + %fy + %fz + %f = 0", plane.b, plane.c, plane.d, plane.a);

    }

    printlog("\n--------------- Test Rejection ---------------\n");
    {
        GALine l = makeLine(0, 0, 0, 1, 1, 1);
        GAPlane p = makePlane(0, 0, 1, -1);

        GAPlane plane = perpPlaneAlongLine(p, l);
        debuglog("Plane: %fx + %fy + %fz + %f = 0", plane.b, plane.c, plane.d, plane.a);
    }
    {
        GAPoint pl = makePoint(1,1,1);
        GAPlane p = makePlane(1, 1, 1, 0);

        GALine line = perpPlaneAlongPoint(p, pl);
        debuglog("Line: (%f %f %f) (%f %f %f)", line.a, line.b, line.c, line.d, line.e, line.f);
    }
    {
        GALine l = makeLine(0, 0, 0, 0, 0, 1);
        GAPoint p = makePoint(1, 2, 3);

        GAPlane plane = perpLineAlongPoint(l, p);
        debuglog("Plane: %fx + %fy + %fz + %f = 0", plane.b, plane.c, plane.d, plane.a);
    }

    printlog("\n--------------- Test Projection ----------------\n");
    {
        GAPlane p = makePlane(1, 1, 1, 0);
        GAPoint t = makePoint(-1, -2, 1);

        GAPoint point = projectPointOntoPlane(t, p);
        GALine line = perpPlaneAlongPoint(p, t);
        debuglog("Line: (%f %f %f) (%f %f %f)", line.a, line.b, line.c, line.d, line.e, line.f);
        debuglog("Point: (%f %f %f) %f", point.b, point.c, point.d, point.a);
    }
    {
        GAPlane p = makePlane(1, 1, 1, 0);
        GAPoint t = makePoint(-1, -2, 1);

        GAPlane plane = projectPlaneOntoPoint(p, t);
        debuglog("Plane: %fx + %fy + %fz + %f = 0", plane.b, plane.c, plane.d, plane.a);
    }



    return 0;
}
