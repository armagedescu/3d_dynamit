"use strict";
{
let canvas = document.currentScript.parentElement;

function buildConeHeart (nh = 2, ns = 40, type = Float32Array)
{
   if (ns & 1) ns++;
   let verts    = [];
   let norms    = [];

   let dr = 1.0;
   
   for (let i = 0,   ix = 0, iy = 1, iz = 2;    i < ns; i++,     ix += 9,iy += 9,iz += 9)
   {
      dr      =  2 *  i      * (1/ns); // <-- increase from 0 to 2 (PI)
      let drd =  2 * (i + 1) * (1/ns); // <-- increase from 0 to 2 (PI)
      if (dr  > 1) dr  = 2 - dr;  //<-- decrease when greater than PI
      if (drd > 1) drd = 2 - drd; //<-- decrease when greater than PI
      let ps = [[0.0,                                             0.0,                                           -(1.0)], //<--points in direction of us
                [(dr /nh) * Math.cos(2 * Math.PI *   i    / ns ), (dr /nh) * Math.sin(2 * Math.PI *  i    / ns), -(1 - 1/nh)],
                [(drd/nh) * Math.cos(2 * Math.PI *  (i+1) / ns ), (drd/nh) * Math.sin(2 * Math.PI * (i+1) / ns), -(1 - 1/nh)]];
      let cr = cross3pl (ps[0], ps[1], ps[2]);
      [verts[ix],     verts[iy],     verts[iz]]     = ps[0];
      [verts[ix + 3], verts[iy + 3], verts[iz + 3]] = ps[1];
      [verts[ix + 6], verts[iy + 6], verts[iz + 6]] = ps[2];

      [norms[ix],     norms[iy],     norms[iz]]     = cr;
      [norms[ix + 3], norms[iy + 3], norms[iz + 3]] = cr;
      [norms[ix + 6], norms[iy + 6], norms[iz + 6]] = cr;

   }

   //1 triangle = 3 points * 3 coordinates
   for (let h = 1, ix = ns*9, iy = ns*9 + 1, iz = ns*9 + 2;    h < nh; h++)
   {
      for (let i = 0;    i < ns; i++,       ix += 9,iy += 9,iz += 9)
      {
         dr      =  2 *  i      * (1/ns);
         let drd =  2 * (i + 1) * (1/ns);
         if (dr  > 1) dr  = 2 - dr;  //<-- decrease when greater than PI
         if (drd > 1) drd = 2 - drd; //<-- decrease when greater than PI

         let ps = [  [( h   *dr /nh) * Math.cos(2 * Math.PI *  i   /ns), ( h   *dr /nh) * Math.sin(2 * Math.PI *  i    / ns), -(1 -  h   *1/nh)],  // 1  4
                     [( h   *drd/nh) * Math.cos(2 * Math.PI * (i+1)/ns), ( h   *drd/nh) * Math.sin(2 * Math.PI * (i+1) / ns), -(1 -  h   *1/nh)],  //    6
                     [((h+1)*dr /nh) * Math.cos(2 * Math.PI *  i   /ns), ((h+1)*dr /nh) * Math.sin(2 * Math.PI *  i    / ns), -(1 - (h+1)*1/nh)],  // 2
                     [((h+1)*drd/nh) * Math.cos(2 * Math.PI * (i+1)/ns), ((h+1)*drd/nh) * Math.sin(2 * Math.PI * (i+1) / ns), -(1 - (h+1)*1/nh)]]; // 3  5

         let cr = cross3pl (ps[0], ps[2], ps[3]);

         [verts[ix],     verts[iy],     verts[iz]]     = ps[0];
         [verts[ix + 3], verts[iy + 3], verts[iz + 3]] = ps[2];
         [verts[ix + 6], verts[iy + 6], verts[iz + 6]] = ps[3];

         [norms[ix],     norms[iy],     norms[iz]]     = cr;
         [norms[ix + 3], norms[iy + 3], norms[iz + 3]] = cr;
         [norms[ix + 6], norms[iy + 6], norms[iz + 6]] = cr;

         ix += 9;iy += 9;iz += 9;
         cr = cross3pl (ps[0], ps[3], ps[1]);

         [verts[ix],     verts[iy],     verts[iz]]     = ps[0];
         [verts[ix + 3], verts[iy + 3], verts[iz + 3]] = ps[1];
         [verts[ix + 6], verts[iy + 6], verts[iz + 6]] = ps[3];

         [norms[ix],     norms[iy],     norms[iz]]     = cr;
         [norms[ix + 3], norms[iy + 3], norms[iz + 3]] = cr;
         [norms[ix + 6], norms[iy + 6], norms[iz + 6]] = cr;

      }
   }
   return {verts:new type(verts), norms:new type(norms)};
}
let glmain = () =>
{
   let gl;
   let geometry = buildConeHeart (3, 20);
   let shape = new Dynamit (canvas)
      .withConstColor ([0.0, 1.0, 0.0, 1.0])
      .withVertices3d (geometry.verts)
      .withNormals3d  (geometry.norms)
      .withConstLightDireciton ([-1.0,  -1.0,  1.0])
      ;
   //shape.logStrategyShaders ("cone1HeartGeometry1.js");
   gl = shape.gl;
   gl.clearColor(0.5, 0.5, 0.5, 0.9);
   gl.enable (gl.DEPTH_TEST);
   gl.enable(gl.CULL_FACE);
   gl.clear  (gl.COLOR_BUFFER_BIT);
   gl.clear  (gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

   shape.drawTriangles ();
   return;
};
document.addEventListener('DOMContentLoaded', glmain);
}