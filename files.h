#ifndef FILES_H_
#define FILES_H_

#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

int divisions = 32;                             // resolution of objects (polygons)

//
surface* parse_line(string line) {
    vector<Point> vp;
    string s;
    string type;                                // object type (curve or surface)
    int n;                                      // points in x
    int    m = 1;                               // points in y

    istringstream lin(line);

    lin >> type;

    lin >> n;

    if(type.compare("surface") == 0) {
        lin >> m;
    }

    float x, y, z;

    for(int i = 0; i < n*m; i++) {
        lin >> x >> y >> z;                     // read coordinates
        vp.push_back(Point(x, y, z));           // put anchor coordinates in a vector
    }

    if (type.compare("surface") == 0) {         // SURFACE2D
        surface2d *sr = new surface2d(n-1,m-1);
        for(unsigned int i = 0; i < vp.size(); i++) {
            sr->anchors[i % n][i / n] = new Point(vp[i]); // copy points
        }
        *sr = getBezier(*sr,divisions);         // create surface
        return sr;
    } else if (type.compare("curve") == 0) {    // SURFACE1D
        surface1d *sr = new surface1d(vp.size() - 1);
        for(unsigned int i = 0; i < vp.size(); i++) {
            sr->anchors[i] = new Point(vp[i]);  // copy points
        }
        *sr = getBezier(*sr,divisions);         // create curve
        return sr;
    }
    
    return new surface1d(0);

}

//
vector<surface*> read_file(const char *filename) {
    vector<surface*> v;

    string line;

    ifstream f(filename,std::ifstream::in);

    if(f.is_open()) {
        while (!f.eof()) {
            getline(f, line);                   // read line
            if(line.length()>0) {
                v.push_back(parse_line(line));  // parse text into object, put object in array 
            }
        }
    }

    return v;
}

#endif /* FILES_H_ */
