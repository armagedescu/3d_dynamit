{
let canvas = document.currentScript.parentElement;

let glmain = () =>
{
   let shape1 = new Dynamit (canvas)
      .withConstColor ([0.0, 1.0, 0.0, 1.0])
      .withVertices3d (
            [  0.0, 0.0, 0.0,   -1.0,  0.4,  2.0,    -0.5, -0.3,  2.0,
               0.0, 0.0, 0.0,    0.6, -0.3,  2.0,     0.4,  0.3, -0.0 ])
      .withNormals3d  (
            [ 1.0,  1.0, -1.0,   1.0, 1.0, -1.0,     1.0,  1.0, -1.0,
              1.0,  0.0, -1.0,   1.0, 0.0, -1.0,     1.0,  0.0, -1.0  ]);
   //shape1.logStrategyShaders ("normals_and_colors_2progs.js shape normals");

   let shape2 = new Dynamit (canvas)
      .withVertices3d (
            [ 0.0, 0.0, 0.0,   -0.3, -0.5,  2.0,     0.5, -0.6,  2.0,
              0.0, 0.0, 0.0,    0.3,  0.4,  2.0,    -0.5,  0.6, -0.0  ])
      //.withColors4d  (
      //      [  0.0, 1.0, 0.0, 1.0,      0.0, 1.0, 0.0, 1.0,       0.0, 1.0, 0.0, 1.0,
      //         1.0, 0.0, 0.0, 1.0,      1.0, 0.0, 0.0, 1.0,       1.0, 0.0, 0.0, 1.0 ]);
      .withColors3d  (
            [  0.0, 1.0, 0.0,      0.0, 1.0, 0.0,      0.0, 1.0, 0.0,
               1.0, 0.0, 0.0,      1.0, 0.0, 0.0,      1.0, 0.0, 0.0]);
   //shape2.logStrategyShaders("normals_and_colors_2progs.js shape2 colors");

   let gl = shape1.gl; //shared gl context
   gl.clearColor (0.5, 0.5, 0.5, 0.9);
   gl.enable (gl.DEPTH_TEST);
   gl.clear  (gl.COLOR_BUFFER_BIT);

   for (shape of [shape1, shape2])
      shape.drawTriangles ();
   
};
document.addEventListener('DOMContentLoaded', glmain);
}