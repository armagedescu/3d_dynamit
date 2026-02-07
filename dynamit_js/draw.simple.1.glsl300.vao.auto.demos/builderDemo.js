{
"use strict";
let canvas = document.currentScript.parentElement;

let glmain = () => {
   // Polar builder - Rose curve cone
   let rose = Builder.polar()
      .formula("cos(5 * theta)")
      .domain(0, 2 * Math.PI)
      .sectors(120)
      .slices(4)
      .color([1.0, 0.3, 0.5])
      .buildConeIndexedWithColor();
   
   // Cartesian builder - Parabola extruded as cylinder
   let parabola = Builder.cartesian()
      .formula("x * x")
      .domain(-1, 1)
      .sectors(40)
      .slices(10)
      .color([0.3, 0.7, 1.0])
      .buildCylinderIndexedWithColor();
   
   let shape1 = new Dynamit(canvas)
      .withVertices3d(rose.verts)
      .withNormals3d(rose.norms)
      .withColors4d(rose.colors)
      .withIndices(rose.indices, Uint32Array)
      .withConstLightDirection([-1, -1, 1])
      .withTransformMatrix4f();
   
   // Chain second shape to share the same program
   let shape2 = new Dynamit(shape1)
      .withVertices3d(parabola.verts)
      .withNormals3d(parabola.norms)
      .withColors4d(parabola.colors)
      .withIndices(parabola.indices, Uint32Array);
   
   shape1.logStrategyShaders("Builder Demo:");
   
   let gl = shape1.gl;
   gl.enable(gl.DEPTH_TEST);
   gl.clearColor(0.15, 0.15, 0.2, 1.0);
   
   let animate = (time) => {
      let t = time * 0.001;
      let cos = Math.cos(t), sin = Math.sin(t);
      
      gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
      
      // Draw rose on left side
      let matrixLeft = [
         0.4 * cos, 0, 0.4 * sin, 0,
         0, 0.4, 0, 0,
        -0.4 * sin, 0, 0.4 * cos, 0,
        -0.5, 0, 0, 1
      ];
      shape1.transformMatrix4f(matrixLeft);
      shape1.drawTrianglesIndexed();
      
      // Draw parabola on right side
      let matrixRight = [
         0.3 * cos, 0.3 * sin, 0, 0,
        -0.3 * sin, 0.3 * cos, 0, 0,
         0, 0, 0.3, 0,
         0.5, 0, 0, 1
      ];
      shape2.transformMatrix4f(matrixRight);
      shape2.drawTrianglesIndexed();
      
      requestAnimationFrame(animate);
   };
   requestAnimationFrame(animate);
};

document.addEventListener('DOMContentLoaded', glmain);
}