#ifndef BEZIER_H_
#define BEZIER_H_

#include <math.h>
#include <iostream>

using namespace std;

const int DIMEN_BUFFER = 256 + 2;               // max divisions

enum OBJECT_TYPE {                              // bezier object types
    SURFACE1D = 1,
    SURFACE2D
};

// basic class Point
class Point {

public:

    float x, y, z;

    Point(float* d) {
        x = d[0];
        y = d[1];
        z = d[2];
    }

    Point(float x0 = 0.0, float y0 = 0.0, float z0 = 0.0) {
        x = x0;
        y = y0;
        z = z0;
    }

    Point operator*(float s) {
        return Point(x * s, y * s, z * s);
    }

    Point operator*(float s) const {
        return Point(x * s, y * s, z * s);
    }

    Point operator+(Point p2) {
        return Point(x + p2.x, y + p2.y, z + p2.z);
    }

};

// basic class surface with properties
class surface {

public:
    int dimen_n;
    int type;

};

// one-dimensional array for bezier curve
class surface1d: public surface {

public:

    Point *anchors[DIMEN_BUFFER];

    surface1d() {
        dimen_n = 0;
        type = SURFACE1D;
    }

    surface1d(int n) {
        dimen_n = (n > DIMEN_BUFFER) ? DIMEN_BUFFER : n;
        type = SURFACE1D;
    }

};

// two-dimensional array for bezier surface
class surface2d: public surface {

public:

    int dimen_m;

    Point *anchors[DIMEN_BUFFER][DIMEN_BUFFER];

    surface2d() {
        dimen_n = dimen_m = 0;
        type = SURFACE2D;
    }

    // by square array
    surface2d(int s) {
        dimen_m = dimen_n = (s > DIMEN_BUFFER) ? DIMEN_BUFFER : s;
        type = SURFACE2D;
    }

    // by non-square array
    surface2d(int n, int m) {
        dimen_n = (n > DIMEN_BUFFER) ? DIMEN_BUFFER : n;
        dimen_m = (m > DIMEN_BUFFER) ? DIMEN_BUFFER : m;
        type = SURFACE2D;
    }

};

// factorial
int fact(int n) {
    return (n <= 1) ? 1 : n * fact(n - 1);
}

// bernstein polynomial function
float bernstein(int n, int i, float u) {
    return fact(n) / (fact(i) * fact(n - i)) * pow(u, i) * pow(1 - u, n - i);
}

//
surface1d getBezier(surface1d a, int divs) {
    surface1d s(divs + 1);

    int t, i;
    float tv, b;

    Point *p;

    for (t = 0; t <= divs; t++) {
        tv = ((float) t) / ((float) divs);
        p = new Point();
        for (i = 0; i <= a.dimen_n; i++) {
            b = bernstein(a.dimen_n, i, tv);
            *p = *p + *a.anchors[i] * b;
        } // END i
        s.anchors[t] = p;
    } // END t

    return s;
}

//
surface2d getBezier(surface2d a, int divs) {
    surface2d s(divs + 1);

    int i, j, u, v;
    float ur, vr, bi, bj;

    Point *p;

    glColor3f(1.0, 0.0, 1.0);

    for (u = 0; u <= divs; u++) {
        ur = ((float) u) / ((float) divs);
        for (v = 0; v <= divs; v++) {
            vr = ((float) v) / ((float) divs);
            p = new Point();
            for (i = 0; i <= a.dimen_n; i++) {
                bi = bernstein(a.dimen_n, i, ur);
                for (j = 0; j <= a.dimen_m; j++) {
                    bj = bernstein(a.dimen_m, j, vr);
//                    cout << i << " " << j << endl;
//                    cout << a.anchors[i][j]->x << "," << a.anchors[i][j]->y << "," << a.anchors[i][j]->z << " \n";
                    *p = *p + *a.anchors[i][j] * bi * bj;
                } // END j
            } // END i
            s.anchors[u][v] = p;
        } // END v
    } // END u

    return s;

}

#endif /* BEZIER_H_ */
