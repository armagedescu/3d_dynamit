#include "enabler.h"
#ifdef __DYNAMIT_SPHERE_DODECAHEDRON_CPP__

#define _USE_MATH_DEFINES
#include <cmath>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <array>
#include <map>
#include <set>
#include <utility>
#include <cassert>
#include <algorithm>
#include <Dynamit.h>
#include <config.h>
#include <callbacks.h>

using namespace dynamit;

//========================================
// Dodecahedron Sphere Generator
//========================================

constexpr int DODECAHEDRON_VERTICES = 20;
constexpr int DODECAHEDRON_FACES = 12;
constexpr int PENTAGON_SIDES = 5;

enum class WindingOrder {
    CCW = 0,  // Counter-clockwise (OpenGL standard)
    CW = 1    // Clockwise
};

enum class CoatingType {
    External,  // External surface only (default)
    Internal,  // Internal surface only
    Both       // Both meshes separate
};

struct Vertex {
    float x, y, z;      // position
    float nx, ny, nz;   // normal

    Vertex(float x = 0, float y = 0, float z = 0)
        : x(x), y(y), z(z), nx(0), ny(0), nz(0) {
    }

    Vertex& operator+=(const Vertex& other) {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    Vertex& operator*=(float scalar) {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }

    Vertex& normalizeExterior() {
        float len = std::hypot(x, y, z);
        x /= len;
        y /= len;
        z /= len;
        nx = x;
        ny = y;
        nz = z;
        return *this;
    }

    Vertex& normalizeInterior() {
        float len = std::hypot(x, y, z);
        x /= len;
        y /= len;
        z /= len;
        nx = -x;
        ny = -y;
        nz = -z;
        return *this;
    }

    Vertex operator+(const Vertex& other) const {
        return Vertex(x + other.x, y + other.y, z + other.z);
    }

    Vertex operator*(float scalar) const {
        return Vertex(x * scalar, y * scalar, z * scalar);
    }

    float distance(const Vertex& other) const {
        return std::hypot(x - other.x, y - other.y, z - other.z);
    }
};

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};

struct DodecahedronTopology {
    std::vector<Vertex> vertices;
    std::vector<std::pair<int, int>> edges;
    std::vector<std::array<int, PENTAGON_SIDES>> pentagonFaces;
    std::vector<Vertex> pentagonCenters;
};

inline void addTriangle(std::vector<uint32_t>& indices,
    uint32_t v0, uint32_t v1, uint32_t v2,
    WindingOrder order) {
    if (order == WindingOrder::CCW) {
        indices.push_back(v0);
        indices.push_back(v1);
        indices.push_back(v2);
    }
    else {
        indices.push_back(v0);
        indices.push_back(v2);
        indices.push_back(v1);
    }
}

std::vector<Vertex> generateDodecahedronVerticesBase() {
    const float PHI = (1.0f + std::sqrt(5.0f)) / 2.0f;
    const float INV_PHI = 1.0f / PHI;

    std::vector<Vertex> vertices;
    vertices.reserve(DODECAHEDRON_VERTICES);

    for (int x : {-1, 1})
        for (int y : {-1, 1})
            for (int z : {-1, 1})
                vertices.emplace_back(x, y, z);

    for (int s1 : {-1, 1})
        for (int s2 : {-1, 1})
            vertices.emplace_back(PHI * s1, 0, INV_PHI * s2);

    for (int s1 : {-1, 1})
        for (int s2 : {-1, 1})
            vertices.emplace_back(INV_PHI * s2, PHI * s1, 0);

    for (int s1 : {-1, 1})
        for (int s2 : {-1, 1})
            vertices.emplace_back(0, INV_PHI * s2, PHI * s1);

    return vertices;
}

