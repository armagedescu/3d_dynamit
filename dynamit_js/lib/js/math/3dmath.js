"use strict";

const TypedArray = Object.getPrototypeOf(Int8Array);
// Add to Array function set, similar to TypedArray
if (!Array.prototype.set)
{
   const TypedArray = Object.getPrototypeOf(Int8Array);
   //Decorate Array with TypedArray like set and advanced iterable detection
   //Note, string is iterable, but not array type
   Array.prototype.set = function (a)
   {
      let i = 0, len = a.length;
      if (len > this.length) throw new Error("RangeError: offset is out of bounds");
      for (; i < len; i++) this[i] = a[i];
   }
   Array.prototype.isArrayType = function (obj)
   {
      if (!obj) return false;
      if (obj instanceof TypedArray || Array.isArray(obj) ) return true;
      //consider return false here
      if (typeof obj == "string") return false;
      return typeof obj[Symbol.iterator] === 'function';
   }
   Array.prototype.isIterable  = function (obj)
   {
      if (!obj) return false;
      return typeof obj[Symbol.iterator] === 'function'; //this includes string
   }
}
class NumberUtil {
   //make string from float
   //if number has no decimals a .0 will be added
   static floatize (a, tofixed = 1) {
      return a > Math.trunc(a) ? Number.parseFloat(a) : Number.parseFloat(a).toFixed(tofixed);
   }
   static floatStringify (a, tofixed = 1) {
      return NumberUtil.floatize (a, tofixed).toString();
   }
}
class ArrayUtil
{
   static isIterable (obj) {
      if (!obj) return false;
      return typeof obj[Symbol.iterator] === 'function';
   }
   static isArrayType (obj) {
      if (!obj) return false;
      if (typeof obj == "string") return false;
      if (obj instanceof TypedArray || Array.isArray(obj) ) return true;
      return typeof obj[Symbol.iterator] === 'function';
   }
   static validateType (obj) {
      if(!ArrayUtil.isArrayType (obj)) throw new Error ("Array type must be provided");
      return obj;
   }
   static toArray (a) {
      if (!a) return [];
      if (ArrayUtil.isArrayType(a)) return a;
      return [a];
   }
   static buildFloatStrings (floats) {
      return ArrayUtil.toArray (floats).map (a => NumberUtil.floatStringify (a));
   }
   static toArrayType (value, arrayType) {
      if (value instanceof arrayType) return value;
      return new arrayType(value);
   }
   static selectArray (a, ... args) {
      if (this.isArrayType (a)) return a;
      return [a, ... args];
   }
   //some test code
   //console.assert (!ArrayUtil.isArrayType ("hello world"), "string is is not array");
   //console.assert (!ArrayUtil.isArrayType (1), "int is is not array");
   //console.assert ( ArrayUtil.isArrayType ([1]));
   //console.assert ( ArrayUtil.isArrayType (["1", "int is is not array"]), "array of strings is array");
   //console.assert ( ArrayUtil.isArrayType (new Float32Array ([1.])));
}
////Disable getContext entirely
//HTMLCanvasElement.prototype.getContext = undefined;
////Disable WebGPU
//const fun = HTMLCanvasElement.prototype.getContext;
//HTMLCanvasElement.prototype.privateContext = fun; //HTMLCanvasElement.prototype.getContext;
//HTMLCanvasElement.prototype.getContext = function (a)
//{
//   if (a == "webgpu") return undefined;
//   return this.privateContext(a);
//}
//// or Disable WebGPU
//if (!HTMLCanvasElement.prototype.getContext2)
//{
//   HTMLCanvasElement.prototype.getContext2 = HTMLCanvasElement.prototype.getContext;
//   HTMLCanvasElement.prototype.getContext = function (a)
//   {
//      if (a == "webgpu") return undefined;
//      return this.getContext2(a);
//   }
//}


