{
"use strict";
let canvas = document.currentScript.parentElement;

let glmain = () => {
   // Simple cylinder
   let geometry = Builder.polar()
      .formula("0.3")
      .domain(0, 2 * Math.PI)
      .sectors(30)
      .slices(5)
      .color([0.2, 0.8, 0.4])
      .buildCylinderIndexedWithColor();
   
   let shape = new Dynamit(canvas)
      .withVertices3d(geometry.verts)
      .withNormals3d(geometry.norms)
      .withColors4d(geometry.colors)
      .withIndices(geometry.indices, Uint32Array)
      .withConstLightDirection([-1, -1, 1])
      .withTranslation4f();  // Uniform translation
   
   shape.logStrategyShaders("Translation Demo:");
   
   let gl = shape.gl;
   gl.enable(gl.DEPTH_TEST);
   gl.clearColor(0.1, 0.1, 0.15, 1.0);
   
   let animate = (time) => {
      let t = time * 0.002;
      
      gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
      
      // Animate translation in circular motion
      let x = 0.5 * Math.cos(t);
      let y = 0.5 * Math.sin(t);
      let z = 0.3 * Math.sin(t * 2);
      
      shape.translate4f(x, y, z, 0);
      shape.drawTrianglesIndexed();
      
      requestAnimationFrame(animate);
   };
   requestAnimationFrame(animate);
};

document.addEventListener('DOMContentLoaded', glmain);
}