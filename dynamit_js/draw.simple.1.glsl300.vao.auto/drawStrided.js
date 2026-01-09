{
"use strict";
let canvas = document.currentScript.parentElement;

let glmain = () =>
{
   let gl = canvas.getContext('webgl2');

   // Test 1: Strided vertices + colors (interleaved)
   // Format: x, y, z, r, g, b per vertex
   let triangle1 = new Dynamit(canvas)
      .withStride([
         // x,    y,    z,    r,   g,   b
          0.0,  0.5,  0.0,  1.0, 0.0, 0.0,  // top - red
         -0.5, -0.5,  0.0,  0.0, 1.0, 0.0,  // bottom-left - green
          0.5, -0.5,  0.0,  0.0, 0.0, 1.0   // bottom-right - blue
      ], 6 * 4)  // stride = 6 floats * 4 bytes
      .withStrideVertices(3)
      .withStrideColors(3);

   // Test 2: Chained strided with different data
   let triangle2 = new Dynamit(triangle1)
      .withStride([
         // x,    y,    z,    r,   g,   b
         -0.8,  0.8,  0.0,  1.0, 1.0, 0.0,  // yellow
         -0.8,  0.3,  0.0,  0.0, 1.0, 1.0,  // cyan
         -0.3,  0.3,  0.0,  1.0, 0.0, 1.0   // magenta
      ], 6 * 4);

   // Test 3: Strided with normals + lighting
   let litTriangle = new Dynamit(canvas)
      .withStride([
         // x,    y,    z,   nx,    ny,   nz
          0.5,  0.8,  0.0,  0.0,   1.0,  -0.5,
          0.3,  0.3,  0.0,  0.0,   0.0,  -1.0,
          0.8,  0.3,  0.0,  0.0,  -0.5,  -1.0
      ], 6 * 4)
      .withStrideVertices(3)
      .withStrideNormals(3)
      .withConstColor([0.0, 1.0, 0.0, 1.0])
      .withConstLightDirection([-1.0, -1.0, 1.0]);

   // Log shaders for debugging
   //triangle1.logStrategyShaders("drawStrided.js - triangle1 (vertex+color):");
   //litTriangle.logStrategyShaders("drawStrided.js - litTriangle (vertex+normal):");

   gl.clearColor(0.3, 0.3, 0.3, 1.0);
   gl.enable(gl.DEPTH_TEST);
   gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

   // Draw strided triangles (triangle1 draws both via chain)
   triangle1.drawTriangles();

   // Draw lit strided triangle
   litTriangle.drawTriangles();
};

document.addEventListener('DOMContentLoaded', glmain);
}