"use strict";

//========================================
// GeometryBuffers - Output container for build methods
//========================================
class GeometryBuffers {
   constructor() {
      this.verts = [];
      this.norms = [];
      this.texCoords = [];
      this.colors = [];
      this.indices = [];
   }
}

//========================================
// Helper functions
//========================================
function crossProductNormalLefthanded(x0, y0, z0, x1, y1, z1, x2, y2, z2, invert = false) {
   let ax = x1 - x0, ay = y1 - y0, az = z1 - z0;
   let bx = x2 - x0, by = y2 - y0, bz = z2 - z0;
   
   let nx = ay * bz - az * by;
   let ny = az * bx - ax * bz;
   let nz = ax * by - ay * bx;
   
   if (invert) { nx = -nx; ny = -ny; nz = -nz; }
   
   let len = Math.sqrt(nx*nx + ny*ny + nz*nz);
   if (len > 0.0001) { nx /= len; ny /= len; nz /= len; }
   
   return { nx, ny, nz };
}

//========================================
// PolarBuilder - builds geometry from polar formulas r(theta)
//========================================
class PolarBuilder {
   #formula = "1";
   #domainStart = 0;
   #domainEnd = 2 * Math.PI;
   #sectors = 20;
   #slices = 3;
   #turbo = true;
   #smooth = true;
   #doubleCoated = false;
   #reversed = false;
   #colorOuter = [1, 1, 1, 1];
   #colorInner = [1, 1, 1, 1];
   
