"use strict";

class glcontroller
{
   timer  = {  stop : true, switchStop(){this.stop = this.stop ? false : true;}  }; //, 
   camera =
   {
      // rotation matrix
      // x                             %  y                             %  z
      // |   1    0    0|  | 1  0  0|  %  | cos    0  sin|  | 0  0  1|  %  | cos -sin    0|  | 0 -1  0|
      // |   0  cos -sin|  | 0  0 -1|  %  |   0    1    0|  | 0  1  0|  %  | sin  cos    0|  | 1  0  0|
      // |   0  sin  cos|  | 0  1  0|  %  |-sin    0  cos|  |-1  0  0|  %  |   0    0    1|  | 0  0  1|
      ysensibility: 0.05,
      xsensibility: 0.05,
      pos    : [0,                    0,   0],
      up     : [0,                    1,   0],
      target : [0, -Math.sin(Math.PI/8),  -1],
      zoom   : 1,
      decodeDelta(delta){if (delta > 0) return -1; else if (delta < 0) return 1; return 0;},
      set ypos (delta)
      {
         this.pos[1]    +=   this.ysensibility * this.decodeDelta(delta) * Math.PI;
         this.target[1] +=   this.ysensibility * this.decodeDelta(delta) * Math.PI;
      },
      set xpos (delta)
      {
        this.pos[0]    +=  -this.xsensibility * this.decodeDelta(delta) * Math.PI;
        this.target[0] +=  -this.xsensibility * this.decodeDelta(delta) * Math.PI;
      },
      set ytg  (delta) {  this.target[1] +=  -this.ysensibility * this.decodeDelta(delta) * Math.PI; },
      set xtg  (delta) {  this.target[0] +=   this.xsensibility * this.decodeDelta(delta) * Math.PI; },
      get matrix()
      {
         //TODO: Remove m4 dependency !!!
         let cameraMatrix = m4.lookAt(this.pos, this.target, this.up);
         //let cameraMatrix = m4.lookAt(cameraPosition, target, up);
         let viewMatrix = m4.inverse(cameraMatrix);
         //return m4.identity();
         return viewMatrix;
      }
   }; //,
   model =
   {
      timeenable      : [false,    true, false],
      timesensitivity : [0.4,       0.4,   0.4],
      axes            : [0,         0,     0  ], //[Math.PI/8,   0,     0],
      set deltat (delta)
      {
         delta *= Math.PI;
         if (this.timeenable [0]) this.axes [0] += this.timesensitivity [0] * delta; // x rotation disabled
         if (this.timeenable [1]) this.axes [1] += this.timesensitivity [1] * delta; // y rotation enabled
         if (this.timeenable [2]) this.axes [2] += this.timesensitivity [2] * delta; // z rotation disabled
         //console.log("set deltat to: " + delta);
      },
      //TODO: Remove m4 dependency !!!
      get matrix()
      {
         let mtx =  m4.xRotate(m4.identity (), this.axes [0]);
         mtx     =  m4.yRotate(mtx,            this.axes [1]);
         mtx     =  m4.zRotate(mtx,            this.axes [2]);
         //console.log("got matrix: " + JSON.stringify(mtx));
         //return m4.identity();
         return mtx;

      }
   }
}

function cloneMouseEvent (event)
{
   return { type :event.type, shiftKey : event.shiftKey, ctrlKey : event.ctrlKey, movementY : event.movementY, movementX : event.movementX,
   deltaX : event.deltaX, deltaY : event.deltaY, deltaZ : event.deltaZ, deltaMode : event.deltaMode};
}
