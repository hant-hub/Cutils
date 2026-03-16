#define SB_IMPL
#include <core/cutils.h>

#define GA_IMPL
#include <ga/ga.h>


int main() {

    {
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
        GAPoint p1 = {1, 1, 1, 1};
        GAPoint p2 = {1, 0, 0, 0};
        GAPoint p3 = {1, 0, 1, 0};

        GALine l = joinPoints3(p1, p2);
        GAPlane plane = joinPointLine(p3, l);

        debuglog("%fx + %fy + %fz + %f = 0", plane.b, plane.c, plane.d, plane.a);
    }



    return 0;
}
