#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>
#include <cstring>
#include <cassert>
#include <algorithm>
#include <unordered_set>

#include "Point.h"
#include "Rows.h"
#include "VoxelGrid.h"

void find_air(const std::vector<unsigned int> &vox_idx, VoxelGrid& voxels);
void find_empty_neighbors(const std::string &vox_idx, const std::unordered_set<std::string>& visited,
                          std::unordered_set<std::string>& unvisited, VoxelGrid& voxels);
std::string my_to_string(const std::vector<unsigned int>& vec);

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

std::string my_to_string(const std::vector<unsigned int>& vec) {
    std::string out;
    for (auto s : vec) {
        out += static_cast<char>(s);
    }
    return out;
}

void find_air(const std::vector<unsigned int>& vox_idx, VoxelGrid& voxels) {
    std::unordered_set<std::string> visited, unvisited;
    if (voxels(vox_idx[0], vox_idx[1], vox_idx[2]) == 1) {
        std::cerr << "Starting voxel is not empty" << std::endl;
        exit(1);
    }
    voxels(vox_idx[0], vox_idx[1], vox_idx[2]) = 2;
    std::string idx_str = my_to_string(vox_idx);
    visited.insert(idx_str);
    find_empty_neighbors(idx_str, visited, unvisited, voxels);
    while (!unvisited.empty()) {
        std::string current_vox = *(unvisited.begin());
        visited.insert(current_vox);
        find_empty_neighbors(current_vox, visited, unvisited, voxels);
        unvisited.erase(current_vox);
    }
}

void find_empty_neighbors(const std::string &vox_idx, const std::unordered_set<std::string>& visited,
                          std::unordered_set<std::string>& unvisited, VoxelGrid& voxels) {
    unsigned int i = static_cast<unsigned char>(vox_idx[0]);
    unsigned int j = static_cast<unsigned char>(vox_idx[1]);
    unsigned int k = static_cast<unsigned char>(vox_idx[2]);
    std::vector<std::vector<unsigned int>> neighbors = {{i - 1, j, k}, {i + 1, j, k}, {i, j - 1, k}, {i, j + 1, k},
                                                        {i, j, k - 1}, {i, j, k + 1}};
    for (const auto& n : neighbors) {
        if ((0 <= n[0]) && (n[0] < voxels.max_x) && (0 <= n[1]) && (n[1] < voxels.max_y) && (0 <= n[2]) && (n[2] < voxels.max_z)) {
            if (voxels(n[0], n[1], n[2]) != 1) {
                std::string curr_vox = my_to_string(n);
                if (unvisited.find(curr_vox) == unvisited.end() && (visited.find(curr_vox) == visited.end())){
                    voxels(n[0], n[1], n[2]) = 2;
                    unvisited.insert(curr_vox);
                }
            }
        }
    }
}

int main(int argc, const char *argv[]) {
    const char *file_in = "../bag_bk.obj";
    const char *file_out = "vox.obj";
    float voxel_size = 1.0;

    // Read file
    std::vector<Point> vertices;
    std::vector<std::vector<unsigned int>> faces;
    std::ifstream file(file_in);
    std::string str;
    std::string letter;
    float x, y, z;
    float min_x = 0, min_y = 0, min_z = 0;
    float max_x = 0, max_y = 0, max_z = 0;
    std::vector<unsigned int> f1(3);
    unsigned int v1, v2, v3;
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
            ss >> v1 >> v2 >> v3;
            f1 = {v1 - 1, v2 - 1, v3 - 1};
            faces.emplace_back(f1);
        }
        ss.clear();
        ss.str(std::string());
    }