   formula(f) { this.#formula = f; return this; }
   domain(start, end = null) {
      if (end === null) { this.#domainEnd = start; }
      else { this.#domainStart = start; this.#domainEnd = end; }
      return this;
   }
   domainShift(newEnd) {
      let range = this.#domainEnd - this.#domainStart;
      this.#domainStart = newEnd - range;
      this.#domainEnd = newEnd;
      return this;
   }
   sectors(n) { this.#sectors = n; return this; }
   slices(n) { this.#slices = n; return this; }
   sectorsSlices(s, sl) { this.#sectors = s; this.#slices = sl; return this; }
   slicesSectors(sl, s) { this.#slices = sl; this.#sectors = s; return this; }
   turbo(enabled = true) { this.#turbo = enabled; return this; }
   smooth(enabled = true) { this.#smooth = enabled; return this; }
   edged(enabled = true) { return this.smooth(!enabled); }
   doubleCoated(enabled = true) { this.#doubleCoated = enabled; return this; }
   singleCoated(enabled = true) { return this.doubleCoated(!enabled); }
   reversed(enabled = true) { this.#reversed = enabled; return this; }
   nonreversed(enabled = true) { return this.reversed(!enabled); }
   
   color(rgba, rgbaInner = null) {
      if (rgba.length === 3) rgba = [...rgba, 1];
      this.#colorOuter = rgba;
      if (rgbaInner) {
         if (rgbaInner.length === 3) rgbaInner = [...rgbaInner, 1];
         this.#colorInner = rgbaInner;
      } else {
         this.#colorInner = rgba;
      }
      return this;
   }
   
   // Build methods
   buildConeIndexed() {
      let buffers = new GeometryBuffers();
      this.#buildConeIndexedInternal(buffers, false);
      return { verts: buffers.verts, norms: buffers.norms, texCoords: buffers.texCoords, indices: buffers.indices };
   }
   
   buildConeIndexedWithColor() {
      let buffers = new GeometryBuffers();
      this.#buildConeIndexedInternal(buffers, false);
      return { verts: buffers.verts, norms: buffers.norms, colors: buffers.colors, indices: buffers.indices };
   }
   
   buildCylinderIndexed() {
      let buffers = new GeometryBuffers();
      this.#buildCylinderIndexedInternal(buffers, false);
      return { verts: buffers.verts, norms: buffers.norms, texCoords: buffers.texCoords, indices: buffers.indices };
   }
   
   buildCylinderIndexedWithColor() {
      let buffers = new GeometryBuffers();
      this.#buildCylinderIndexedInternal(buffers, false);
      return { verts: buffers.verts, norms: buffers.norms, colors: buffers.colors, indices: buffers.indices };
   }
   
   #buildConeIndexedInternal(buffers, isSecondCoat) {
      let compiler = new ExpressionCompiler();
      let theta = { value: 0 };
      
      let exprR = compiler.compile(this.#formula);
      exprR.bind("theta", () => theta.value);
      
      const zTip = this.#reversed ? 0 : -1;
      const zBase = this.#reversed ? -1 : 0;
      const c = isSecondCoat ? this.#colorInner : this.#colorOuter;
      
      let addVertex = (x, y, z, nx, ny, nz, u, v) => {
         let idx = buffers.verts.length / 3;
         buffers.verts.push(x, y, z);
         buffers.norms.push(nx, ny, nz);
         buffers.texCoords.push(u, v);
         buffers.colors.push(c[0], c[1], c[2], c[3]);
         return idx;
      };
      
      let domainRange = this.#domainEnd - this.#domainStart;
      
      // Tip vertex
      let tipIndex = addVertex(0, 0, zTip, 0, 0, isSecondCoat ? 1 : -1, 0.5, 0);
      
      // Build first ring
      let baseRing = [];
      for (let i = 0; i <= this.#sectors; i++) {
         theta.value = this.#domainStart + domainRange * i / this.#sectors;
         let scale = 1 / this.#slices;
         
         let x = exprR.cylX(theta.value) * scale;
         let y = exprR.cylY(theta.value) * scale;
         let z = zTip + (zBase - zTip) * scale;
         
         // Approximate normal
         let r = exprR.eval();
         let cosT = Math.cos(theta.value);
         let sinT = Math.sin(theta.value);
         let nx = r * cosT;
         let ny = r * sinT;
         let nz = 1;
         
         if (isSecondCoat) { nx = -nx; ny = -ny; nz = -nz; }
         
         let len = Math.sqrt(nx*nx + ny*ny + nz*nz);
         if (len > 0.0001) { nx /= len; ny /= len; nz /= len; }
         
         baseRing.push(addVertex(x, y, z, nx, ny, nz, i / this.#sectors, scale));
      }
      
      // Tip triangles
      for (let i = 0; i < this.#sectors; i++) {
         if (!isSecondCoat) {
            buffers.indices.push(tipIndex, baseRing[i], baseRing[i + 1]);
         } else {
            buffers.indices.push(tipIndex, baseRing[i + 1], baseRing[i]);
         }
      }
      
      // Remaining rings
      let prevRing = baseRing;
      for (let h = 1; h < this.#slices; h++) {
         let scale = (h + 1) / this.#slices;
         let z = zTip + (zBase - zTip) * scale;
         let currRing = [];
         
         for (let i = 0; i <= this.#sectors; i++) {
            theta.value = this.#domainStart + domainRange * i / this.#sectors;
            
            let x = exprR.cylX(theta.value) * scale;
            let y = exprR.cylY(theta.value) * scale;
            
            let r = exprR.eval();
            let cosT = Math.cos(theta.value);
            let sinT = Math.sin(theta.value);
            let nx = r * cosT;
            let ny = r * sinT;
            let nz = 1;
            
            if (isSecondCoat) { nx = -nx; ny = -ny; nz = -nz; }
            
            let len = Math.sqrt(nx*nx + ny*ny + nz*nz);
            if (len > 0.0001) { nx /= len; ny /= len; nz /= len; }
            
            currRing.push(addVertex(x, y, z, nx, ny, nz, i / this.#sectors, scale));
         }
         
         // Quads
         for (let i = 0; i < this.#sectors; i++) {
            let v00 = prevRing[i], v01 = prevRing[i + 1];
            let v10 = currRing[i], v11 = currRing[i + 1];
            
            if (!isSecondCoat) {
               buffers.indices.push(v00, v10, v01);
               buffers.indices.push(v01, v10, v11);
            } else {
               buffers.indices.push(v00, v01, v10);
               buffers.indices.push(v01, v11, v10);
            }
         }
         
         prevRing = currRing;
      }
      
      if (!isSecondCoat && this.#doubleCoated) {
         this.#buildConeIndexedInternal(buffers, true);
      }
   }
   
   #buildCylinderIndexedInternal(buffers, isSecondCoat) {
      let compiler = new ExpressionCompiler();
      let theta = { value: 0 };
      
      let exprR = compiler.compile(this.#formula);
      exprR.bind("theta", () => theta.value);
      
      const c = isSecondCoat ? this.#colorInner : this.#colorOuter;
      
      let addVertex = (x, y, z, nx, ny, nz, u, v) => {
         let idx = buffers.verts.length / 3;
         buffers.verts.push(x, y, z);
         buffers.norms.push(nx, ny, nz);
         buffers.texCoords.push(u, v);
         buffers.colors.push(c[0], c[1], c[2], c[3]);
         return idx;
      };
      
      let domainRange = this.#domainEnd - this.#domainStart;
      
      // Build first ring at z=0
      let prevRing = [];
      for (let i = 0; i <= this.#sectors; i++) {
         theta.value = this.#domainStart + domainRange * i / this.#sectors;
         
         let x = exprR.cylX(theta.value);
         let y = exprR.cylY(theta.value);
         
         let r = exprR.eval();
         let cosT = Math.cos(theta.value);
         let sinT = Math.sin(theta.value);
         let nx = r * cosT;
         let ny = r * sinT;
         let nz = 0;
         
         if (isSecondCoat) { nx = -nx; ny = -ny; }
         
         let len = Math.sqrt(nx*nx + ny*ny);
         if (len > 0.0001) { nx /= len; ny /= len; }
         
         prevRing.push(addVertex(x, y, 0, nx, ny, nz, i / this.#sectors, 0));
      }
      
      // Build remaining rings
      for (let h = 1; h <= this.#slices; h++) {
         let t = h / this.#slices;
         let z = -t;
         let currRing = [];
         
         for (let i = 0; i <= this.#sectors; i++) {
            theta.value = this.#domainStart + domainRange * i / this.#sectors;
            
            let x = exprR.cylX(theta.value);
            let y = exprR.cylY(theta.value);
            
            let r = exprR.eval();
            let cosT = Math.cos(theta.value);
            let sinT = Math.sin(theta.value);
            let nx = r * cosT;
            let ny = r * sinT;
            let nz = 0;
            
            if (isSecondCoat) { nx = -nx; ny = -ny; }
            
            let len = Math.sqrt(nx*nx + ny*ny);
            if (len > 0.0001) { nx /= len; ny /= len; }
            
            currRing.push(addVertex(x, y, z, nx, ny, nz, i / this.#sectors, t));
         }
         
         // Quads
         for (let i = 0; i < this.#sectors; i++) {
            let v00 = prevRing[i], v01 = prevRing[i + 1];
            let v10 = currRing[i], v11 = currRing[i + 1];
            
            if (isSecondCoat) {
               buffers.indices.push(v00, v10, v01);
               buffers.indices.push(v01, v10, v11);
            } else {
               buffers.indices.push(v00, v01, v10);
               buffers.indices.push(v01, v11, v10);
            }
         }
         
         prevRing = currRing;
      }
      
      if (!isSecondCoat && this.#doubleCoated) {
         this.#buildCylinderIndexedInternal(buffers, true);
      }
   }
}

//========================================
// CartesianBuilder - builds geometry from cartesian formulas Y=f(X)
//========================================
class CartesianBuilder {
   #formula = "x";
   #xStart = -1;
   #xEnd = 1;
   #sectors = 10;
   #slices = 1;
   #turbo = true;
   #smooth = true;
   #doubleCoated = false;
   #reversed = false;
   #colorOuter = [1, 1, 1, 1];
   #colorInner = [1, 1, 1, 1];
   
   formula(f) { this.#formula = f; return this; }
   domain(xStart, xEnd) { this.#xStart = xStart; this.#xEnd = xEnd; return this; }
   sectors(n) { this.#sectors = n; return this; }
   slices(n) { this.#slices = n; return this; }
   sectorsSlices(s, sl) { this.#sectors = s; this.#slices = sl; return this; }
   slicesSectors(sl, s) { this.#slices = sl; this.#sectors = s; return this; }
   turbo(enabled = true) { this.#turbo = enabled; return this; }
   smooth(enabled = true) { this.#smooth = enabled; return this; }
   edged(enabled = true) { return this.smooth(!enabled); }
   doubleCoated(enabled = true) { this.#doubleCoated = enabled; return this; }
   singleCoated(enabled = true) { return this.doubleCoated(!enabled); }
   reversed(enabled = true) { this.#reversed = enabled; return this; }
   nonreversed(enabled = true) { return this.reversed(!enabled); }
   
   color(rgba, rgbaInner = null) {
      if (rgba.length === 3) rgba = [...rgba, 1];
      this.#colorOuter = rgba;
      if (rgbaInner) {
         if (rgbaInner.length === 3) rgbaInner = [...rgbaInner, 1];
         this.#colorInner = rgbaInner;
      } else {
         this.#colorInner = rgba;
      }
      return this;
   }
   
   buildConeIndexed() {
      let buffers = new GeometryBuffers();
      this.#buildConeIndexedInternal(buffers, false);
      return { verts: buffers.verts, norms: buffers.norms, texCoords: buffers.texCoords, indices: buffers.indices };
   }
   
   buildConeIndexedWithColor() {
      let buffers = new GeometryBuffers();
      this.#buildConeIndexedInternal(buffers, false);
      return { verts: buffers.verts, norms: buffers.norms, colors: buffers.colors, indices: buffers.indices };
   }
   
   buildCylinderIndexed() {
      let buffers = new GeometryBuffers();
      this.#buildCylinderIndexedInternal(buffers, false);
      return { verts: buffers.verts, norms: buffers.norms, texCoords: buffers.texCoords, indices: buffers.indices };
   }
   
   buildCylinderIndexedWithColor() {
      let buffers = new GeometryBuffers();
      this.#buildCylinderIndexedInternal(buffers, false);
      return { verts: buffers.verts, norms: buffers.norms, colors: buffers.colors, indices: buffers.indices };
   }
   
   #buildConeIndexedInternal(buffers, isSecondCoat) {
      let compiler = new ExpressionCompiler();
      let xVar = { value: 0 };
      
      let exprY = compiler.compile(this.#formula);
      exprY.bind("x", () => xVar.value);
      
      const zTip = this.#reversed ? 0 : -1;
      const zBase = this.#reversed ? -1 : 0;
      const c = isSecondCoat ? this.#colorInner : this.#colorOuter;
      const dx = (this.#xEnd - this.#xStart) / this.#sectors;
      
      // Tip at center
      let tipX = (this.#xStart + this.#xEnd) / 2;
      xVar.value = tipX;
      let tipY = exprY.eval();
      
      let addVertex = (x, y, z, nx, ny, nz, u, v) => {
         let idx = buffers.verts.length / 3;
         buffers.verts.push(x, y, z);
         buffers.norms.push(nx, ny, nz);
         buffers.texCoords.push(u, v);
         buffers.colors.push(c[0], c[1], c[2], c[3]);
         return idx;
      };
      
      // Precompute base curve
      let baseX = [], baseY = [];
      for (let i = 0; i <= this.#sectors; i++) {
         let x = this.#xStart + dx * i;
         xVar.value = x;
         baseX.push(x);
         baseY.push(exprY.eval());
      }
      
      // Tip vertex
      let tipIndex = addVertex(tipX, tipY, zTip, 0, 0, isSecondCoat ? 1 : -1, 0.5, 0);
      
      // First ring
      let firstRing = [];
      let scale = 1 / this.#slices;
      let z = zTip + (zBase - zTip) * scale;
      
      for (let i = 0; i <= this.#sectors; i++) {
         let x = tipX + (baseX[i] - tipX) * scale;
         let y = tipY + (baseY[i] - tipY) * scale;
         
         let nx = -(baseY[i] - tipY);
         let ny = baseX[i] - tipX;
         let nz = 1;
         if (isSecondCoat) { nx = -nx; ny = -ny; nz = -nz; }
         
         let len = Math.sqrt(nx*nx + ny*ny + nz*nz);
         if (len > 0.0001) { nx /= len; ny /= len; nz /= len; }
         
         firstRing.push(addVertex(x, y, z, nx, ny, nz, i / this.#sectors, scale));
      }
      
      // Tip triangles
      for (let i = 0; i < this.#sectors; i++) {
         if (!isSecondCoat) {
            buffers.indices.push(tipIndex, firstRing[i], firstRing[i + 1]);
         } else {
            buffers.indices.push(tipIndex, firstRing[i + 1], firstRing[i]);
         }
      }
      
      // Remaining rings
      let prevRing = firstRing;
      for (let h = 1; h < this.#slices; h++) {
         scale = (h + 1) / this.#slices;
         z = zTip + (zBase - zTip) * scale;
         let currRing = [];
         
         for (let i = 0; i <= this.#sectors; i++) {
            let x = tipX + (baseX[i] - tipX) * scale;
            let y = tipY + (baseY[i] - tipY) * scale;
            
            let nx = -(baseY[i] - tipY);
            let ny = baseX[i] - tipX;
            let nz = 1;
            if (isSecondCoat) { nx = -nx; ny = -ny; nz = -nz; }
            
            let len = Math.sqrt(nx*nx + ny*ny + nz*nz);
            if (len > 0.0001) { nx /= len; ny /= len; nz /= len; }
            
            currRing.push(addVertex(x, y, z, nx, ny, nz, i / this.#sectors, scale));
         }
         
         for (let i = 0; i < this.#sectors; i++) {
            let v00 = prevRing[i], v01 = prevRing[i + 1];
            let v10 = currRing[i], v11 = currRing[i + 1];
            
            if (!isSecondCoat) {
               buffers.indices.push(v00, v10, v01);
               buffers.indices.push(v01, v10, v11);
            } else {
               buffers.indices.push(v00, v01, v10);
               buffers.indices.push(v01, v11, v10);
            }
         }
         
         prevRing = currRing;
      }
      
      if (!isSecondCoat && this.#doubleCoated) {
         this.#buildConeIndexedInternal(buffers, true);
      }
   }
   
   #buildCylinderIndexedInternal(buffers, isSecondCoat) {
      let compiler = new ExpressionCompiler();
      let xVar = { value: 0 };
      
      let exprY = compiler.compile(this.#formula);
      exprY.bind("x", () => xVar.value);
      
      const c = isSecondCoat ? this.#colorInner : this.#colorOuter;
      const dx = (this.#xEnd - this.#xStart) / this.#sectors;
      
      let addVertex = (x, y, z, nx, ny, nz, u, v) => {
         let idx = buffers.verts.length / 3;
         buffers.verts.push(x, y, z);
         buffers.norms.push(nx, ny, nz);
         buffers.texCoords.push(u, v);
         buffers.colors.push(c[0], c[1], c[2], c[3]);
         return idx;
      };
      
      // Precompute curve
      let curveX = [], curveY = [];
      for (let i = 0; i <= this.#sectors; i++) {
         let x = this.#xStart + dx * i;
         xVar.value = x;
         curveX.push(x);
         curveY.push(exprY.eval());
      }
      
      // First ring at z=0
      let prevRing = [];
      for (let i = 0; i <= this.#sectors; i++) {
         let x = curveX[i], y = curveY[i];
         
         // Normal perpendicular to curve
         let nx, ny;
         if (i === 0) {
            nx = -(curveY[1] - curveY[0]);
            ny = curveX[1] - curveX[0];
         } else if (i === this.#sectors) {
            nx = -(curveY[i] - curveY[i-1]);
            ny = curveX[i] - curveX[i-1];
         } else {
            nx = -(curveY[i+1] - curveY[i-1]) / 2;
            ny = (curveX[i+1] - curveX[i-1]) / 2;
         }
         
         if (isSecondCoat) { nx = -nx; ny = -ny; }
         
         let len = Math.sqrt(nx*nx + ny*ny);
         if (len > 0.0001) { nx /= len; ny /= len; }
         
         prevRing.push(addVertex(x, y, 0, nx, ny, 0, i / this.#sectors, 0));
      }
      
      // Remaining rings
      for (let h = 1; h <= this.#slices; h++) {
         let t = h / this.#slices;
         let z = -t;
         let currRing = [];
         
         for (let i = 0; i <= this.#sectors; i++) {
            let x = curveX[i], y = curveY[i];
            
            let nx, ny;
            if (i === 0) {
               nx = -(curveY[1] - curveY[0]);
               ny = curveX[1] - curveX[0];
            } else if (i === this.#sectors) {
               nx = -(curveY[i] - curveY[i-1]);
               ny = curveX[i] - curveX[i-1];
            } else {
               nx = -(curveY[i+1] - curveY[i-1]) / 2;
               ny = (curveX[i+1] - curveX[i-1]) / 2;
            }
            
            if (isSecondCoat) { nx = -nx; ny = -ny; }
            
            let len = Math.sqrt(nx*nx + ny*ny);
            if (len > 0.0001) { nx /= len; ny /= len; }
            
            currRing.push(addVertex(x, y, z, nx, ny, 0, i / this.#sectors, t));
         }
         
         for (let i = 0; i < this.#sectors; i++) {
            let v00 = prevRing[i], v01 = prevRing[i + 1];
            let v10 = currRing[i], v11 = currRing[i + 1];
            
            if (isSecondCoat) {
               buffers.indices.push(v00, v10, v01);
               buffers.indices.push(v01, v10, v11);
            } else {
               buffers.indices.push(v00, v01, v10);
               buffers.indices.push(v01, v11, v10);
            }
         }
         
         prevRing = currRing;
      }
      
      if (!isSecondCoat && this.#doubleCoated) {
         this.#buildCylinderIndexedInternal(buffers, true);
      }
   }
}

//========================================
// Builder - factory class
//========================================
class Builder {
   static polar()     { return new PolarBuilder(); }
   static cartesian() { return new CartesianBuilder(); }
}