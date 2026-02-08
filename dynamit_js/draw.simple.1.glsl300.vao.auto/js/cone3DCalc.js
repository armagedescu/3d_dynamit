{
"use strict";
let canvas = document.currentScript.parentElement;

let glmain = () =>
{
   let gl = canvas.getContext('webgl2');

   // Test various polar formulas

   // Circle (r = 1)
   let circle = buildConePolar("1", 0, 2 * Math.PI, 20, 10);
   
   // Heart cone (r = theta/PI for 0..PI, then r = (2*PI - theta)/PI for PI..2*PI)
   let heartHalf1 = buildConePolar("theta / PI", 0, Math.PI, 20, 10);
   let heartHalf2 = buildConePolar("(2*PI - theta) / PI", Math.PI, 2 * Math.PI, 200, 300);
   
   // 5-pointed star
   let star = buildConePolar("(1 + 0.5 * cos(5 * theta)) / 1.5", 0, 2 * Math.PI, 200, 300);
   
   // Create Dynamit shapes
   let shapeCircle = new Dynamit(canvas)
      .withVertices3d(circle.verts)
      .withNormals3d(circle.norms)
      .withConstColor([0.0, 1.0, 0.0, 1.0])
      .withConstLightDirection([-1.0, -1.0, 1.0])
      .withConstTranslation([-0.5, 0.5, 0, 0]);

   let shapeHeart1 = new Dynamit(canvas)
      .withVertices3d(heartHalf1.verts)
      .withNormals3d(heartHalf1.norms)
      .withConstColor([1.0, 0.0, 0.5, 1.0])
      .withConstLightDirection([-1.0, -1.0, 1.0])
      .withConstTranslation([0.5, 0.5, 0, 0]);
   
   let shapeHeart2 = new Dynamit(shapeHeart1)
      .withVertices3d(heartHalf2.verts)
      .withNormals3d(heartHalf2.norms);

   let shapeStar = new Dynamit(canvas)
      .withVertices3d(star.verts)
      .withNormals3d(star.norms)
      .withConstColor([1.0, 1.0, 0.0, 1.0])
      .withConstLightDirection([-1.0, -1.0, 1.0])
      .withConstTranslation([0.0, -0.5, 0, 0]);

   // Log for debugging
   console.log("3DCalculator test:");
   console.log("  Circle vertices:", circle.verts.length / 3);
   console.log("  Heart half1 vertices:", heartHalf1.verts.length / 3);
   console.log("  Star vertices:", star.verts.length / 3);

   // Test expression directly
   let expr = new Expression("sin(theta) + cos(theta)");
   expr.bind("theta", Math.PI / 4);
   console.log("  sin(PI/4) + cos(PI/4) =", expr.eval(), "(expected:", Math.SQRT2, ")");

   //shapeCircle.logStrategyShaders("cone3DCalc.js:");

   gl.clearColor(0.3, 0.3, 0.3, 1.0);
   gl.enable(gl.DEPTH_TEST);
   gl.enable(gl.CULL_FACE);
   gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

   //// Uncomment to draw wireframe
   //let polygonModeExt = gl.getExtension('WEBGL_polygon_mode');
   //if (polygonModeExt)
   //   polygonModeExt.polygonModeWEBGL(gl.FRONT_AND_BACK, polygonModeExt.LINE_WEBGL);

   // Draw all shapes
   shapeCircle.drawTriangles();
   shapeHeart1.drawTriangles();  // Also draws shapeHeart2 via chain
   shapeStar.drawTriangles();
};

document.addEventListener('DOMContentLoaded', glmain);
}