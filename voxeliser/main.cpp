#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>
#include <cstring>
#include <cassert>
#include <algorithm>


#include "Point.h"
#include "Rows.h"
#include "VoxelGrid.h"

float signed_volume(const Point &a, const Point &b, const Point &c, const Point &d) {
    const Point cross = (b-d).cross(c-d);
    return ((a-d).dot(cross)) / 6;
}

//bool intersects(const Point &orig, const Point &dest, const Point &v0, const Point &v1, const Point &v2) {
//    bool test1 = (bool (signed_volume(orig, dest, v0, v1) > 0 && signed_volume(orig, dest, v0, v2) < 0)
//            || bool (signed_volume(orig, dest, v0, v1) < 0 && signed_volume(orig, dest, v0, v2) > 0));
//    bool test2 = (bool (signed_volume(orig, dest, v1, v0) > 0 && signed_volume(orig, dest, v1, v2) < 0)
//            || bool (signed_volume(orig, dest, v1, v0) < 0 && signed_volume(orig, dest, v1, v2) > 0));
//    bool test3 = (bool (signed_volume(orig, dest, v2, v0) > 0 && signed_volume(orig, dest, v2, v1) < 0)
//            || bool (signed_volume(orig, dest, v2, v0) < 0 && signed_volume(orig, dest, v2, v1) > 0));
//    if (test1 && test2 && test3) {
//        //std::cout << "TRUE ------------------------ TRUE !!" << std::endl;
//        return true;
//    }
//    //std::cout << "False" << std::endl;
//    return false;
//}

bool intersects(const Point &orig, const Point &dest, const Point &v0, const Point &v1, const Point &v2) {
    if(signed_volume(v0, v1, v2, orig) * signed_volume(v0, v1, v2, dest) <= 0){
        float test1 = signed_volume(orig, dest, v0, v1) * signed_volume(orig, dest, v0, v2);
        float test2 = signed_volume(orig, dest, v1, v0) * signed_volume(orig, dest, v1, v2);
        float test3 = signed_volume(orig, dest, v2, v0) * signed_volume(orig, dest, v2, v1);
        if (test1 < 0 && test2 < 0 && test3 < 0){
            return true;
        }
        return false;
    }
    return false;
}

std::vector<Point> bbox(const Point &v0, const Point &v1, const Point &v2) {
    std::vector<float> x = {v0[0], v1[0], v2[0]}, y = {v0[1], v1[1], v2[1]}, z = {v0[2], v1[2], v2[2]};
    auto minmaxX = std::minmax_element(x.begin(), x.end()), minmaxY = std::minmax_element(y.begin(), y.end()), minmaxZ = std::minmax_element(z.begin(), z.end());
    std::vector<Point> bbox3D;
    bbox3D.push_back(Point(*minmaxX.first, *minmaxY.first, *minmaxZ.first));
    bbox3D.push_back(Point(*minmaxX.second, *minmaxY.second, *minmaxZ.second));
    return bbox3D;
}

