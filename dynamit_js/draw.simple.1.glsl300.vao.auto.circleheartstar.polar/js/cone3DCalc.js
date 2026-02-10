{
"use strict";
let canvas = document.currentScript.parentElement;

let glmain = () =>
{
   let gl = canvas.getContext('webgl2');

   //verious formula based shapes (meshes) built with polar builder
   let geometry = {
      circle: Builder.polar().buildConeIndexed(),
      heart: [
         Builder.polar().formula ("theta/PI").domain(0, Math.PI).buildConeIndexed(),
         Builder.polar().formula ("(2*PI - theta)/PI").domain(Math.PI, 2 * Math.PI).buildConeIndexed()
      ],
      star5p: Builder.polar().formula ("(1 + 0.5 * cos(5 * theta)) / 1.5").sectors_slices (200, 300).buildConeIndexed()
   };

   // Create Dynamit shapes
   let shapeCircle = new Dynamit(canvas)
      .withVertices3d(geometry.circle.verts)
      .withNormals3d(geometry.circle.norms)
      .withIndices(geometry.circle.indices, Uint32Array)
      .withConstColor([0.0, 1.0, 0.0, 1.0])
      .withConstLightDirection([-1.0, -1.0, 1.0])
      .withConstTranslation([-0.5, 0.5, 0, 0])
      ;

   //let heartHalf0 =  geometry.heart.half[0];
   let shapeHeart = new Dynamit(canvas)
      .withVertices3d(geometry.heart[0].verts)
      .withNormals3d(geometry.heart[0].norms)
      .withIndices(geometry.heart[0].indices, Uint32Array)
      .withConstColor([1.0, 0.0, 0.5, 1.0])
      .withConstLightDirection([-1.0, -1.0, 1.0])
      .withConstTranslation([0.5, 0.5, 0, 0])
      ;
   //
   let shapeHeart2 = new Dynamit(shapeHeart)
      .withVertices3d(geometry.heart[1].verts)
      .withNormals3d(geometry.heart[1].norms)
      .withIndices(geometry.heart[1].indices, Uint32Array)
      //,with all other properties inherited from shapeHeart
      ;

  ////
  let shapeStar = new Dynamit(canvas)
     .withVertices3d(geometry.star5p.verts)
     .withNormals3d(geometry.star5p.norms)
     .withIndices(geometry.star5p.indices, Uint32Array)
     .withConstColor([1.0, 1.0, 0.0, 1.0])
     .withConstLightDirection([-1.0, -1.0, 1.0])
     .withConstTranslation([0.0, -0.5, 0, 0])
     ;
   //
   //// Log for debugging
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
   shapeCircle.drawTrianglesIndexed();
   shapeHeart.drawTrianglesIndexed();
   //shapeHeart1.drawTriangles();  // Also draws shapeHeart2 via chain
   shapeStar.drawTrianglesIndexed();
};

document.addEventListener('DOMContentLoaded', glmain);
}