// Original implementation - used for generating precomputed data
std::vector<std::array<int, PENTAGON_SIDES>> findPentagonalFacesOriginal(
    const std::vector<Vertex>& vertices,
    const std::vector<std::pair<int, int>>& edges) {

    std::vector<std::vector<int>> adjacency(vertices.size());
    for (const std::pair<int, int>& edge : edges) {
        adjacency[edge.first].push_back(edge.second);
        adjacency[edge.second].push_back(edge.first);
    }

    std::vector<std::array<int, PENTAGON_SIDES>> faces;
    std::set<std::set<int>> foundFaces;

    for (size_t start = 0; start < vertices.size(); start++) {
        const std::vector<int>& neighbors = adjacency[start];

        for (size_t i = 0; i < neighbors.size(); i++) {
            int v1 = neighbors[i];
            
            for (int v2 : adjacency[v1]) {
                if (v2 == static_cast<int>(start)) continue;
                
                for (int v3 : adjacency[v2]) {
                    if (v3 == static_cast<int>(start) || v3 == v1) continue;

                    for (int v4 : adjacency[v3]) {
                        if (v4 == v2 || v4 == v1 || v4 == static_cast<int>(start)) continue;

                        bool closesLoop = false;
                        for (int n : adjacency[v4])
                            if (n == static_cast<int>(start)) {
                                closesLoop = true;
                                break;
                            }

                        if (closesLoop) {
                            std::set<int> faceSet = { static_cast<int>(start), v1, v2, v3, v4 };

                            if (foundFaces.find(faceSet) == foundFaces.end()) {
                                const Vertex& vert0 = vertices[start];
                                const Vertex& vert1 = vertices[v1];
                                const Vertex& vert2 = vertices[v2];
                                
                                float e1x = vert1.x - vert0.x;
                                float e1y = vert1.y - vert0.y;
                                float e1z = vert1.z - vert0.z;
                                
                                float e2x = vert2.x - vert0.x;
                                float e2y = vert2.y - vert0.y;
                                float e2z = vert2.z - vert0.z;
                                
                                float nx = e1y * e2z - e1z * e2y;
                                float ny = e1z * e2x - e1x * e2z;
                                float nz = e1x * e2y - e1y * e2x;
                                
                                float dot = nx * vert0.x + ny * vert0.y + nz * vert0.z;
                                
                                if (dot < 0) {
                                    faces.push_back({ static_cast<int>(start), v1, v2, v3, v4 });
                                } else {
                                    faces.push_back({ static_cast<int>(start), v4, v3, v2, v1 });
                                }
                                
                                foundFaces.insert(faceSet);
                            }
                        }
                    }
                }
            }
        }
    }

    return faces;
}

// Precomputed pentagonal faces with correct CCW winding order (outward-facing)
// Each face lists 5 vertex indices in order around the pentagon
constexpr std::array<std::array<int, PENTAGON_SIDES>, 12> DODECAHEDRON_FACES_INDICES = {{
    {0, 16, 17, 2, 8},
    {0, 8, 9, 1, 12},
    {0, 12, 13, 4, 16},
    {1, 9, 3, 19, 18},
    {1, 18, 5, 13, 12},
    {2, 14, 3, 9, 8},
    {2, 17, 6, 15, 14},
    {3, 14, 15, 7, 19},
    {4, 10, 6, 17, 16},
    {4, 13, 5, 11, 10},
    {5, 18, 19, 7, 11},
    {6, 10, 11, 7, 15}
}};

std::vector<std::array<int, PENTAGON_SIDES>> findPentagonalFaces(
    const std::vector<Vertex>& /*vertices*/,
    const std::vector<std::pair<int, int>>& /*edges*/) {
    return std::vector<std::array<int, PENTAGON_SIDES>>(
        DODECAHEDRON_FACES_INDICES.begin(), 
        DODECAHEDRON_FACES_INDICES.end()
    );
}

