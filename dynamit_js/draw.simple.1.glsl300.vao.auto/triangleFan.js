{
let canvas = document.currentScript.parentElement;

let glmain = () =>
{
   //TODO: Effects research area
   let gl;
   //full triangle fan, only ns sectors, no nh slices
   //with wrong normals, achive some glowing effect
   let geometry = function (ns = 16, dr = 0.6, type = Float32Array) 
      {
         let verts    = [0.8, 0.8, 0];
         let norms    = [0.0, 0.0, 1.0];
         for (let i = 0, ix = 3,iy = 4,iz = 5; i <= ns; i++, ix += 3,iy += 3,iz += 3)
         {
            [verts [ix], verts [iy], verts [iz]] = [dr * Math.cos(2 * Math.PI * i / ns),   dr * Math.sin(2 * Math.PI * i / ns),   -1];
            [norms [ix], norms [iy], norms [iz]] = [0, -1,  1];
         }
         return {verts:new type(verts), norms:new type(norms)};
      } ();
    let shape = new Dynamit (canvas)
            .withVertices3d  (geometry.verts)
            .withNormals3d   (geometry.norms)
            .withConstColor  ([ 0.0,  1.0,  0.0,  0.1])
            .withConstLightDirection ([0.0, 1.2, 0.0], false);
   //shape.logStrategyShaders ("triangleFan.js");
   gl = shape.gl;
   gl.clearColor(0.5, 0.5, 0.5, 0.9);
   gl.enable(gl.DEPTH_TEST);
   gl.clear (gl.COLOR_BUFFER_BIT);
   shape.drawTriangleFan ();
   return;
};
document.addEventListener('DOMContentLoaded', glmain);
}