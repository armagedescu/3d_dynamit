"use strict";
{
let canvas = document.currentScript.parentElement;

function buildCone (nh, ns, type = Float32Array)
{
   let verts    = [];
   let norms    = [];
   let  dr = 0.6;

   for (let i = 0, ix = 0,iy = 1,iz = 2; i < ns; i++, ix += 9,iy += 9,iz += 9)
   {
      [verts[ix],     verts[iy],     verts[iz]]     = [0.0, 0.0, -0.7] ;//<-- tip of the cone, points to us
      [verts[ix + 3], verts[iy + 3], verts[iz + 3]] = [dr * Math.cos(2 * Math.PI * i / ns),       dr * Math.sin(2 * Math.PI * i / ns),      0] ;
      [verts[ix + 6], verts[iy + 6], verts[iz + 6]] = [dr * Math.cos(2 * Math.PI * (i+1) / ns),   dr * Math.sin(2 * Math.PI * (i+1) / ns),  0] ;

      [norms[ix],     norms[iy],     norms[iz]]     = [verts[ix + 3],   verts[iy + 3],  -0.7] ;//<-- tip of the cone, points to us
      [norms[ix + 3], norms[iy + 3], norms[iz + 3]] = [verts[ix + 3],   verts[iy + 3],  -0.7] ;
      [norms[ix + 6], norms[iy + 6], norms[iz + 6]] = [verts[ix + 6],   verts[iy + 6],  -0.7] ;
   }
   return {verts:new type(verts), norms:new type(norms)};
}
let glmain = () =>
{
   let gl, animate;
   let geometry = buildCone (1, 5);
   let shape = new Dynamit (canvas)
      //.withShaderSources (vs, fs)
      .withConstColor ([0.0, 1.0, 0.0, 1.0])
      .withVertices3d (geometry.verts)
      .withNormals3d  (geometry.norms)
      .withLightDirection3f ([1.0, 0.0, 1.0])
      ;
   //shape.logStrategyShaders ("cone1Animate2.js");
   gl = shape.gl;
   gl.clearColor(0.5, 0.5, 0.5, 0.9);
   gl.enable (gl.DEPTH_TEST);
   gl.clear  (gl.COLOR_BUFFER_BIT);
   gl.clear  (gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
   //shape.bind ();
   shape.lightDirection3f ([1.0, 0.0, 1.0]);
   shape.drawTriangles ();

   animate = (time) =>
   {
      gl.clearColor(0.5, 0.5, 0.5, 0.9);
      gl.enable(gl.DEPTH_TEST);
      gl.clear (gl.COLOR_BUFFER_BIT);
      gl.clear (gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
      shape.lightDirection3f ([Math.cos (time * 0.002),  Math.sin (time * 0.002), 1]);
      shape.drawTriangles ();
      window.requestAnimationFrame(animate);
   }
   animate(0);
   return;

};
document.addEventListener('DOMContentLoaded', glmain);
}