// Precomputed edges for dodecahedron based on vertex generation order in generateDodecahedronVerticesBase()
// Indices 0-7:   cube vertices (±1, ±1, ±1)
// Indices 8-11:  (±PHI, 0, ±1/PHI)
// Indices 12-15: (±1/PHI, ±PHI, 0)
// Indices 16-19: (0, ±1/PHI, ±PHI)
constexpr std::array<std::pair<int, int>, 30> DODECAHEDRON_EDGES = {{
    {0, 8},  {0, 12}, {0, 16},   // vertex 0: (-1,-1,-1)
    {1, 9},  {1, 12}, {1, 18},   // vertex 1: (-1,-1,+1)
    {2, 8},  {2, 14}, {2, 17},   // vertex 2: (-1,+1,-1)
    {3, 9},  {3, 14}, {3, 19},   // vertex 3: (-1,+1,+1)
    {4, 10}, {4, 13}, {4, 16},   // vertex 4: (+1,-1,-1)
    {5, 11}, {5, 13}, {5, 18},   // vertex 5: (+1,-1,+1)
    {6, 10}, {6, 15}, {6, 17},   // vertex 6: (+1,+1,-1)
    {7, 11}, {7, 15}, {7, 19},   // vertex 7: (+1,+1,+1)
    {8, 9},   {10, 11},          // PHI-x axis pairs
    {12, 13}, {14, 15},          // PHI-y axis pairs
    {16, 17}, {18, 19}           // PHI-z axis pairs
}};

std::vector<std::pair<int, int>> findEdges(const std::vector<Vertex>& /*vertices*/) {
    return std::vector<std::pair<int, int>>(DODECAHEDRON_EDGES.begin(), DODECAHEDRON_EDGES.end());
}

DodecahedronTopology generateDodecahedronTopology() {
    DodecahedronTopology topology;
    topology.vertices = generateDodecahedronVerticesBase();

    for (Vertex& v : topology.vertices)
        v.normalizeExterior();

    topology.edges = findEdges(topology.vertices);
    topology.pentagonFaces = findPentagonalFaces(topology.vertices, topology.edges);

    topology.pentagonCenters.reserve(DODECAHEDRON_FACES);
    for (const std::array<int, PENTAGON_SIDES>& pentagon : topology.pentagonFaces) {
        Vertex center(0, 0, 0);
        for (int idx : pentagon)
            center += topology.vertices[idx];
        center *= 1.0f / PENTAGON_SIDES;
        center.normalizeExterior();
        topology.pentagonCenters.push_back(center);
    }

    return topology;
}

const DodecahedronTopology& getDodecahedronTopology() {
    static DodecahedronTopology topology = generateDodecahedronTopology();
    return topology;
}

Mesh generateSubdividedMeshExternal(int subdivisionLevel, WindingOrder order) {
    assert(subdivisionLevel > 0);

    Mesh mesh;

    int N = subdivisionLevel;
    int verticesPerPentagon = (PENTAGON_SIDES * (N + 1) * (N + 2)) / 2 + 1;
    int trianglesPerPentagon = PENTAGON_SIDES * (N + 1) * (N + 1);
    int indicesPerPentagon = trianglesPerPentagon * 3;

    mesh.vertices.reserve(DODECAHEDRON_FACES * verticesPerPentagon);
    mesh.indices.reserve(DODECAHEDRON_FACES * indicesPerPentagon);

    const DodecahedronTopology& topology = getDodecahedronTopology();

    for (size_t faceIdx = 0; faceIdx < topology.pentagonFaces.size(); faceIdx++) {
        const std::array<int, PENTAGON_SIDES>& pentagon = topology.pentagonFaces[faceIdx];
        const Vertex& center = topology.pentagonCenters[faceIdx];

        std::vector<std::vector<int>> rings;
        rings.reserve(N + 1);

        for (int ring = 0; ring <= N; ring++) {
            float t = static_cast<float>(ring) / (N + 1);
            int numSegments = N - ring + 1;

            std::vector<int> ringVertices;
            ringVertices.reserve(PENTAGON_SIDES * numSegments);

            for (int edge = 0; edge < PENTAGON_SIDES; edge++) {
                int v0_idx = pentagon[edge];
                int v1_idx = pentagon[(edge + 1) % PENTAGON_SIDES];

                const Vertex& corner0 = topology.vertices[v0_idx];
                const Vertex& corner1 = topology.vertices[v1_idx];

                Vertex radialCorner0 = corner0 * (1.0f - t) + center * t;
                Vertex radialCorner1 = corner1 * (1.0f - t) + center * t;
                radialCorner0.normalizeExterior();
                radialCorner1.normalizeExterior();

                for (int seg = 0; seg < numSegments; seg++) {
                    float s = static_cast<float>(seg) / numSegments;
                    Vertex v = radialCorner0 * (1.0f - s) + radialCorner1 * s;
                    v.normalizeExterior();

                    int vertexIdx = static_cast<int>(mesh.vertices.size());
                    mesh.vertices.push_back(v);
                    ringVertices.push_back(vertexIdx);
                }
            }

            rings.push_back(ringVertices);
        }

        int centerIdx = static_cast<int>(mesh.vertices.size());
        mesh.vertices.push_back(center);

        for (int ring = 0; ring < N; ring++) {
            const std::vector<int>& outerRing = rings[ring];
            const std::vector<int>& innerRing = rings[ring + 1];

            int outerSegments = N - ring + 1;
            int innerSegments = N - ring;

            for (int edge = 0; edge < PENTAGON_SIDES; edge++) {
                int outerStart = edge * outerSegments;
                int innerStart = edge * innerSegments;

                for (int i = 0; i < innerSegments; i++) {
                    int o0 = outerRing[outerStart + i];
                    int o1 = outerRing[outerStart + i + 1];
                    int i0 = innerRing[innerStart + i];

                    addTriangle(mesh.indices, o0, o1, i0, order);

                    // Second triangle of the quad
                    int i1 = innerRing[(innerStart + i + 1) % innerRing.size()];
                    addTriangle(mesh.indices, o1, i1, i0, order);
                }

                // Edge wrap triangle connecting last outer vertex to next edge
                int o_last = outerRing[outerStart + outerSegments - 1];
                int o_next = outerRing[(outerStart + outerSegments) % outerRing.size()];
                int i_last = innerRing[(innerStart + innerSegments) % innerRing.size()];

                addTriangle(mesh.indices, o_last, o_next, i_last, order);
            }
        }

        const std::vector<int>& lastRing = rings[N];
        for (size_t i = 0; i < lastRing.size(); i++) {
            int next = (i + 1) % lastRing.size();
            addTriangle(mesh.indices, lastRing[i], lastRing[next], centerIdx, order);
        }
    }

    return mesh;
}

