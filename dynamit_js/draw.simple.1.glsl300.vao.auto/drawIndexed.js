{
"use strict";
let canvas = document.currentScript.parentElement;

let glmain = () =>
{
   let gl = canvas.getContext('webgl2');

   // Test 1: Simple indexed quad (4 vertices, 6 indices for 2 triangles)
   let quad = new Dynamit (canvas)
      .withVertices3d([
         -0.8, -0.8, 0.0,  // 0: bottom-left
         -0.2, -0.8, 0.0,  // 1: bottom-right
         -0.2, -0.2, 0.0,  // 2: top-right
         -0.8, -0.2, 0.0   // 3: top-left
      ])
      .withIndices([
         0, 1, 2,  // Triangle 1
         0, 2, 3   // Triangle 2
      ])
      .withConstColor([0.0, 1.0, 0.0, 1.0]);

   // Test 2: Chained indexed triangles
   let triangle1 = new Dynamit (canvas)
      .withVertices3d([
          0.2, -0.8, 0.0,  // 0
          0.8, -0.8, 0.0,  // 1
          0.5, -0.2, 0.0   // 2
      ])
      .withIndices([0, 1, 2])
      .withConstColor([1.0, 0.0, 0.0, 1.0]);

   let triangle2 = new Dynamit (triangle1)
      .withVertices3d([
         -0.3, 0.2, 0.0,  // 0
          0.3, 0.2, 0.0,  // 1
          0.0, 0.8, 0.0   // 2
      ])
      .withIndices([0, 1, 2]);

   // Log shaders for debugging
   //quad.logStrategyShaders("drawIndexed.js - quad:");

   gl.clearColor(0.5, 0.5, 0.5, 0.9);
   gl.enable(gl.DEPTH_TEST);
   gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

   // Draw indexed quad
   quad.drawTrianglesIndexed();

   // Draw chained indexed triangles (triangle1 draws both)
   triangle1.drawTrianglesIndexed();
};

document.addEventListener('DOMContentLoaded', glmain);
}