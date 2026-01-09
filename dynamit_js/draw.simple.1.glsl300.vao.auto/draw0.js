{
let canvas = document.currentScript.parentElement;

let glmain = () =>
{
   if (true)
   {
      if (false){}
   }
   let shape1 = new Dynamit (canvas).withVertices2d(
                     [ 0.0, -0.5,   -0.8, -0.3,   -0.5, -0.8,
                       0.0, -0.5,    0.8, -0.1,   -0.4,  0.3 ]);
                     //.withConstColor ([0.7, 0.7, 0.7, 1]);
   shape1.logStrategyShaders ("draw0.js shape1 static draw");
   let shape2 = new Dynamit (canvas)
                  .withVertices3d ([ 0.0, 0.0, 0.0,  -1.0, 0.4, 2.0,   -0.5, -0.6,  2.0,
                                    0.0, 0.0, 0.0,   0.4, 0.4, 2.0,   -0.4,  0.5, -0.0  ])
                  .withConstTranslation ([0.5, 0.5, 0.0, 0.0])
                  .withConstColor ([0.0, 1.0, 0.0, 1.0]);
   //shape2.logStrategyShaders ("draw0.js shape2 translate");
   let gl = shape1.gl;
   gl.clearColor (0.5, 0.5, 0.5, 0.9);
   gl.enable (gl.DEPTH_TEST);
   gl.clear  (gl.COLOR_BUFFER_BIT);
   shape1.drawTriangles ();
   shape2.drawTriangles ();
};

document.addEventListener('DOMContentLoaded', glmain);
}