//========================================
// Dynamit Integration Helpers
//========================================

// Convert indexed mesh to flat vertex arrays for Dynamit
void meshToFlatArrays(const Mesh& mesh, std::vector<float>& verts, std::vector<float>& norms) {
    verts.clear();
    norms.clear();
    
    size_t triangleCount = mesh.indices.size() / 3;
    verts.reserve(triangleCount * 9);  // 3 vertices * 3 coords
    norms.reserve(triangleCount * 9);  // 3 normals * 3 coords
    
    for (size_t i = 0; i < mesh.indices.size(); i++) {
        const Vertex& v = mesh.vertices[mesh.indices[i]];
        verts.push_back(v.x);
        verts.push_back(v.y);
        verts.push_back(v.z);
        
        norms.push_back(v.nx);
        norms.push_back(v.ny);
        norms.push_back(v.nz);
    }
}

// Convert mesh to indexed arrays for glDrawElements
void meshToIndexedArrays(const Mesh& mesh,
                         std::vector<float>& verts,
                         std::vector<float>& norms,
                         std::vector<float>& texCoords,
                         std::vector<uint32_t>& indices) {
    verts.clear();
    norms.clear();
    texCoords.clear();
    indices.clear();
    
    verts.reserve(mesh.vertices.size() * 3);
    norms.reserve(mesh.vertices.size() * 3);
    texCoords.reserve(mesh.vertices.size() * 2);
    indices.reserve(mesh.indices.size());
    
    // Output unique vertices
    for (const Vertex& v : mesh.vertices) {
        verts.push_back(v.x);
        verts.push_back(v.y);
        verts.push_back(v.z);
        
        norms.push_back(v.nx);
        norms.push_back(v.ny);
        norms.push_back(v.nz);
        
        // Spherical UV mapping
        float u = std::atan2(v.nz, v.nx) / (2.0f * static_cast<float>(M_PI)) + 0.5f;
        float vCoord = std::asin(v.ny) / static_cast<float>(M_PI) + 0.5f;
        texCoords.push_back(u);
        texCoords.push_back(vCoord);
    }
    
    // Copy indices directly
    indices.assign(mesh.indices.begin(), mesh.indices.end());
}

