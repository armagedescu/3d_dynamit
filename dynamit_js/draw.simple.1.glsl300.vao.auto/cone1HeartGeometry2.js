"use strict";
{
let canvas = document.currentScript.parentElement;

function buildConeHeart (nh, ns, type = Float32Array)
{
   if(ns & 1) ns++;
   let verts    = [];
   let norms    = [];
   let PI_2 = 2 * Math.PI;      //2 * Pi
   let FI_S = 2 * Math.PI / ns; //angular size of one sector
   let R_S  = 2 / ns;           //Radius = FI normalized (FI / PI)
   let D_H  = 1 / nh;

   for (let i = 0,   ix = 0, iy = 1, iz = 2;    i < ns; i++,     ix += 9,iy += 9,iz += 9)
   {
      let ps, cr;
      if (i < ns / 2)
      {
         let fi1 = i   * FI_S;
         let fi2 = fi1 + FI_S; //next fi
         
         let rc1 = i   * R_S; //<-- radius coeficient
         let rc2 = rc1 + R_S; //<-- next radius coeficient

         let r1 = rc1 / nh; //<-- radius
         let r2 = rc2 / nh; //<-- next radius

         ps = [[0.0,                  0.0,                  -(1.0)], //<--points in direction of us
               [r1 * Math.cos(fi1),   r1 * Math.sin(fi1),   -(1 - D_H)],
               [r2 * Math.cos(fi2),   r2 * Math.sin(fi2),   -(1 - D_H)]];

         cr = [[0,   0,   0],
               [(Math.sin(fi1) + fi1 * Math.cos(fi1)),  -(Math.cos(fi1) - fi1 * Math.sin(fi1)),   -(1 + fi1)],
               [(Math.sin(fi2) + fi2 * Math.cos(fi2)),  -(Math.cos(fi2) - fi2 * Math.sin(fi2)),   -(1 + fi2)]];
         //break;
      } else
      {
         let i2 = i - ns / 2;
         let fi1 = -i2   * FI_S;
         let fi2 =  fi1  - FI_S;

         let rc1 = i2  * R_S; //<-- radius coeficients
         let rc2 = rc1 + R_S; //<-- radius coeficients

         let r1 = rc1 / nh; //<-- radius
         let r2 = rc2 / nh; //<-- radius

         ps =     [[0.0,                  0.0,                  -(1.0)], //<--points to us
                   [r2 * Math.cos(fi2),   r2 * Math.sin(fi2),   -(1 - D_H)],
                   [r1 * Math.cos(fi1),   r1 * Math.sin(fi1),   -(1 - D_H)]];

         cr = [[ 0,  0,  0],
               [-(Math.sin(fi2) + fi2 * Math.cos(fi2)), (Math.cos(fi2) - fi2 * Math.sin(fi2)), -(1 + fi2)],
               [-(Math.sin(fi1) + fi1 * Math.cos(fi1)), (Math.cos(fi1) - fi1 * Math.sin(fi1)), -(1 + fi1)]];
      }
      [verts[ix],     verts[iy],     verts[iz]]     = ps[0];
      [verts[ix + 3], verts[iy + 3], verts[iz + 3]] = ps[1];
      [verts[ix + 6], verts[iy + 6], verts[iz + 6]] = ps[2];

      [norms[ix],     norms[iy],     norms[iz]]     = cr[0];
      [norms[ix + 3], norms[iy + 3], norms[iz + 3]] = cr[1];
      [norms[ix + 6], norms[iy + 6], norms[iz + 6]] = cr[2];
   }

   //1 triangle = 3 points * 3 coordinates
   for (let h = 1, ix = ns*9, iy = ns*9 + 1, iz = ns*9 + 2;    h < nh; h++)
   {
      let h1n = h/nh, h2n = (h+1)/nh; //<-- start and stop height
      for (let i = 0;    i < ns; i++,       ix += 9,iy += 9,iz += 9)
      {
         let ps, cr;
         if (i < ns / 2)
         {
            let fi1 = i   * FI_S;
            let fi2 = fi1 + FI_S;

            let rc1 = i   * R_S; //<-- radius coeficients
            let rc2 = rc1 + R_S; //<-- radius coeficients
            //if (rc1 > 1) rc1 = 2 - rc1; //<-- decreases when passes Pi
            //if (rc2 > 1) rc2 = 2 - rc2; //<-- decreases when passes Pi
            let r11 = rc1 * h1n; //<-- radius
            let r12 = rc2 * h1n; //<-- radius
            let r21 = rc1 * h2n; //<-- radius
            let r22 = rc2 * h2n; //<-- radius

            ps = [[r11 * Math.cos(fi1),  r11 * Math.sin(fi1),  -(1 - h1n)],  // <-- points [1] []    [1]   [4]
                  [r12 * Math.cos(fi2),  r12 * Math.sin(fi2),  -(1 - h1n)],  // <-- points [ ] []    [ ]   [6]
                  [r21 * Math.cos(fi1),  r21 * Math.sin(fi1),  -(1 - h2n)],  // <-- points [3] []    [2]   [ ]
                  [r22 * Math.cos(fi2),  r22 * Math.sin(fi2),  -(1 - h2n)]]; // <-- points [2] []    [3]   [5]
         
            cr = [[(Math.sin(fi1) + fi1 * Math.cos(fi1)),  (-(Math.cos(fi1) - fi1 * Math.sin(fi1))),  -(1 + fi1)],
                  [(Math.sin(fi2) + fi2 * Math.cos(fi2)),  (-(Math.cos(fi2) - fi2 * Math.sin(fi2))),  -(1 + fi2)],
                  [(Math.sin(fi1) + fi1 * Math.cos(fi1)),  (-(Math.cos(fi1) - fi1 * Math.sin(fi1))),  -(1 + fi1)],
                  [(Math.sin(fi2) + fi2 * Math.cos(fi2)),  (-(Math.cos(fi2) - fi2 * Math.sin(fi2))),  -(1 + fi2)]
                  ];

         } else
         {
            let i2 = i - ns / 2;
            let fi1 = -i2 * FI_S;
            let fi2 = fi1 - FI_S;

            let rc1 = i2  * R_S; //<-- radius coeficients
            let rc2 = rc1 + R_S; //<-- radius coeficients
            //if (rc1 > 1) rc1 = 2 - rc1; //<-- decreases when passes Pi
            //if (rc2 > 1) rc2 = 2 - rc2; //<-- decreases when passes Pi
            let r11 = rc1 * h1n; //<-- radius
            let r12 = rc2 * h1n; //<-- radius
            let r21 = rc1 * h2n; //<-- radius
            let r22 = rc2 * h2n; //<-- radius

            ps = [[r11 * Math.cos(fi1),  r11 * Math.sin(fi1),  -(1 - h1n)],  // <-- points [1]   [4]
                  [r12 * Math.cos(fi2),  r12 * Math.sin(fi2),  -(1 - h1n)],  // <-- points [ ]   [6]
                  [r21 * Math.cos(fi1),  r21 * Math.sin(fi1),  -(1 - h2n)],  // <-- points [2]   [ ]
                  [r22 * Math.cos(fi2),  r22 * Math.sin(fi2),  -(1 - h2n)]]; // <-- points [3]   [5]

            cr = [[-(Math.sin(fi1) + fi1 * Math.cos(fi1)),  (Math.cos(fi1) - fi1 * Math.sin(fi1)),  -(1 + fi1)],
                  [-(Math.sin(fi2) + fi2 * Math.cos(fi2)),  (Math.cos(fi2) - fi2 * Math.sin(fi2)),  -(1 + fi2)],
                  [-(Math.sin(fi1) + fi1 * Math.cos(fi1)),  (Math.cos(fi1) - fi1 * Math.sin(fi1)),  -(1 + fi1)],
                  [-(Math.sin(fi2) + fi2 * Math.cos(fi2)),  (Math.cos(fi2) - fi2 * Math.sin(fi2)),  -(1 + fi2)]
                  ];

         }

         [verts[ix],     verts[iy],     verts[iz]]     = ps[0];
         [verts[ix + 3], verts[iy + 3], verts[iz + 3]] = ps[2];
         [verts[ix + 6], verts[iy + 6], verts[iz + 6]] = ps[3];

         [norms[ix],     norms[iy],     norms[iz]]     = cr[0];
         [norms[ix + 3], norms[iy + 3], norms[iz + 3]] = cr[2];
         [norms[ix + 6], norms[iy + 6], norms[iz + 6]] = cr[3];

         ix += 9;iy += 9;iz += 9;

         [verts[ix],     verts[iy],     verts[iz]]     = ps[0];
         [verts[ix + 3], verts[iy + 3], verts[iz + 3]] = ps[1];
         [verts[ix + 6], verts[iy + 6], verts[iz + 6]] = ps[3];

         [norms[ix],     norms[iy],     norms[iz]]     = cr[0];
         [norms[ix + 3], norms[iy + 3], norms[iz + 3]] = cr[1];
         [norms[ix + 6], norms[iy + 6], norms[iz + 6]] = cr[3];

      }
   }
   return {verts:new type(verts), norms:new type(norms)};
}

let glmain = () =>
{
   let gl;
   let geometry = buildConeHeart (2, 20);
   let shape = new Dynamit (canvas)
      .withConstColor ([0.0, 1.0, 0.0, 1.0])
      .withVertices3d (geometry.verts)
      .withNormals3d  (geometry.norms)
      .withConstLightDireciton ([-1.0,  -1.0,  1.0])
      ;
   //shape.logStrategyShaders ("cone1HeartGeometry2.js");
   gl = shape.gl;
   gl.clearColor(0.5, 0.5, 0.5, 0.9);
   gl.enable (gl.DEPTH_TEST);
   gl.clear  (gl.COLOR_BUFFER_BIT);
   gl.clear  (gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

   shape.drawTriangles ();
   return;

};
document.addEventListener('DOMContentLoaded', glmain);
}