VoxelGrid miniGrid(std::vector<Point> bbox, float min_x, float min_y, float min_z, float voxelSize) {
    int minX = floor((bbox[0][0] - min_x) / voxelSize); // "ceil", used to round up and "floor" to round down
    int minY = floor((bbox[0][1] - min_y)/ voxelSize);
    int minZ = floor((bbox[0][2] - min_z)/ voxelSize);
    int maxX = floor((bbox[1][0] - min_x)/ voxelSize);
    int maxY = floor((bbox[1][1] - min_y)/ voxelSize);
    int maxZ = floor((bbox[1][2] - min_z)/ voxelSize);
    int rowX = maxX - minX + 1;
    int rowY = maxY - minY + 1;
    int rowZ = maxZ - minZ + 1;
    Rows miniRows(rowX, rowY, rowZ);
    VoxelGrid miniGrid(miniRows.x, miniRows.y, miniRows.z);
    return miniGrid;
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

    // Voxelise
    std::cout << "Voxels total: " << voxels.max_x * voxels.max_y * voxels.max_z << std::endl;
    std::cout << "Original voxel grid has: = " << voxels.max_x << " max Y = " << voxels.max_y << " max Z = " << voxels.max_z << std::endl;
    int n = 1;
    for (auto const &triangle: faces) {
        std::cout << " " << std::endl;
        std::cout << "Triangle number " << n << std::endl;
        // triangle's vertices
        Point t0 = vertices[triangle[0]], t1 = vertices[triangle[1]], t2 = vertices[triangle[2]];
        // Check bbox validity
        //std::cout << "3D Bbox: " << bbox(t0, t1, t2)[0] << bbox(t0, t1, t2)[1] << std::endl;
        assert(bbox(t0, t1, t2)[0][0] <= bbox(t0, t1, t2)[1][0]
        && bbox(t0, t1, t2)[0][1] <= bbox(t0, t1, t2)[1][1]
        && bbox(t0, t1, t2)[0][2] <= bbox(t0, t1, t2)[1][2]);
        // We get the voxel subset corresponding to the triangle
        VoxelGrid subset = miniGrid(bbox(t0, t1, t2), min_x, min_y, min_z, voxel_size);
        std::cout << "Voxel subset has max X = " << subset.max_x << " max Y = " << subset.max_y << " max Z = " << subset.max_z << std::endl;
        std::cout << "Number of voxels to test: " << subset.max_z * subset.max_y * subset.max_x << std::endl;
        assert(subset.max_z * subset.max_y * subset.max_x < voxels.max_x * voxels.max_y * voxels.max_z);

        // loop through the subset
        int voxelCount = 0;
        for (int x = 0; x < subset.max_x; x++) {
            for (int y = 0; y < subset.max_y; y++) {
                for (int z = 0; z < subset.max_z; z++) {
                    voxelCount++;
                    //std::cout << "Pixel coordinates: X = " << x << " Y = " << y+1 << " Z = " << z+1 << " --------- VOXEL NUMBER " << voxelCount << std::endl;
                    // Voxel's target
                    Point targetA1((x + 1/2) * voxel_size, y, (z + 1/2) * voxel_size);
                    Point targetA2((x + 1/2) * voxel_size, (y + 1) * voxel_size, (z + 1/2) * voxel_size);
                    Point targetB1(x, (y + 1/2) * voxel_size, (z + 1/2) * voxel_size);
                    Point targetB2((x + 1) * voxel_size, (y + 1/2) * voxel_size, (z + 1/2) * voxel_size);
                    Point targetC1((x + 1/2) * voxel_size, (y + 1/2) * voxel_size, z);
                    Point targetC2((x + 1/2) * voxel_size, (y + 1/2) * voxel_size, (z + 1) * voxel_size);
                    // test number 1: 2 corresponding targets have to be on different side of the triangle
//                    bool testA = bool (signed_volume(t0, t1, t2, targetA1) >= 0 && signed_volume(t0, t1, t2, targetA2) <= 0)
//                                 || bool (signed_volume(t0, t1, t2, targetA1) <= 0 && signed_volume(t0, t1, t2, targetA2) >= 0);
//                    bool testB = bool (signed_volume(t0, t1, t2, targetB1) >= 0 && signed_volume(t0, t1, t2, targetB2) <= 0)
//                                 || bool (signed_volume(t0, t1, t2, targetB1) <= 0 && signed_volume(t0, t1, t2, targetB2) >= 0);
//                    bool testC = bool (signed_volume(t0, t1, t2, targetC1) >= 0 && signed_volume(t0, t1, t2, targetC2) <= 0)
//                                 || bool (signed_volume(t0, t1, t2, targetC1) <= 0 && signed_volume(t0, t1, t2, targetC2) >= 0);
                    if (intersects(targetA1, targetA2, t0, t1, t2) || intersects(targetB1, targetB2, t0, t1, t2) || intersects(targetC1, targetC2, t0, t1, t2)) {
//                        std::cout << "We have an intersection with the triangle !!!!!!!!!!!!!!!!!!!!! :) " << std::endl;
                        voxels(x, y, z) = 1;

                    }
                    std::cout << voxels(x, y, z);
//                    else {
//                        std::cout << "We don't have an intersection" << std::endl;
//                    }
//                    if (testA && testB && testC) {
//                        std::cout << "TEST 1 VALIDATED !" << std::endl;
//                        // test number 2 (intersects function): does the target really intersects the triangle ?
//                        if (intersects(targetA1, targetA2, t0, t1, t2) && intersects(targetB1, targetB2, t0, t1, t2) && intersects(targetC1, targetC2, t0, t1, t2)) {
//                            std::cout << "We have an intersection with the triangle !!!!!!!!!!!!!!!!!!!!! :) " << std::endl;
//                        }
//                        else {
//                            //std::cout << "TEST 2 failed... sorry !" << std::endl;
//                        }
//                    }
//                    else {
//                        std::cout << "TEST 1 already failed... sorry !" << std::endl;
//                    }
                }
            }
        }
        // Check that the number of voxels in the loop is equivalent to the number expected
        assert(voxelCount == subset.max_z * subset.max_y * subset.max_x);


        // to work on small amount of triangles and make it work first => to be deleted later
        if (n > 1000) {
            return 0;
        }
        n++;
    }


    // Fill model
    // TODO

    // Write voxels
    // TODO

    return 0;
}