// Overload without texture coordinates
void meshToIndexedArrays(const Mesh& mesh,
                         std::vector<float>& verts,
                         std::vector<float>& norms,
                         std::vector<uint32_t>& indices) {
    std::vector<float> texCoords;
    meshToIndexedArrays(mesh, verts, norms, texCoords, indices);
}

// Generate sphere mesh with specified subdivision level
Mesh generateSphereMesh(int subdivisionLevel = 3, WindingOrder order = WindingOrder::CCW) {
    return generateSubdividedMeshExternal(subdivisionLevel, order);
}

//========================================
// Main Application
//========================================
#ifndef NDEBUG
std::vector<std::pair<int, int>> findEdgesOriginal(const std::vector<Vertex>& vertices) {
    std::vector<float> allDistances;
    for (size_t i = 0; i < vertices.size(); i++) {
        for (size_t j = i + 1; j < vertices.size(); j++) {
            allDistances.push_back(vertices[i].distance(vertices[j]));
        }
    }

    std::sort(allDistances.begin(), allDistances.end());

    // Find largest gap to determine edge threshold
    float maxGap = 0;
    int gapIndex = 0;
    for (int i = 1; i < 50 && i < allDistances.size(); i++) {
        float gap = allDistances[i] - allDistances[i - 1];
        if (gap > maxGap) {
            maxGap = gap;
            gapIndex = i;
        }
    }

    const float THRESHOLD = allDistances[gapIndex - 1] * 1.01f;

    std::vector<std::pair<int, int>> edges;
    for (size_t i = 0; i < vertices.size(); i++)
        for (size_t j = i + 1; j < vertices.size(); j++)
            if (vertices[i].distance(vertices[j]) < THRESHOLD)
                edges.push_back({ static_cast<int>(i), static_cast<int>(j) });

    return edges;
}

void printEdgesForConstexpr(const std::vector<Vertex>& vertices) {
    auto edges = findEdgesOriginal(vertices);
    std::sort(edges.begin(), edges.end());
    
    std::cout << "constexpr std::array<std::pair<int, int>, " << edges.size() << "> DODECAHEDRON_EDGES = {{\n";
    for (size_t i = 0; i < edges.size(); i++) {
        std::cout << "    {" << edges[i].first << ", " << edges[i].second << "}";
        if (i < edges.size() - 1) std::cout << ",";
        std::cout << "\n";
    }
    std::cout << "}};\n";
}

void printFacesForConstexpr(const std::vector<Vertex>& vertices, const std::vector<std::pair<int, int>>& edges) {
    auto faces = findPentagonalFacesOriginal(vertices, edges);
    
    std::cout << "constexpr std::array<std::array<int, PENTAGON_SIDES>, " << faces.size() << "> DODECAHEDRON_FACES_INDICES = {{\n";
    for (size_t i = 0; i < faces.size(); i++) {
        std::cout << "    {" << faces[i][0] << ", " << faces[i][1] << ", " 
                  << faces[i][2] << ", " << faces[i][3] << ", " << faces[i][4] << "}";
        if (i < faces.size() - 1) std::cout << ",";
        std::cout << "\n";
    }
    std::cout << "}};\n";
}

