#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>
#include <cstring>
#include <cassert>

#include "Point.h"
#include "Rows.h"
#include "VoxelGrid.h"

float signed_volume(const Point &a, const Point &b, const Point &c, const Point &d) {
    // TODO
    return 0;
}

bool intersects(const Point &orig, const Point &dest, const Point &v0, const Point &v1, const Point &v2) {
    // TODO
    return false;
}

int main(int argc, const char *argv[]) {
    const char *file_in = "bag_bk.obj";
    const char *file_out = "vox.obj";
    float voxel_size = 1.0;

    // Read file
    std::vector<Point> vertices;
    std::vector<std::vector<unsigned int>> faces;
    std::ifstream file("\\Users\\Asus\\Desktop\\OKUL\\DELFT\\Q3\\GEO1004\\Assignment1\\hw1\\bag_bk.obj");
    std::string str;
    std::string letter;
    float x, y, z;
    std::vector<unsigned int> f1(3);
    std::stringstream ss;
    while (std::getline(file, str)) {
        ss << str;
        ss >> letter;
        if (letter == "v") {
            ss >> x >> y >> z;
            vertices.emplace_back(Point(x, y, z));
        } else if (letter == "f") {
            ss >> f1[0] >> f1[1] >> f1[2];
            faces.emplace_back(f1);
        }
        ss.clear();
        ss.str(std::string());
    }
    assert(faces[100][0] == 84);  // a test to check if they are added correctly

    // Create grid
    Rows rows;
    // TODO
    VoxelGrid voxels(rows.x, rows.y, rows.z);

    // Voxelise
    for (auto const &triangle: faces) {
        // TODO
    }

    // Fill model
    // TODO

    // Write voxels
    // TODO

    return 0;
}
