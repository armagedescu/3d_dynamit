//#define _USE_MATH_DEFINES
//#include <cmath>
//#include <GL/glew.h>
//#include <GLFW/glfw3.h>
//#include <iostream>
//#include <vector>
//#include <array>
//
//#include <Dynamit.h>
//#include <builders.h>
//#include <geometry.h>
//#include <config.h>
//#include <callbacks.h>
//
//using namespace dynamit;
//using namespace dynamit::builders;
//using namespace dynamit::geo;
//
//// ============================================================================
//// Constants (from constante.h)
//// ============================================================================
//constexpr int MAXH = 150;
//constexpr int MAXS = 150;
//
//// ============================================================================
//// Heart Cone Builder - Translates HeartConeNormMethod2
//// ============================================================================
//class HeartConeBuilder
//{
//public:
//    static void build(std::vector<float>& verts, std::vector<float>& norms,
//                      int heightSegments = 8, int sectors = 15)
//    {
//        // Ensure even number of sectors
//        sectors += sectors & 1;
//        int nh = std::min(std::max(heightSegments, 1), MAXH);
//        int ns = std::min(std::max(sectors, 3), MAXS);
//        
//        const float rmax = 1.5f;
//        const float ratio = rmax / (ns / 2);
//        const float hstep = 1.0f / nh;
//        const float pistep = 2.0f * static_cast<float>(M_PI) / ns;
//        
//        // Calculate base points and normals for the outer edge
//        std::vector<std::array<float, 3>> basePoints(ns);
//        std::vector<std::array<float, 3>> baseNorms(ns);
//        
//        // First half: 0 to ns/2 (0 to 180 degrees)
//        for (int j = 0; j <= ns / 2; j++)
//        {
//            float dbj = static_cast<float>(j);
//            float rcur = dbj * ratio;  // Archimedes spiral
//            float picur = pistep * dbj;
//            
//            basePoints[j] = { rcur * std::cos(picur), rcur * std::sin(picur), 1.0f };
//            
//            // Calculate normal using cross product of partial derivatives
//            float dFdFi[3] = {
//                rcur * (std::cos(picur) - picur * std::sin(picur)),
//                rcur * (std::sin(picur) + picur * std::cos(picur)),
//                0.0f
//            };
//            float dFdZ[3] = { rcur * std::cos(picur), rcur * std::sin(picur), 1.0f };
//            
//            baseNorms[j] = {
//                dFdFi[1] * dFdZ[2] - dFdFi[2] * dFdZ[1],
//                (dFdFi[2] * dFdZ[0] - dFdFi[0] * dFdZ[2]) * (j < ns / 2 ? 1.0f : -1.0f),
//                dFdFi[0] * dFdZ[1] - dFdFi[1] * dFdZ[0]
//            };
//        }
//        
//        // Second half: mirror symmetry around Y axis
//        for (int j = ns / 2 + 1, z = ns / 2 - 1; j < ns; j++, z--)
//        {
//            basePoints[j] = {  basePoints[z][0], -basePoints[z][1], basePoints[z][2] };
//            baseNorms[j]  = {  baseNorms[z][0],  -baseNorms[z][1],  baseNorms[z][2] };
//        }
//        
//        // Build all layer points
//        std::vector<std::vector<std::array<float, 3>>> points(nh);
//        for (int i = 0; i < nh; i++)
//        {
//            points[i].resize(ns);
//            float hcur = hstep * (i + 1);
//            for (int j = 0; j < ns; j++)
//            {
//                points[i][j] = {
//                    basePoints[j][0] * hcur,
//                    basePoints[j][1] * hcur,
//                    hcur
//                };
//            }
//        }
//        
//        // Generate triangles
//        for (int j = 0; j < ns; j++)
//        {
//            int jnext = (j + 1) % ns;
//            
//            // Layers from bottom to top
//            for (int i = nh - 2; i >= 0; i--)
//            {
//                // Triangle 1
//                addVertex(verts, norms, points[i][j], baseNorms[j]);
//                addVertex(verts, norms, points[i + 1][j], baseNorms[j]);
//                addVertex(verts, norms, points[i + 1][jnext], baseNorms[jnext]);
//                
//                // Triangle 2
//                addVertex(verts, norms, points[i][j], baseNorms[j]);
//                addVertex(verts, norms, points[i + 1][jnext], baseNorms[jnext]);
//                addVertex(verts, norms, points[i][jnext], baseNorms[jnext]);
//            }
//            
//            // Tip triangle
//            std::array<float, 3> tip = { 0.0f, 0.0f, 0.0f };
//            addVertex(verts, norms, tip, baseNorms[j]);
//            addVertex(verts, norms, points[0][j], baseNorms[j]);
//            addVertex(verts, norms, points[0][jnext], baseNorms[jnext]);
//        }
//    }
//    
//private:
//    static void addVertex(std::vector<float>& verts, std::vector<float>& norms,
//                          const std::array<float, 3>& pos, const std::array<float, 3>& norm)
//    {
//        verts.insert(verts.end(), { pos[0], pos[1], pos[2] });
//        norms.insert(norms.end(), { norm[0], norm[1], norm[2] });
//    }
//};
//
//// ============================================================================
//// Axis Lines Shape
//// ============================================================================
//class AxisLines
//{
//public:
//    Dynamit lines;
//    
//    void build()
//    {
//        // X axis (black to red), Y axis (black to green), Z axis (black to blue)
//        std::vector<float> verts = {
//            -5.5f, 0.0f, 0.0f,   5.5f, 0.0f, 0.0f,  // X
//             0.0f,-5.5f, 0.0f,   0.0f, 5.5f, 0.0f,  // Y
//             0.0f, 0.0f,-5.5f,   0.0f, 0.0f, 5.5f   // Z
//        };
//        std::vector<float> colors = {
//            0.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f,  // X: black to red
//            0.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f,  // Y: black to green
//            0.0f, 0.0f, 0.0f,   0.0f, 0.0f, 1.0f   // Z: black to blue
//        };
//        
//        lines.withVertices3d(verts)
//             .withColors3d(colors)
//             .withTransformMatrix4f();
//    }
//    
//    void draw(const mat4<float>& transform)
//    {
//        lines.transformMatrix4f(transform);
//        lines.useProgram();
//        lines.bindVertexArray();
//        glDrawArrays(GL_LINES, 0, 6);
//    }
//};
//
//// ============================================================================
//// Axis Cones
//// ============================================================================
//class AxisCones
//{
//public:
//    Dynamit xCone, yCone, zCone;
//    
//    void build()
//    {
//        std::vector<float> verts, norms;
//        
//        // Build small cone for axis tips (like auxSolidCone(0.1, 0.2))
//        Builder::polar()
//            .sectors_slices(16, 4)
//            .doubleCoated()
//            .buildCone(verts, norms,
//                       scaleMatrix(0.1f, 0.1f, 0.2f));
//        
//        // X cone (red)
//        xCone.withVertices3d(verts)
//             .withNormals3d(norms)
//             .withConstColor(1.0f, 0.0f, 0.0f, 1.0f)
//             .withConstLightDirection(-0.577f, -0.577f, 0.577f)
//             .withTransformMatrix4f();
//        
//        // Y cone (green)
//        yCone.withVertices3d(verts)
//             .withNormals3d(norms)
//             .withConstColor(0.0f, 1.0f, 0.0f, 1.0f)
//             .withConstLightDirection(-0.577f, -0.577f, 0.577f)
//             .withTransformMatrix4f();
//        
//        // Z cone (blue)
//        zCone.withVertices3d(verts)
//             .withNormals3d(norms)
//             .withConstColor(0.0f, 0.0f, 1.0f, 1.0f)
//             .withConstLightDirection(-0.577f, -0.577f, 0.577f)
//             .withTransformMatrix4f();
//    }
//    
//    void draw(const mat4<float>& viewProj)
//    {
//        mat4<float> t;
//        
//        // X cone: at (5.3, 0, 0), rotated 90° around Y
//        t = identity_mat4<float>();
//        rotate_y_mat4(static_cast<float>(M_PI / 2), t);
//        multiply_mat4(translation_mat4(5.3f, 0.0f, 0.0f), t);
//        multiply_mat4(viewProj, t);
//        xCone.transformMatrix4f(t);
//        xCone.drawTriangles();
//        
//        // Y cone: at (0, 5.3, 0), rotated -90° around X
//        t = identity_mat4<float>();
//        rotate_x_mat4(static_cast<float>(-M_PI / 2), t);
//        multiply_mat4(translation_mat4(0.0f, 5.3f, 0.0f), t);
//        multiply_mat4(viewProj, t);
//        yCone.transformMatrix4f(t);
//        yCone.drawTriangles();
//        
//        // Z cone: at (0, 0, 5.3), rotated 180° around X (flip direction)
//        t = identity_mat4<float>();
//        rotate_x_mat4(static_cast<float>(M_PI), t);
//        multiply_mat4(translation_mat4(0.0f, 0.0f, 5.3f), t);
//        multiply_mat4(viewProj, t);
//        zCone.transformMatrix4f(t);
//        zCone.drawTriangles();
//    }
//};
//
//// ============================================================================
//// Central Point
//// ============================================================================
//class CentralPoint
//{
//public:
//    Dynamit point;
//    
//    void build()
//    {
//        // Small quad at origin (simulates GL_POINTS)
//        float s = 0.08f;
//        std::vector<float> verts = {
//            -s, -s, 0.0f,   s, -s, 0.0f,   s,  s, 0.0f,
//            -s, -s, 0.0f,   s,  s, 0.0f,  -s,  s, 0.0f
//        };
//        
//        point.withVertices3d(verts)
//             .withConstColor(0.0f, 0.0f, 0.0f, 1.0f)
//             .withTransformMatrix4f();
//    }
//    
//    void draw(const mat4<float>& transform)
//    {
//        point.transformMatrix4f(transform);
//        point.drawTriangles();
//    }
//};
//
//// ============================================================================
//// Heart Cone Shape (main display object from lab1)
//// ============================================================================
//class HeartConeShape
//{
//public:
//    Dynamit cone;
//    
//    void build(int nh = 8, int ns = 15)
//    {
//        std::vector<float> verts, norms;
//        HeartConeBuilder::build(verts, norms, nh, ns);
//        
//        // Apply the shear/scale matrix from original draw()
//        // s = { 1, 0, 0, 0,  0, 0.95, 0, 0,  0.5, 0, 1, 0,  0, 0, 0, 1 }
//        mat4<float> shear = {
//            1.0f,  0.0f,  0.0f, 0.0f,
//            0.0f,  0.95f, 0.0f, 0.0f,
//            0.5f,  0.0f,  1.0f, 0.0f,
//            0.0f,  0.0f,  0.0f, 1.0f
//        };
//        
//        std::vector<float> dummyNorms = norms; // applyTransformToRange modifies in place
//        applyTransformToRange(shear, verts, dummyNorms, 0);
//        
//        cone.withVertices3d(verts)
//            .withNormals3d(norms)
//            .withConstColor(1.0f, 0.0f, 1.0f, 1.0f)  // Magenta like original
//            .withConstLightDirection(-0.577f, -0.577f, 0.577f)
//            .withTransformMatrix4f();
//    }
//    
//    void draw(const mat4<float>& transform)
//    {
//        cone.transformMatrix4f(transform);
//        cone.drawTriangles();
//    }
//};
//
//// ============================================================================
//// Lab1 Scene - Combines all elements
//// ============================================================================
//class Lab1Scene
//{
//public:
//    AxisLines axisLines;
//    AxisCones axisCones;
//    CentralPoint centralPoint;
//    HeartConeShape heartCone;
//    
//    void build()
//    {
//        axisLines.build();
//        axisCones.build();
//        centralPoint.build();
//        heartCone.build(8, 15);
//    }
//    
//    void drawHeartConeInstance(const mat4<float>& viewProj,
//                               float translateZ, float scaleXY, float scaleZ,
//                               float rotX = 0.0f, float rotZ = 0.0f)
//    {
//        mat4<float> t = identity_mat4<float>();
//        multiply_mat4(scaleMatrix(scaleXY, scaleXY, scaleZ), t);
//        if (rotX != 0.0f) rotate_x_mat4(rotX, t);
//        if (rotZ != 0.0f) rotate_z_mat4(rotZ, t);
//        multiply_mat4(translation_mat4(0.0f, 0.0f, translateZ), t);
//        multiply_mat4(viewProj, t);
//        heartCone.draw(t);
//    }
//    
//    void draw(const mat4<float>& viewProj)
//    {
//        // Draw axes
//        centralPoint.draw(viewProj);
//        axisLines.draw(viewProj);
//        axisCones.draw(viewProj);
//        
//        // Draw heart cone instances (from lab1GlDisplay)
//        // directDisplay: translate(0,0,0), scale(2,2,3)
//        drawHeartConeInstance(viewProj, 0.0f, 2.0f, 3.0f);
//        
//        // mirrorDisplay: rotate(180,1,0,0), rotate(180,0,0,1)
//        drawHeartConeInstance(viewProj, 0.0f, 2.0f, 3.0f,
//                              static_cast<float>(M_PI), static_cast<float>(M_PI));
//        
//        // mirrorDisplay2: rotate(90,1,0,0)
//        drawHeartConeInstance(viewProj, 0.0f, 2.0f, 3.0f,
//                              static_cast<float>(M_PI / 2), 0.0f);
//        
//        // mirrorDisplay3: rotate(270,1,0,0), rotate(180,0,0,1)
//        drawHeartConeInstance(viewProj, 0.0f, 2.0f, 3.0f,
//                              static_cast<float>(3 * M_PI / 2), static_cast<float>(M_PI));
//    }
//};
//
//// ============================================================================
//// Main Application
//// ============================================================================
//int main()
//{
//    GLFWwindow* window = openglWindowInit(800, 600);
//    if (!window)
//        return -1;
//    
//    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
//    
//    // Build scene
//    Lab1Scene scene;
//    scene.build();
//    
//    // OpenGL state
//    glEnable(GL_DEPTH_TEST);
//    glEnable(GL_CULL_FACE);
//    glClearColor(0.4f, 0.5f, 0.6f, 1.0f);
//    
//    // Animation state (replaces getDFi())
//    float rotationAngle = 1.0f;
//    TimeController tc(static_cast<float>(glfwGetTime()));
//    
//    while (!glfwWindowShouldClose(window))
//    {
//        tc.update(static_cast<float>(glfwGetTime()));
//        rotationAngle += 0.02f;  // Same increment as getDFi()
//        
//        processInputs(window);
//        
//        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//        
//        // Build view-projection (replaces spacePrepare + glOrtho)
//        // Original: glOrtho(-6.2, 6.2, -6.2, 6.2, 2, 12), translate(0,0,-6), rotate(35+dFi, 1,0,0), rotate(-35+dFi, 0,1,0)
//        mat4<float> viewProj = identity_mat4<float>();
//        
//        // Orthographic scale (maps -6.2..6.2 to -1..1)
//        float orthoScale = 1.0f / 6.2f;
//        multiply_mat4(scaleMatrix(orthoScale, orthoScale, orthoScale / 2.0f), viewProj);
//        
//        // View rotation (like spacePrepare)
//        rotate_x_mat4((35.0f + rotationAngle) * static_cast<float>(M_PI) / 180.0f, viewProj);
//        rotate_y_mat4((-35.0f + rotationAngle) * static_cast<float>(M_PI) / 180.0f, viewProj);
//        
//        // Draw everything
//        scene.draw(viewProj);
//        
//        glfwSwapBuffers(window);
//        glfwPollEvents();
//    }
//    
//    glfwTerminate();
//    return 0;
//}