{
"use strict";
let canvas = document.currentScript.parentElement;

let glmain = () =>
{
   let shape = new Dynamit (canvas)
               .withVertices3d ()
               .withTransnation4f ();

   let triangle1 = new Dynamit (canvas)
               .withVertices3d ([ 0.0, 0.0, 0.0,   -0.3, -0.5,  2.0,     0.5, -0.6,  2.0 ])
               .withTransnation4f ();
   //triangle1.logStrategyShaders ("animate.js:");
   let triangle2 = new Dynamit (triangle1)
               .withVertices3d ([ 0.0, 0.0, 0.0,    0.3,  0.4,  2.0,    -0.5,  0.6, -0.0  ])
               .withTransnation4f ();
   let gl = shape.gl;

   let animate = (time) =>
   {
      shape.bind();

      gl.clearColor(0.5, 0.5, 0.5, 0.9);
      gl.enable(gl.DEPTH_TEST);
      gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
   
      //same shabe for two triangles, update vertices buffer
      shape.translate4f (0.5 *  Math.cos(time * 0.005), 0.5 *  Math.sin(time * 0.005), 0.5 *  Math.sin(time * 0.005), 0.0);
      shape.updateVertices ([ 0.0, 0.0, 0.0,   -1.0,  0.4,  2.0,    -0.5, -0.3,  2.0 ]);
      shape.drawTriangles();
      shape.updateVertices ([ 0.0, 0.0, 0.0,    0.6, -0.3,  2.0,     0.4,  0.3, -0.0 ]);
      shape.drawTriangles();

      //two shapes, one triangle per shape no update vertices buffer
      triangle1.bind(); //these use shared program
      triangle1.translate4f (0.5 *  Math.cos(time * 0.005), 0.5 *  Math.sin(time * 0.005), 0.5 *  Math.sin(time * 0.005), 0.0); //applies to whole program

      triangle1.drawTriangles ();
      //for (triangl of [triangle1, triangle2])
      //   triangl.drawTriangles ();
      window.requestAnimationFrame(animate);
   }
   animate(0);
};
document.addEventListener('DOMContentLoaded', glmain);
}