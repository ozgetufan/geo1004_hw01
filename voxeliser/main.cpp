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

VoxelGrid miniGrid(std::vector<Point> bbox, float voxelSize) {
/*    float min_x, float min_y, float min_z,
 * int minX = floor((bbox[0][0] - min_x) / voxelSize); // "ceil", used to round up and "floor" to round down
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
    return miniGrid;*/
    // Create grid
    int fix_x, fix_y, fix_z;
    float diffX = bbox[1][0] - bbox[0][0], diffY = bbox[1][1] - bbox[0][1], diffZ = bbox[1][2] - bbox[0][2];
    if (diffX == 0) {
        fix_x = 1;
    }
    else fix_x = ceil((bbox[1][0] - floor(bbox[1][0]) + bbox[0][0] - floor(bbox[0][0])) / voxelSize);
    if (diffY == 0) {
        fix_y = 1;
    }
    else fix_y = ceil((bbox[1][1] - floor(bbox[1][1]) + bbox[0][1] - floor(bbox[0][1])) / voxelSize);
    if (diffZ == 0) {
        fix_z = 1;
    }
    else fix_z = ceil((bbox[1][2] - floor(bbox[1][2]) + bbox[0][2] - floor(bbox[0][2])) / voxelSize);
    int row_x = ((floor(bbox[1][0]) - floor(bbox[0][0])) / voxelSize) + fix_x;
    int row_y = ((floor(bbox[1][1]) - floor(bbox[0][1])) / voxelSize) + fix_y;
    int row_z = ((floor(bbox[1][2]) - floor(bbox[0][2])) / voxelSize) + fix_z;
    Rows miniRows(row_x, row_y, row_z);
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
                max_z = z;
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
    assert(faces[100][0] == 83);  // a test to check if the values are added correctly

    // Create grid
    int fix_x = ceil((max_x - floor(max_x) + min_x - floor(min_x)) / voxel_size);
    int fix_y = ceil((max_y - floor(max_y) + min_y - floor(min_y)) / voxel_size);
    int fix_z = ceil((max_z - floor(max_z) + min_z - floor(min_z)) / voxel_size);
    int row_x = ((floor(max_x) - floor(min_x)) / voxel_size) + fix_x;
    int row_y = ((floor(max_y) - floor(min_y)) / voxel_size) + fix_y;
    int row_z = ((floor(max_z) - floor(min_z)) / voxel_size) + fix_z;
    Rows rows(row_x, row_y, row_z);
    VoxelGrid voxels(rows.x, rows.y, rows.z);

//    voxels(5, 7, 9) = 5;     // A test to assign value to a voxel
//    std::cout << "voxel test: " << voxels(5, 7, 9) << std::endl;

    // Voxelise
    std::cout << "Voxels total: " << voxels.max_x * voxels.max_y * voxels.max_z << std::endl;
    std::cout << "Original voxel grid has: X = " << voxels.max_x << " Y = " << voxels.max_y << " Z = " << voxels.max_z << std::endl;
    int n = 0;
    for (auto const &triangle: faces) {
        std::cout << "Triangle number " << n << std::endl;
        // triangle's vertices
        Point t0 = vertices[triangle[0]], t1 = vertices[triangle[1]], t2 = vertices[triangle[2]];
        // Check bbox validity
        std::vector<Point> box = bbox(t0, t1, t2);
        assert(box[0][0] <= box[1][0] && box[0][1] <= box[1][1] && box[0][2] <= box[1][2]);
        // We define the mini voxel grid
        int fix_x, fix_y, fix_z;
        float diffX = box[1][0] - box[0][0], diffY = box[1][1] - box[0][1], diffZ = box[1][2] - box[0][2];
        if (diffX == 0) {
            fix_x = 1;
        }
        else fix_x = ceil((box[1][0] - floor(box[1][0]) + box[0][0] - floor(box[0][0])) / voxel_size);
        if (diffY == 0) {
            fix_y = 1;
        }
        else fix_y = ceil((box[1][1] - floor(box[1][1]) + box[0][1] - floor(box[0][1])) / voxel_size);
        if (diffZ == 0) {
            fix_z = 1;
        }
        else fix_z = ceil((box[1][2] - floor(box[1][2]) + box[0][2] - floor(box[0][2])) / voxel_size);
        int offsetX = (floor(box[0][0]) - floor(min_x)) / voxel_size,
                offsetY = (floor(box[0][1]) - floor(min_y)) / voxel_size,
                offsetZ = (floor(box[0][2])  - floor(min_z)) / voxel_size;
        int row_x = ((floor(box[1][0]) - floor(box[0][0])) / voxel_size) + fix_x;
        int row_y = ((floor(box[1][1]) - floor(box[0][1])) / voxel_size) + fix_y;
        int row_z = ((floor(box[1][2]) - floor(box[0][2])) / voxel_size) + fix_z;
        Rows miniRows(row_x, row_y, row_z);
        VoxelGrid subset(miniRows.x, miniRows.y, miniRows.z);
        assert(subset.max_z * subset.max_y * subset.max_x < voxels.max_x * voxels.max_y * voxels.max_z);
        //std::cout << "Subgrid dimension: X = " << subset.max_x << " Y = " << subset.max_y << " Z = " << subset.max_z << std::endl;
        // loop through the subset
        int voxelCount = 0;
        for (int x = 0; x < subset.max_x -1; x += voxel_size) {
            for (int y = 0; y < subset.max_y -1; y += voxel_size) {
                for (int z = 0; z < subset.max_z -1; z += voxel_size) {
                    voxelCount++;
                    // equivalence with big grid
                    int bigX = offsetX + x, bigY = offsetY + y, bigZ = offsetZ + z;
                    std::cout << "Equivalent big grid: " << bigX << " Y " << bigY << " Z " << bigZ << std::endl;
                    // Voxel's target
                    Point targetA1((bigX + 0.5) * voxel_size, bigY * voxel_size, (bigZ + 0.5) * voxel_size);
                    Point targetA2((bigX + 0.5) * voxel_size, (bigY + 1) * voxel_size, (bigZ + 0.5) * voxel_size);
                    Point targetB1(bigX * voxel_size, (bigY + 0.5) * voxel_size, (bigZ + 0.5) * voxel_size);
                    Point targetB2((bigX + 1) * voxel_size, (bigY + 0.5) * voxel_size, (bigZ + 0.5) * voxel_size);
                    Point targetC1((bigX + 0.5) * voxel_size, (bigY + 0.5) * voxel_size, bigZ * voxel_size);
                    Point targetC2((bigX + 0.5) * voxel_size, (bigY + 0.5) * voxel_size, (bigZ + 1) * voxel_size);
                    // we assign 1 to the voxel intersecting the triangular mesh
                    if (intersects(targetA1, targetA2, t0, t1, t2) || intersects(targetB1, targetB2, t0, t1, t2) || intersects(targetC1, targetC2, t0, t1, t2)) {
                        // find equivalent voxel in big grid
                        voxels(bigX, bigY, bigZ) = 1;
/*                        Point p1(bigX, bigY, bigZ), p2(bigX+voxel_size, bigY, bigZ), p3(bigX+voxel_size, bigY+voxel_size, bigZ), p4(bigX, bigY+voxel_size, bigZ),
                        p5(bigX, bigY, bigZ+voxel_size), p6(bigX+voxel_size, y, bigZ+voxel_size), p7(bigX+voxel_size, bigY+voxel_size, bigZ+voxel_size), p8(x, bigY+voxel_size, bigZ+voxel_size);
                        //new_vertices.emplace_back(p1, p2, p3, p4, p5, p6, p7, p8);
                        new_vertices.push_back(p1);
                        new_vertices.push_back(p2);
                        new_vertices.push_back(p3);
                        new_vertices.push_back(p4);
                        new_vertices.push_back(p5);
                        new_vertices.push_back(p6);
                        new_vertices.push_back(p7);
                        new_vertices.push_back(p8);*/
                    }
                }
            }
        }
        // Check that the number of voxels in the loop is equivalent to the number expected
        //assert(voxelCount == subset.total_voxels);
        n++;
    }

    // Fill model
    std::vector<unsigned int> starting_voxel_idx = {voxels.max_x - 1, 0, 0};
    find_air(starting_voxel_idx, voxels);
    for (int i = 0; i < voxels.total_voxels; i++ ) {
        if (voxels.voxels[i] == 0) {
            voxels.voxels[i] = 1;
        }
    }

    // Write voxels
    std::vector<Point> new_vertices;
    std::vector<std::vector<unsigned int>> new_faces;
//    unsigned int total_voxels = (voxels.max_x * voxels.max_y * voxels.max_z);
    for (int x1 = floor(min_x); x1 < (voxels.max_x + floor(min_x)); x1 += voxel_size) {
        for (int y1 = floor(min_y); y1 < (voxels.max_y + floor(min_y)); y1 += voxel_size) {
            for (int z1 = floor(min_z); z1 < (voxels.max_z + floor(min_z)); z1 += voxel_size) {
                //std::cout << "Voxel value = " << voxels(x1,y1,z1) << std::endl;
                new_vertices.emplace_back(Point(x1, y1, z1));
            }
        }
    }
//    std::cout << new_vertices[0] << std::endl;
    std::ofstream output_file(file_out);
    if (output_file.is_open())
    {
        // write vertices
        for(Point p:new_vertices)
        {
            output_file << "v " << p[0] << " " << p[1] << " " << p[2] << "\n";
        }
        // write faces
        // TODO
        std::cout << "file is written in " << file_out << std::endl;
        output_file.close();
    }
    else std::cout << "Unable to open file";

    // Compute and print out the building's volume
    // TODO

    return 0;
}