void verifyEdges(const std::vector<Vertex>& vertices) {
    auto computed = findEdgesOriginal(vertices);
    auto precomputed = findEdges(vertices);
    std::sort(computed.begin(), computed.end());
    std::sort(precomputed.begin(), precomputed.end());
    
    if (computed != precomputed) {
        std::cout << "Edge mismatch! Correct edges:\n";
        printEdgesForConstexpr(vertices);
        assert(false && "Edge mismatch!");
    }
}
#endif
int main() {
#ifndef NDEBUG
    verifyEdges(generateDodecahedronVerticesBase());
    
    // Print faces for precomputation
    auto verts = generateDodecahedronVerticesBase();
    auto edges = findEdgesOriginal(verts);
    printFacesForConstexpr(verts, edges);
#endif
    //GLFWwindow* window = openglWindowInit(1280, 720);
    GLFWwindow* window = openglWindowInit(720, 720);
    if (!window)
        return -1;
        
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "=== Dynamit Dodecahedron Sphere Demo ===\n\n";
    
    // Generate three spheres with different subdivision levels
    std::cout << "Generating sphere meshes...\n";
    
    // Sphere 1: Low poly (subdivision level 1)
    Mesh meshLow = generateSphereMesh(1, WindingOrder::CCW);
    std::vector<float> vertsLow, normsLow;
    meshToFlatArrays(meshLow, vertsLow, normsLow);
    //std::cout << "Low poly sphere: " << vertsLow.size() / 3 
    //          << " vertices, " << vertsLow.size() / 9 << " triangles\n";
    
    //// Sphere 2: Medium poly (subdivision level 3)
    Mesh meshMed = generateSphereMesh(3, WindingOrder::CCW);
    std::vector<float> vertsMed, normsMed;
    meshToFlatArrays(meshMed, vertsMed, normsMed);
    //std::cout << "Medium poly sphere: " << vertsMed.size() / 3 
    //          << " vertices, " << vertsMed.size() / 9 << " triangles\n";
    ////
    ////// Sphere 3: High poly (subdivision level 5)
    Mesh meshHigh = generateSphereMesh(5, WindingOrder::CCW);
    std::vector<float> vertsHigh, normsHigh;
    meshToFlatArrays(meshHigh, vertsHigh, normsHigh);
    //std::cout << "High poly sphere: " << vertsHigh.size() / 3 
    //          << " vertices, " << vertsHigh.size() / 9 << " triangles\n\n";

	//// Indexed draw example:
    //std::vector<float> verts, norms, texCoords;
    //std::vector<uint32_t> indices;
    //meshToIndexedArrays(mesh, verts, norms, texCoords, indices
    //// Draw with:
    //// glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    
    // Create Dynamit shapes with lighting
    
    // Left sphere - Low poly, red
    Dynamit sphereLow;
    sphereLow.withVertices3d(vertsLow)
             .withNormals3d(normsLow)
             .withConstColor(1.0f, 0.2f, 0.2f, 1.0f)  // Red
             .withConstTranslation(-0.5f, 0.5f, -0.0f, 0.0f)
        //.withConstTranslation(-0.0f, 0.0f, -0.0f, 0.0f)
             .withConstLightDirection(-1.0f, -1.0f, 1.0f);
    
    // Center sphere - Medium poly, green
    Dynamit sphereMed;
    sphereMed.withVertices3d(vertsMed)
             .withNormals3d(normsMed)
             .withConstColor(0.2f, 1.0f, 0.2f, 1.0f)  // Green
             .withConstTranslation(0.0f, 0.0f, 0.5f, 0.0f)
             .withConstLightDirection(-1.0f, -1.0f, 1.0f);
    //
    //// Right sphere - High poly, blue
    Dynamit sphereHigh;
    sphereHigh.withVertices3d(vertsHigh)
              .withNormals3d(normsHigh)
              .withConstColor(0.2f, 0.4f, 1.0f, 1.0f)  // Blue
              .withConstTranslation(-0.3f, -0.5f, 0.2f, 0.0f)
              .withConstLightDirection(-1.0f, -1.0f, 1.0f);
    
    // Optional: Log generated shaders for first sphere
    std::cout << "Generated shaders for medium poly sphere:\n";
    //sphereLow.logGeneratedShaders();
    sphereMed.logGeneratedShaders();
    
    // OpenGL setup
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);  // Enable backface culling for better performance
    glCullFace(GL_BACK);
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);  // Dark blue background
    
    std::cout << "\nRendering...\n";
    std::cout << "Press ESC to exit\n";
    
    // Render loop
    while (!glfwWindowShouldClose(window)) {
        glPolygonMode(GL_FRONT_AND_BACK, glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS ? GL_LINE : GL_FILL);
        processInputs(window);
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Draw all three spheres
        //sphereLow.drawTriangles();
        sphereMed.drawTriangles();
        //sphereHigh.drawTriangles();
        
        glfwPollEvents();
        glfwSwapBuffers(window);
    }
    
    glfwTerminate();
    std::cout << "Application closed successfully\n";
    return 0;
}

#endif