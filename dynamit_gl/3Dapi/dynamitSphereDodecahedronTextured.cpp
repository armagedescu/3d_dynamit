#include "enabler.h"
#ifdef __DYNAMIT_SPHERE_DODECAHEDRON_TEXTURED_CPP__

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
#include <TextureLoader.h>
#include <stb_image.h>

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

    Vertex operator+(const Vertex& other) const {
        return Vertex(x + other.x, y + other.y, z + other.z);
    }

    Vertex operator*(float scalar) const {
        return Vertex(x * scalar, y * scalar, z * scalar);
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

// Precomputed pentagonal faces with correct CCW winding order (outward-facing)
// Each face lists 5 vertex indices in order around the pentagon
constexpr std::array<std::array<int, PENTAGON_SIDES>, 12> DODECAHEDRON_FACES_INDICES = { {
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
constexpr std::array<std::pair<int, int>, 30> DODECAHEDRON_EDGES = { {
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
} };

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

Mesh generateSphereMesh(int subdivisionLevel = 3, WindingOrder order = WindingOrder::CCW) {
    return generateSubdividedMeshExternal(subdivisionLevel, order);
}

//========================================
// Textured Shaders
//========================================
const char* texturedVertexShader = R"(
#version 330 core
layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;

out vec3 normalVary;
out vec2 texCoordVary;

uniform vec4 constTranslation;

void main() {
    gl_Position = vec4(vertex, 1.0) + constTranslation;
    normalVary = normal;
    texCoordVary = texCoord;
}
)";

const char* texturedFragmentShader = R"(
#version 330 core
precision mediump float;

in vec3 normalVary;
in vec2 texCoordVary;

out vec4 fragColor;

uniform sampler2D sphereTexture;
const vec3 lightDirection = vec3(-1.0, -1.0, 1.0);

void main() {
    float prod = -dot(normalize(lightDirection), normalize(normalVary));
    prod = max(prod, 0.2);
    vec4 texColor = texture(sphereTexture, texCoordVary);
    fragColor = vec4(texColor.rgb * prod, 1.0);
    fragColor = vec4(texColor.rgba * prod);
}
)";

//========================================
// Main Application
//========================================
int main() {
    GLFWwindow* window = openglWindowInit(720, 720);
    if (!window)
        return -1;

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "=== Dynamit Textured Dodecahedron Sphere Demo ===\n\n";

    // Load texture
    stbi_set_flip_vertically_on_load(true);
    unsigned int texture = LoadTexture("bitmaps/world_map_texture.png");

    // Generate sphere mesh
    Mesh mesh = generateSphereMesh(4, WindingOrder::CCW);

    // Build interleaved data: pos(3) + normal(3) + uv(2) = 8 floats per vertex
    std::vector<float> strideData;
    strideData.reserve(mesh.vertices.size() * 8);

    for (const Vertex& v : mesh.vertices) {
        strideData.push_back(v.x);
        strideData.push_back(v.y);
        strideData.push_back(v.z);
        strideData.push_back(v.nx);
        strideData.push_back(v.ny);
        strideData.push_back(v.nz);
        // Spherical UV mapping
        float u = std::atan2(v.nz, v.nx) / (2.0f * static_cast<float>(M_PI)) + 0.5f;
        float vCoord = std::asin(std::clamp(v.ny, -1.0f, 1.0f)) / static_cast<float>(M_PI) + 0.5f;
        strideData.push_back(u);
        strideData.push_back(vCoord);
    }

    std::cout << "Sphere: " << mesh.vertices.size() << " vertices, "
        << mesh.indices.size() / 3 << " triangles\n";

    // Create Dynamit with stride layout and custom shaders
    Dynamit sphere;
    sphere.withStride(strideData, 8 * sizeof(float))
        .withStrideVertices(3)
        .withStrideNormals(3)
        .withStrideTexCoords(2)
        .withIndices(mesh.indices)
        .withShaderSources(texturedVertexShader, texturedFragmentShader);

    // Build and use program, then set uniforms
    sphere.useProgram();

    GLint texLoc = glGetUniformLocation(sphere.program.id, "sphereTexture");
    GLint transLoc = glGetUniformLocation(sphere.program.id, "constTranslation");

    glUniform1i(texLoc, 0);
    glUniform4f(transLoc, 0.0f, 0.0f, 0.0f, 0.0f);

    // OpenGL setup
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glClearColor(0.5f, 0.5f, 0.55f, 1.0f);

    std::cout << "\nRendering textured sphere...\n";
    std::cout << "Press F11 for wireframe, ESC to exit\n";

    while (!glfwWindowShouldClose(window)) {
        glPolygonMode(GL_FRONT_AND_BACK, glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS ? GL_LINE : GL_FILL);
        processInputs(window);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Bind texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        sphere.drawTrianglesIndexed();

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    std::cout << "Application closed successfully\n";
    return 0;
}

#endif