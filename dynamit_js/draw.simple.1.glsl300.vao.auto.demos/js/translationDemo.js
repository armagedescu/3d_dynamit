{
"use strict";
let canvas = document.currentScript.parentElement;

let glmain = () => {
   // Simple cylinder with 45 degree rotation around X
   let geometry = Builder.polar()
      //.doubleCoated()
      .formula("0.8")
      .domain(0, 2 * Math.PI)
      .sectors(30)
      .slices(2)  // Now correctly builds 2 rings
      .color([0.8, 0.8, 0.4])
      //.rotateX(Math.PI / 4)  // 45 degrees around X axis
      //.buildCylinderIndexedWithColor()
      .buildConeIndexedWithColor();
   
   let shape = new Dynamit(canvas)
      .withVertices3d(geometry.verts)
      .withNormals3d(geometry.norms)
      .withColors4d(geometry.colors)
      .withIndices(geometry.indices, Uint32Array)
      .withConstLightDirection([-1, -1, 1])
      .withTranslation4f();
   
   shape.logStrategyShaders("Translation Demo:");
   
   let gl = shape.gl;
   gl.enable(gl.DEPTH_TEST);
   gl.enable(gl.CULL_FACE);
   gl.clearColor(0.1, 0.1, 0.15, 1.0);

   //let polygonMode = gl.getExtension("WEBGL_polygon_mode"); //show lines: gl.FRONT_AND_BACK mode.LINE_WEBGL mode.FILL_WEBGL
   //if (true) polygonMode.polygonModeWEBGL(gl.FRONT_AND_BACK, polygonMode.LINE_WEBGL);
   //else polygonMode.polygonModeWEBGL(gl.FRONT_AND_BACK, polygonMode.FILL_WEBGL);

   
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