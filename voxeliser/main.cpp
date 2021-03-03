#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>
#include <cstring>
#include <cassert>
#include <algorithm>
#include <unordered_set>
# include <deque>
#include <chrono>

#include "Point.h"
#include "Rows.h"
#include "VoxelGrid.h"

auto start = std::chrono::high_resolution_clock::now();

float signed_volume(const Point &a, const Point &b, const Point &c, const Point &d) {
    const Point cross = (b-d).cross(c-d);
    return ((a-d).dot(cross)) / 6;
}

bool intersects(const Point &orig, const Point &dest, const Point &v0, const Point &v1, const Point &v2) {
    if(signed_volume(v0, v1, v2, orig) * signed_volume(v0, v1, v2, dest) <= 0) {
        float test1 = signed_volume(orig, dest, v0, v1) * signed_volume(orig, dest, v0, v2);
        float test2 = signed_volume(orig, dest, v1, v0) * signed_volume(orig, dest, v1, v2);
        float test3 = signed_volume(orig, dest, v2, v0) * signed_volume(orig, dest, v2, v1);
        if (test1 < 0 && test2 < 0 && test3 < 0) {
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

int main(int argc, const char *argv[]) {
    const char *file_in = "../bag_bk.obj";
    const char *file_out = "vox.obj";
    float voxel_size = 1;

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

    // Create grid
    int fix_x = ceil((max_x - floor(max_x) + min_x - floor(min_x)) / voxel_size);
    int fix_y = ceil((max_y - floor(max_y) + min_y - floor(min_y)) / voxel_size);
    int fix_z = ceil((max_z - floor(max_z) + min_z - floor(min_z)) / voxel_size);
    int row_x = floor(((max_x) - (min_x)) / voxel_size) + fix_x;
    int row_y = floor(((max_y) - (min_y)) / voxel_size) + fix_y;
    int row_z = floor(((max_z) - (min_z)) / voxel_size) + fix_z;
    Rows rows(row_x, row_y, row_z);
    VoxelGrid voxels(rows.x, rows.y, rows.z);

    // Voxelise
    std::cout << "Voxels total: " << voxels.max_x * voxels.max_y * voxels.max_z << std::endl;
    std::cout << "Original voxel grid has: X = " << voxels.max_x << " Y = " << voxels.max_y << " Z = " << voxels.max_z << std::endl;
    for (auto const &triangle: faces) {
        // triangle's vertices
        Point t0 = vertices[triangle[0]], t1 = vertices[triangle[1]], t2 = vertices[triangle[2]];
        // Check bbox validity
        std::vector<Point> box = bbox(t0, t1, t2);
        assert(box[0][0] <= box[1][0] && box[0][1] <= box[1][1] && box[0][2] <= box[1][2]);
        // We define the mini voxel grid
        int fix_x2, fix_y2, fix_z2;
        float diffX = box[1][0] - box[0][0], diffY = box[1][1] - box[0][1], diffZ = box[1][2] - box[0][2];
        if (diffX == 0) {
            fix_x2 = 1;
        }
        else fix_x2 = ceil((box[1][0] - floor(box[1][0]) + box[0][0] - floor(box[0][0])) / voxel_size);
        if (diffY == 0) {
            fix_y2 = 1;
        }
        else fix_y2 = ceil((box[1][1] - floor(box[1][1]) + box[0][1] - floor(box[0][1])) / voxel_size);
        if (diffZ == 0) {
            fix_z2 = 1;
        }
        else fix_z2 = ceil((box[1][2] - floor(box[1][2]) + box[0][2] - floor(box[0][2])) / voxel_size);
        int offsetX = (floor(box[0][0]) - floor(min_x)) / voxel_size -1,
                offsetY = (floor(box[0][1]) - floor(min_y)) / voxel_size -1,
                offsetZ = (floor(box[0][2])  - floor(min_z)) / voxel_size -1;
        int row_x2 = ((floor(box[1][0]) - floor(box[0][0])) / voxel_size) + fix_x2 +3;
        int row_y2 = ((floor(box[1][1]) - floor(box[0][1])) / voxel_size) + fix_y2 +3;
        int row_z2 = ((floor(box[1][2]) - floor(box[0][2])) / voxel_size) + fix_z2 +3;
        Rows miniRows(row_x2, row_y2, row_z2);
        VoxelGrid subset(miniRows.x, miniRows.y, miniRows.z);
        assert(subset.total_voxels < voxels.total_voxels);
        // loop through the subset
        for (int xSub = 0; xSub < subset.max_x-1; xSub += 1) {
            for (int ySub = 0; ySub < subset.max_y-1; ySub += 1) {
                for (int zSub = 0; zSub < subset.max_z-1; zSub += 1) {
                    // Equivalence with big grid
                    int bigX = offsetX + xSub, bigY = offsetY + ySub, bigZ = offsetZ + zSub;
                    // Voxel's target
                    Point targetA1(floor(min_x) + (bigX + 0.5) * voxel_size, floor(min_y) + bigY * voxel_size, floor(min_z) + (bigZ + 0.5) * voxel_size);
                    Point targetA2(floor(min_x) + (bigX + 0.5) * voxel_size, floor(min_y) + (bigY + 1) * voxel_size, floor(min_z) + (bigZ + 0.5) * voxel_size);
                    Point targetB1(floor(min_x) + bigX * voxel_size, floor(min_y) + (bigY + 0.5) * voxel_size, floor(min_z) + (bigZ + 0.5) * voxel_size);
                    Point targetB2(floor(min_x) + (bigX + 1) * voxel_size, floor(min_y) + (bigY + 0.5) * voxel_size, floor(min_z) + (bigZ + 0.5) * voxel_size);
                    Point targetC1(floor(min_x) + (bigX + 0.5) * voxel_size, floor(min_y) + (bigY + 0.5) * voxel_size, floor(min_z) + bigZ * voxel_size);
                    Point targetC2(floor(min_x) + (bigX + 0.5) * voxel_size, floor(min_y) + (bigY + 0.5) * voxel_size, floor(min_z) + (bigZ + 1) * voxel_size);
                    // We assign 1 to the voxel intersecting the triangular mesh
                    if (intersects(targetA1, targetA2, t0, t1, t2) || intersects(targetB1, targetB2, t0, t1, t2) || intersects(targetC1, targetC2, t0, t1, t2)) {
                        if (bigX < voxels.max_x && bigY < voxels.max_y && bigZ < voxels.max_z) {
                            voxels(bigX, bigY, bigZ) = 2;
                            if (voxel_size < 1) {
                                if (bigX+1 < voxels.max_x) {voxels(bigX+1, bigY, bigZ) = 2;}
                                if (bigX-1 >= 0) {voxels(bigX-1, bigY, bigZ) = 2;}
                                if (bigY+1 < voxels.max_y) {voxels(bigX, bigY+1, bigZ) = 2;}
                                if (bigY-1 >= 0) {voxels(bigX, bigY-1, bigZ) = 2;}
                                if (bigZ+1 < voxels.max_z) {voxels(bigX, bigY, bigZ+1) = 2;}
                                if (bigZ-1 >= 0) {voxels(bigX, bigY, bigZ-1) = 2;}
                            }
                        }
                        else continue;
                    }
                }
            }
        }
    }

    // Fill model: the queue starts with an angle and gets filled with its neighbors (if voxel value == 0)
    std::deque<std::vector<int>> q;
    q.push_back({0,0,0});
    assert (voxels(0,0,0) == 0);
    voxels(0,0,0) = 1;
    while (!q.empty()){
        std::vector<int> vec = q.front();
        q.pop_front();
        if (((vec[0]+1 >= 0) && (vec[0]+1 < voxels.max_x)) && (voxels(vec[0] + 1, vec[1], vec[2]) == 0))
        {
            std::vector<int> v1{vec[0] +1, vec[1], vec[2]};
            voxels(vec[0] +1, vec[1], vec[2]) = 1;
            q.push_back(v1);
        }
        if (((vec[1]+1 >= 0) && (vec[1]+1 < voxels.max_y)) && (voxels(vec[0], vec[1] + 1, vec[2]) == 0))
        {
            std::vector<int> v2{vec[0], vec[1] + 1, vec[2]};
            voxels(vec[0], vec[1] +1, vec[2]) = 1;
            q.push_back(v2);
        }
        if (((vec[2]+1 >= 0) && (vec[2]+1 < voxels.max_z)) && (voxels(vec[0], vec[1], vec[2]+1) == 0))
        {
            std::vector<int> v3{vec[0], vec[1], vec[2] + 1};
            voxels(vec[0], vec[1], vec[2] +1) = 1;
            q.push_back(v3);
        }
        if (((vec[0]-1 >= 0) && (vec[0]-1 < voxels.max_x)) && (voxels(vec[0]-1, vec[1], vec[2]) == 0))
        {
            std::vector<int> v4{vec[0]-1, vec[1], vec[2]};
            voxels(vec[0]-1, vec[1], vec[2]) = 1;
            q.push_back(v4);
        }
        if (((vec[1]-1 >= 0) && (vec[1]-1 < voxels.max_y)) && (voxels(vec[0], vec[1]-1, vec[2]) == 0))
        {
            std::vector<int> v5{vec[0], vec[1]-1, vec[2]};
            voxels(vec[0], vec[1]-1, vec[2]) = 1;
            q.push_back(v5);
        }
        if (((vec[2]-1 >= 0) && (vec[2]-1 < voxels.max_z)) && (voxels(vec[0], vec[1], vec[2]-1) == 0))
        {
            std::vector<int> v6{vec[0], vec[1], vec[2]-1};
            voxels(vec[0], vec[1], vec[2]-1) = 1;
            q.push_back(v6);
        }
        assert (q.size() <= voxels.total_voxels);
    }

    // Write voxels
    int bound = 0, interior = 0;
    std::vector<Point> new_vertices;
    std::vector<std::vector<unsigned int>> new_faces;
    // "dist" allows to scale the voxel representation (voxel_size * 0.5 gives scale 1 representation)
    float dist = voxel_size * 0.5;
    assert(2 * dist <= voxel_size);
    unsigned int id = 1;
    for (int x1 = 0; x1 < voxels.max_x; x1 += 1) {
        for (int y1 = 0; y1 < voxels.max_y; y1 += 1) {
            for (int z1 = 0; z1 < voxels.max_z; z1 += 1) {
                if (voxels(x1, y1, z1) == 0) {
                    interior++;
                }
                if (voxels(x1, y1, z1) == 2) {
                    bound++;
                }
                if (voxels(x1, y1, z1) != 1) {
                    Point center((floor(min_x) + (x1 + 0.5) * voxel_size), (floor(min_y) + (y1 + 0.5) * voxel_size), (floor(min_z) + (z1 + 0.5) * voxel_size));
                    // Points to store
                    Point p1(center[0] - dist, center[1] - dist, center[2] - dist);
                    Point p2(center[0] + dist, center[1] - dist, center[2] - dist);
                    Point p3(center[0] + dist, center[1] + dist, center[2] - dist);
                    Point p4(center[0] - dist, center[1] + dist, center[2] - dist);
                    Point p5(center[0] - dist, center[1] - dist, center[2] + dist);
                    Point p6(center[0] + dist, center[1] - dist, center[2] + dist);
                    Point p7(center[0] + dist, center[1] + dist, center[2] + dist);
                    Point p8(center[0] - dist, center[1] + dist, center[2] + dist);
                    // Faces to store
                    std::vector<unsigned int> faceDown{id, id+1, id+2, id+3}, faceUp{id+4, id+5, id+6, id+7},
                            faceLeft{id, id+4, id+7, id+3}, faceRight{id+1, id+5, id+6, id+2},
                            faceFront{id, id+1, id+5, id+4}, faceBack{id+3, id+2, id+6, id+7};
                    // Store them in lists
                    new_vertices.emplace_back(p1), new_vertices.emplace_back(p2);
                    new_vertices.emplace_back(p3), new_vertices.emplace_back(p4);
                    new_vertices.emplace_back(p5), new_vertices.emplace_back(p6);
                    new_vertices.emplace_back(p7), new_vertices.emplace_back(p8);
                    new_faces.emplace_back(faceDown), new_faces.emplace_back(faceUp),
                            new_faces.emplace_back(faceRight), new_faces.emplace_back(faceLeft),
                            new_faces.emplace_back(faceFront), new_faces.emplace_back(faceBack);
                    id+=8;
                    }
                }
            }
        }

    std::ofstream output_file(file_out);
    if (output_file.is_open())
    {
        // write vertices
        for(Point p:new_vertices)
        {
            output_file << "v " << p[0] << " " << p[1] << " " << p[2] << "\n";
        }
        output_file << "o 0" << "\n";
        // write faces
        for(std::vector<unsigned int> id:new_faces)
        {
            output_file << "f " << id[0] << " " << id[1] << " " << id[2] << " " << id[3] << "\n";
        }
        std::cout << "file is written in " << file_out << std::endl;
        output_file.close();
    }
    else std::cout << "Unable to open file";

    // Compute and print out the building's volume
    float vox_volume = voxel_size * voxel_size * voxel_size;
    std::cout << std::endl << "Voxel's volume is " << vox_volume << std::endl;
    std::cout << "Number of interior voxel: " << interior << " and number of boundary voxels: " << bound << std::endl;
    float volume = bound * vox_volume/2 + interior * vox_volume;
    std::cout << "The building's volume is " << volume << " meter cubes." << std::endl;

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end-start;
    std::cout << "Execution time:  " << diff.count() << " s\n";

    return 0;
}
