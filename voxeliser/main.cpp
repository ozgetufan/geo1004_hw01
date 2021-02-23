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
    const Point cross = (b-d).cross(c-d);
    return ((a-d).dot(cross)) / 6;
}

bool intersects(const Point &orig, const Point &dest, const Point &v0, const Point &v1, const Point &v2) {
    bool test1 = (bool (signed_volume(orig, dest, v0, v1) > 0 && signed_volume(orig, dest, v0, v2) < 0) || bool (signed_volume(orig, dest, v0, v1) < 0 && signed_volume(orig, dest, v0, v2) > 0));
    bool test2 = (bool (signed_volume(orig, dest, v1, v0) > 0 && signed_volume(orig, dest, v1, v2) < 0) || bool (signed_volume(orig, dest, v1, v0) < 0 && signed_volume(orig, dest, v1, v2) > 0));
    bool test3 = (bool (signed_volume(orig, dest, v2, v0) > 0 && signed_volume(orig, dest, v2, v1) < 0) || bool (signed_volume(orig, dest, v2, v0) < 0 && signed_volume(orig, dest, v2, v1) > 0));
    if (test1 && test2 && test3) {
        std::cout << "TRUE ------------------------ TRUE !!" << std::endl;
        return true;
    }
    std::cout << "False" << std::endl;
    return false;
}

std::vector<Point> bbox(const Point &v0, const Point &v1, const Point &v2) {
    std::vector<float> x = {v0[0], v1[0], v2[0]}, y = {v0[1], v1[1], v2[1]}, z = {v0[2], v1[2], v2[2]};
    auto minmaxX = std::minmax_element(x.begin(), x.end()), minmaxY = std::minmax_element(y.begin(), y.end()), minmaxZ = std::minmax_element(z.begin(), z.end());
    std::vector<Point> bbox3D;
    bbox3D.push_back(Point(*minmaxX.first, *minmaxY.first, *minmaxZ.first));
    bbox3D.push_back(Point(*minmaxX.second, *minmaxY.second, *minmaxZ.second));
    std::cout << "3D Bbox: " << bbox3D[0] << bbox3D[1] << std::endl;
    return bbox3D;
}

int main(int argc, const char *argv[]) {
    const char *file_in = "bag_bk.obj";
    const char *file_out = "vox.obj";
    float voxel_size = 1.0;

    // Read file
    std::vector<Point> vertices;
    std::vector<std::vector<unsigned int>> faces;
    std::ifstream file("../bag_bk.obj");
    std::string str;
    std::string letter;
    float x, y, z;
    float min_x = 0, min_y = 0, min_z = 0;
    float max_x = 0, max_y = 0, max_z = 0;
    std::vector<unsigned int> f1(3);
    std::stringstream ss;
    while (std::getline(file, str)) {
        ss << str;
        ss >> letter;
        if (letter == "v") {
            ss >> x >> y >> z;
            if (x < min_x) {
                min_x = x;
            }
            if (y < min_y) {
                min_y = y;
            }
            if (z < min_z) {
                min_z = z;
            }
            if (x > max_x) {
                max_x = x;
            }
            if (y > max_y) {
                max_y = y;
            }
            if (z > max_z) {
                max_z = x;
            }
            vertices.emplace_back(Point(x, y, z));
        } else if (letter == "f") {
            ss >> f1[0] >> f1[1] >> f1[2];
            faces.emplace_back(f1);
        }
        ss.clear();
        ss.str(std::string());
    }
    assert(faces[100][0] == 84);  // a test to check if the values are added correctly

    // Create grid
    int row_x = int((max_x - min_x) / voxel_size) + 1; // for each row, we added 1 to make sure that we cover all the area
    int row_y = int((max_y - min_y) / voxel_size) + 1;
    int row_z = int((max_z - min_z) / voxel_size) + 1;
    Rows rows(row_x, row_y, row_z);
    VoxelGrid voxels(rows.x, rows.y, rows.z);

//    voxels(5, 7, 9) = 5;     // A test to assign value to a voxel
//    std::cout << "voxel test: " << voxels(5, 7, 9) << std::endl;

/*
    //tests
    std::cout << "Vertices: " << vertices.size() << std::endl;
    std::cout << "Faces: " << faces.size() << std::endl;
    std::cout << "signed volume: " << signed_volume(vertices[0], vertices[1], vertices[2], vertices[1000]) << std::endl;
    std::cout << "signed volume: " << signed_volume(vertices[100], vertices[101], vertices[102], vertices[1003]) << std::endl;
    std::cout << "signed volume: " << signed_volume(vertices[0], vertices[1], vertices[2], vertices[1999]) << std::endl;

    for (int i=0; i < 300; i++) {
        intersects(vertices[i], vertices[i+1], vertices[faces[i][0]], vertices[faces[i][1]], vertices[faces[i][2]]);
    }
*/
    std::cout << "Voxel value: " << voxels(0,0,0) << std::endl;
    std::cout << "Voxel grid X: " << row_x << std::endl;
    std::cout << "Voxel grid Y: " << row_y << std::endl;
    std::cout << "Voxel grid Z: " << row_z << std::endl;
    std::cout << "Voxels total: " << voxels.max_x * voxels.max_y * voxels.max_z << std::endl;

    // Voxelise
    int n = 1;
    for (auto const &triangle: faces) {
        std::cout << "Triangle number " << n << std::endl;
        // triangle's vertex
        Point t0 = vertices[triangle[0]], t1 = vertices[triangle[1]], t2 = vertices[triangle[2]];
        bbox(t0, t1, t2);

/*        // Voxels in z dimension loop:
        for (int z = 0; z < row_z; z++) {
            // Voxels coordinates of lower x rows
            float voxZ = min_z + z * voxel_size;
            float voxX = min_x, voxY = min_y, voxNextZ = min_z + (z+1) * voxel_size;
            if (v1[2] <= voxNextZ && v1[2] >= voxZ) {
                std::cout << "We found the voxel zRow, it's coordinates are, x: " << voxX << " and y " << voxY << " and z " << voxZ << std::endl;
            }
        }*/

        n++;
    }

    // Fill model
    // TODO

    // Write voxels
    // TODO

    return 0;
}