//    std::cout << vertices.size() << std::endl;
//    std::cout << vertices[2417] << std::endl;
//    std::cout << vertices[2418] << std::endl;
//    return 0;
    assert(faces[100][0] == 83);  // a test to check if the values are added correctly

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
    int n = 0;
    for (auto const &triangle: faces) {
//        std::cout << " " << std::endl;
//        std::cout << "Triangle number " << n << std::endl;
        // triangle's vertices
        Point t0 = vertices[triangle[0]], t1 = vertices[triangle[1]], t2 = vertices[triangle[2]];
        // Check bbox validity
        //std::cout << "3D Bbox: " << bbox(t0, t1, t2)[0] << bbox(t0, t1, t2)[1] << std::endl;
        assert(bbox(t0, t1, t2)[0][0] <= bbox(t0, t1, t2)[1][0]
        && bbox(t0, t1, t2)[0][1] <= bbox(t0, t1, t2)[1][1]
        && bbox(t0, t1, t2)[0][2] <= bbox(t0, t1, t2)[1][2]);
        // We get the voxel subset corresponding to the triangle
        VoxelGrid subset = miniGrid(bbox(t0, t1, t2), min_x, min_y, min_z, voxel_size);
//        std::cout << "Voxel subset has max X = " << subset.max_x << " max Y = " << subset.max_y << " max Z = " << subset.max_z << std::endl;
//        std::cout << "Number of voxels to test: " << subset.max_z * subset.max_y * subset.max_x << std::endl;
        assert(subset.max_z * subset.max_y * subset.max_x < voxels.max_x * voxels.max_y * voxels.max_z);
        // loop through the subset
        int voxelCount = 0;
        for (int x = 0; x < subset.max_x; x++) {
            for (int y = 0; y < subset.max_y; y++) {
                for (int z = 0; z < subset.max_z; z++) {
                    voxelCount++;
                    //std::cout << "Pixel coordinates: X = " << x << " Y = " << y+1 << " Z = " << z+1 << " --------- VOXEL NUMBER " << voxelCount << std::endl;
                    // Voxel's target
                    Point targetA1((x + 0.5) * voxel_size, y * voxel_size, (z + 0.5) * voxel_size);
                    Point targetA2((x + 0.5) * voxel_size, (y + 1) * voxel_size, (z + 0.5) * voxel_size);
                    Point targetB1(x * voxel_size, (y + 0.5) * voxel_size, (z + 0.5) * voxel_size);
                    Point targetB2((x + 1) * voxel_size, (y + 0.5) * voxel_size, (z + 0.5) * voxel_size);
                    Point targetC1((x + 0.5) * voxel_size, (y + 0.5) * voxel_size, z * voxel_size);
                    Point targetC2((x + 0.5) * voxel_size, (y + 0.5) * voxel_size, (z + 1) * voxel_size);
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
//                    std::cout << voxels(x, y, z);
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
        assert(voxelCount == subset.total_voxels);


        // to work on small amount of triangles and make it work first => to be deleted later
//        if (n > 1000) {
//            return 0;
//        }

        n++;
    }

//    std::cout << "Hello" <<std::endl;
//    for (int i = 0; i < max_x; i++) {
//        for (int j = 0; j < max_y; j++) {
//            for (int k = 0; k < max_z; k++) {
//                std::cout << voxels(x, y, z);
//            }
//        }
//    }

    // Fill model
    // TODO
    std::vector<unsigned int> starting_voxel_idx = {voxels.max_x - 1, 0, 0};
    find_air(starting_voxel_idx, voxels);
    for (int i = 0; i < voxels.total_voxels; i++ ) {
        if (voxels.voxels[i] == 0) {
            voxels.voxels[i] = 1;
        }
    }

    // Write voxels
    // TODO
    std::vector<Point> new_vertices;
    std::vector<std::vector<unsigned int>> new_faces;
//    unsigned int total_voxels = (voxels.max_x * voxels.max_y * voxels.max_z);
    for (int x1 = 0; x1 < max_x; x1++) {
        for (int y1 = 0; y1 < max_y; y1++) {
            for (int z1 = 0; z1 < max_z; z1++) {
                new_vertices.emplace_back(Point(x1, y1, z1));
            }
        }
    }
//    std::cout << new_vertices[0] << std::endl;
    std::ofstream output_file(file_out);
    if (output_file.is_open())
    {
        for(int i = 0; i < new_vertices.size(); i++)
        {
            output_file << "v " << new_vertices[i][0] << " " << new_vertices[i][1] << " " << new_vertices[i][2] << "\n";
        }
        std::cout << "file is written" << std::endl;
        output_file.close();
    }
    else std::cout << "Unable to open file";


    return 0;
}