function wrap (x)
{
   if (isFinite(x)) return x;
   if (x == Number.POSITIVE_INFINITY) return Number.MAX_VALUE;
   if (x == Number.NEGATIVE_INFINITY) return Number.MIN_VALUE;
   return 0.0;
}
function cross3p  (p1, p2, p3) //3D cross between three 3D points Right Handed
{
   let v1 = deltav (p1, p2);
   let v2 = deltav (p1, p3);
   return [(v1[1] * v2[2] - v1[2] * v2[1]),   (v1[2] * v2[0] - v1[0] * v2[2]),   (v1[0] * v2[1] - v1[1] * v2[0])];
}
function cross3pl  (p1, p2, p3) //3D cross between three 3D points Left Handed
{
   let v1 = deltav (p1, p2);
   let v2 = deltav (p1, p3);
   return [(v1[2] * v2[1] - v1[1] * v2[2]),   (v1[0] * v2[2] - v1[2] * v2[0]),  (v1[1] * v2[0] - v1[0] * v2[1])];
}
function cross3pn (p1, p2, p3) //3D cross between three 3D points, normalized Right Handed
{
   return normv (cross3p (p1, p2, p3));
}
function cross3pnl (p1, p2, p3) //3D cross between three 3D points, normalized, Left Handed
{
   return normv (cross3pl (p1, p2, p3));
}
function cross3 (v1, v2) //3D cross between two 3D vectors Right Handed
{
   return [(v1[1] * v2[2] - v1[2] * v2[1]),   (v1[2] * v2[0] - v1[0] * v2[2]),   (v1[0] * v2[1] - v1[1] * v2[0])];
}
function cross3l (v1, v2) //3D cross between two 3D vectors Left Handed
{
   return  [(v1[2] * v2[1] - v1[1] * v2[2]),   (v1[0] * v2[2] - v1[2] * v2[0]),  (v1[1] * v2[0] - v1[0] * v2[1])];
}

// component wise vector operations
function deltav  (v1, v2)    { return v2.map ( (a, i) => a - v1[i] );}
function mulv    (v,  f)     { return v.map  ( (a, i) => a * f[i] ); }
//dot product of two vectors
function dotv    (v,  f)     { return mulv   (v, f).reduce((a, b) => a + b, 0); }

//return new normalized vector
function normv   (norm)      { let len = Math.hypot ( ... norm); return norm.map(a => a / len); }

// creates array of transformed ts by scalar r
function offset  (r, ... ts) { return ts.map (a => a + r); }
function resize  (r, ... ts) { return ts.map (a => a * r); }
function shrink  (r, ... ts) { return ts.map (a => a / r); }

// reates array of transformed v1 component vise by v2
function offsetv (v1, v2)    { return v2.map ( (a, i) => v1[i] + a  ); }
function shrinkv (v1, v2)    { return v2.map ( (a, i) => v1[i] / a  ); }
function resizev (v1, v2)    { return v2.map ( (a, i) => v1[i] * a  ); }

function norm3nz (x1, y1, z1, x2, y2, z2, rs) {
   if (!rs) rs = 1;
   //let vec = normv ([x1, y1, z1], [x2, y2, z2]);
   let vec = cross3 ([x1, y1, z1], [x2, y2, z2]);
   let h   = Math.hypot (... vec);
   vec     = shrink (h,  ... vec);
   return  resize   (rs, ... vec);
};

//TODO: To review correctness and usefullness of commented constants
//const RADGRAD    = Math.PI / 180;
//const C2PI       = 2 * Math.PI;
function deg(r) { return r * 180 / Math.PI; }
function rad(d) { return d * Math.PI / 180; }

function identity1(arraytype) {
   if (!arraytype) arraytype = Array;
   return arraytype.from ([1]);
}
function identity2(arraytype) {
   if (!arraytype) arraytype = Array;
   return arraytype.from ([1, 0,  0, 1]);
}
function identity3(arraytype) {
   if (!arraytype) arraytype = Array;
   return arraytype.from ([1, 0, 0,   0, 1, 0,   0, 0, 1]);
}
function identity4(arraytype) {
   if (!arraytype) arraytype = Array;
   return arraytype.from ([1, 0, 0, 0,   0, 1, 0, 0,   0, 0, 1, 0,   0, 0, 0, 1]);
}
function identity(n, arraytype) {
   if (!arraytype) arraytype = Array;
   let ret = new arraytype(n * n).fill(0);
   for (let i = 0; i < n; i++)
      ret[i * n + i] = 1;
   return ret;
}

let angle2coord = (r,  fi) => [r * Math.cos(fi),   r * Math.sin(fi)];
let sect2coord  = (r, sec) => angle2coord (r, sec * 2 * Math.PI);