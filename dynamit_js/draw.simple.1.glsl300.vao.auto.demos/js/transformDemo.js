{
"use strict";
let canvas = document.currentScript.parentElement;

let glmain = () => {
   // Build a star shape using polar formula
   let geometry = Builder.polar()
      .formula("1 + 0.5 * cos(5 * theta)")
      //.formula("1")
      .domain(0, 2 * Math.PI)
      .sectors(60)
      .slices(5)
      .color([1.0, 0.5, 0.2])
      .buildConeIndexedWithColor();
   
   let shape = new Dynamit(canvas)
      .withVertices3d(geometry.verts)
      .withNormals3d(geometry.norms)
      .withColors4d(geometry.colors)
      .withIndices(geometry.indices, Uint32Array)
      .withConstLightDirection([-1, -1, 1])
      .withTransformMatrix4f();
   
   //shape.logStrategyShaders("Transform Matrix Demo:");
   
   let gl = shape.gl;
   gl.enable(gl.DEPTH_TEST);
   gl.clearColor(0.1, 0.1, 0.2, 1.0);
   
   let animate = (time) => {
      let t = time * 0.001;
      
      // Rotation around Y axis
      let cosY = Math.cos(t), sinY = Math.sin(t);
      // Rotation around X axis  
      let cosX = Math.cos(t * 0.3), sinX = Math.sin(t * 0.3);
      
      // Combined rotation matrix (Y * X)
      let matrix = [
         cosY,        sinX * sinY,  cosX * sinY, 0,
         0,           cosX,        -sinX,        0,
        -sinY,        sinX * cosY,  cosX * cosY, 0,
         0,           0,            0,           1
      ];
      
      gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
      
      shape.transformMatrix4f(matrix);
      shape.drawTrianglesIndexed();
      
      requestAnimationFrame(animate);
   };
   requestAnimationFrame(animate);
};

document.addEventListener('DOMContentLoaded', glmain);
}