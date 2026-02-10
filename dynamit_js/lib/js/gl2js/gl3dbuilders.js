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
   
   applyTransform(matrix) {
      for (let i = 0; i < this.verts.length; i += 3) {
         let x = this.verts[i], y = this.verts[i+1], z = this.verts[i+2];
         this.verts[i]   = matrix[0]*x + matrix[4]*y + matrix[8]*z  + matrix[12];
         this.verts[i+1] = matrix[1]*x + matrix[5]*y + matrix[9]*z  + matrix[13];
         this.verts[i+2] = matrix[2]*x + matrix[6]*y + matrix[10]*z + matrix[14];
      }
      for (let i = 0; i < this.norms.length; i += 3) {
         let x = this.norms[i], y = this.norms[i+1], z = this.norms[i+2];
         this.norms[i]   = matrix[0]*x + matrix[4]*y + matrix[8]*z;
         this.norms[i+1] = matrix[1]*x + matrix[5]*y + matrix[9]*z;
         this.norms[i+2] = matrix[2]*x + matrix[6]*y + matrix[10]*z;
         let len = Math.sqrt(this.norms[i]**2 + this.norms[i+1]**2 + this.norms[i+2]**2);
         if (len > 0.0001) {
            this.norms[i] /= len; this.norms[i+1] /= len; this.norms[i+2] /= len;
         }
      }
   }
}

//========================================
// Helper functions
//========================================
function rotationXMatrix(angle) {
   let c = Math.cos(angle), s = Math.sin(angle);
   return [1,0,0,0, 0,c,s,0, 0,-s,c,0, 0,0,0,1];
}
function rotationYMatrix(angle) {
   let c = Math.cos(angle), s = Math.sin(angle);
   return [c,0,-s,0, 0,1,0,0, s,0,c,0, 0,0,0,1];
}
function rotationZMatrix(angle) {
   let c = Math.cos(angle), s = Math.sin(angle);
   return [c,s,0,0, -s,c,0,0, 0,0,1,0, 0,0,0,1];
}

//========================================
// PolarBuilder - r(theta)
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
   #transformMatrix = null;
   
   formula(f) { this.#formula = f; return this; }
   domain(start, end = null) {
      if (end === null) { this.#domainEnd = start; }
      else { this.#domainStart = start; this.#domainEnd = end; }
      return this;
   }
   domainShift(newEnd) { let range = this.#domainEnd - this.#domainStart; this.#domainStart = newEnd - range; this.#domainEnd = newEnd; return this; }
   sectors(n) { this.#sectors = n; return this; }
   slices(n) { this.#slices = n; return this; }
   slices_sectors(sl, s) { this.#slices = sl; this.#sectors = s; return this; }
   sectors_slices(s, sl) { this.#sectors = s; this.#slices = sl; return this; }
   turbo(enabled = true) { this.#turbo = enabled; return this; }
   smooth(enabled = true) { this.#smooth = enabled; return this; }
   edged(enabled = true) { return this.smooth(!enabled); }
   doubleCoated(enabled = true) { this.#doubleCoated = enabled; return this; }
   singleCoated(enabled = true) { return this.doubleCoated(!enabled); }
   reversed(enabled = true) { this.#reversed = enabled; return this; }
   nonreversed(enabled = true) { return this.reversed(!enabled); }
   transform(matrix) { this.#transformMatrix = matrix; return this; }
   rotateX(angle) { this.#transformMatrix = rotationXMatrix(angle); return this; }
   rotateY(angle) { this.#transformMatrix = rotationYMatrix(angle); return this; }
   rotateZ(angle) { this.#transformMatrix = rotationZMatrix(angle); return this; }
   color(rgba, rgbaInner = null) {
      if (rgba.length === 3) rgba = [...rgba, 1];
      this.#colorOuter = rgba;
      this.#colorInner = rgbaInner ? (rgbaInner.length === 3 ? [...rgbaInner, 1] : rgbaInner) : rgba;
      return this;
   }
   
   #applyTransformIfNeeded(buffers) { if (this.#transformMatrix) buffers.applyTransform(this.#transformMatrix); }
   
   buildConeIndexed() { let b = new GeometryBuffers(); this.#buildConeIndexedInternal(b, false); this.#applyTransformIfNeeded(b); return { verts: b.verts, norms: b.norms, texCoords: b.texCoords, indices: b.indices }; }
   buildConeIndexedWithColor() { let b = new GeometryBuffers(); this.#buildConeIndexedInternal(b, false); this.#applyTransformIfNeeded(b); return { verts: b.verts, norms: b.norms, colors: b.colors, indices: b.indices }; }
   buildCylinderIndexed() { let b = new GeometryBuffers(); this.#buildCylinderIndexedInternal(b, false); this.#applyTransformIfNeeded(b); return { verts: b.verts, norms: b.norms, texCoords: b.texCoords, indices: b.indices }; }
   buildCylinderIndexedWithColor() { let b = new GeometryBuffers(); this.#buildCylinderIndexedInternal(b, false); this.#applyTransformIfNeeded(b); return { verts: b.verts, norms: b.norms, colors: b.colors, indices: b.indices }; }
   
   #buildConeIndexedInternal(buffers, isSecondCoat) {
      let compiler = new ExpressionCompiler();
      let theta = { value: 0 };
      
      let exprR = compiler.compile(this.#formula);
      exprR.bind("theta", () => theta.value);
      
      let exprDR = simplify(exprR.derivative("theta"));
      exprDR.bind("theta", () => theta.value);
      
      const zTip = this.#reversed ? 0 : -1;
      const zBase = this.#reversed ? -1 : 0;
      const c = isSecondCoat ? this.#colorInner : this.#colorOuter;
      let domainRange = this.#domainEnd - this.#domainStart;
      
      let addVertex = (x, y, z, nx, ny, nz, u, v) => {
         let idx = buffers.verts.length / 3;
         buffers.verts.push(x, y, z);
         buffers.norms.push(nx, ny, nz);
         buffers.texCoords.push(u, v);
         buffers.colors.push(c[0], c[1], c[2], c[3]);
         return idx;
      };
      
      let tipIndex = addVertex(0, 0, zTip, 0, 0, 0, 0.5, 0);
      
      let baseX = [], baseY = [], baseNx = [], baseNy = [], baseNz = [];
      let baseRing = [];
      
      for (let i = 0; i <= this.#sectors; i++) {
         theta.value = this.#domainStart + domainRange * i / this.#sectors;
         let u = i / this.#sectors;
         
         let r = exprR.eval();
         let dr = exprDR.eval();
         
         let x = exprR.cylX(theta.value) / this.#slices;
         let y = exprR.cylY(theta.value) / this.#slices;
         
         let cosT = Math.cos(theta.value);
         let sinT = Math.sin(theta.value);
         
         // Normal calculation matching C++:
         // nx = dr*sin + r*cos, ny = -(dr*cos - r*sin), nz = -1
         let nx = dr * sinT + r * cosT;
         let ny = -(dr * cosT - r * sinT);
         let nz = -1;
         
         if (isSecondCoat) nz = 1;
         if (this.#reversed) {
            if (!isSecondCoat) { nx = -nx; ny = -ny; }
         } else {
            if (isSecondCoat) { nx = -nx; ny = -ny; }
         }
         
         let len = Math.sqrt(nx*nx + ny*ny + nz*nz);
         if (len > 0.0001) { nx /= len; ny /= len; nz /= len; }
         
         baseX[i] = x * this.#slices;
         baseY[i] = y * this.#slices;
         baseNx[i] = nx;
         baseNy[i] = ny;
         baseNz[i] = nz;
         
         let firstRingZ = zTip + (zBase - zTip) / this.#slices;
         baseRing[i] = addVertex(x, y, firstRingZ, nx, ny, nz, u, 1 / this.#slices);
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
      if (this.#slices > 1) {
         let prevRing = baseRing;
         
         for (let h = 1; h < this.#slices; h++) {
            let h2n = (h + 1) / this.#slices;
            let z = zTip + (zBase - zTip) * h2n;
            let currRing = [];
            
            for (let i = 0; i <= this.#sectors; i++) {
               let x, y, nx, ny, nz;
               let u = i / this.#sectors;
               
               if (this.#turbo) {
                  x = baseX[i] * h2n;
                  y = baseY[i] * h2n;
                  nx = baseNx[i];
                  ny = baseNy[i];
                  nz = baseNz[i];
               } else {
                  theta.value = this.#domainStart + domainRange * i / this.#sectors;
                  let r = exprR.eval();
                  let dr = exprDR.eval();
                  
                  x = exprR.cylX(theta.value) * h2n;
                  y = exprR.cylY(theta.value) * h2n;
                  
                  let cosT = Math.cos(theta.value);
                  let sinT = Math.sin(theta.value);
                  nx = dr * sinT + r * cosT;
                  ny = -(dr * cosT - r * sinT);
                  nz = -1;
                  
                  if (isSecondCoat) nz = 1;
                  if (this.#reversed) {
                     if (!isSecondCoat) { nx = -nx; ny = -ny; }
                  } else {
                     if (isSecondCoat) { nx = -nx; ny = -ny; }
                  }
                  
                  let len = Math.sqrt(nx*nx + ny*ny + nz*nz);
                  if (len > 0.0001) { nx /= len; ny /= len; nz /= len; }
               }
               
               currRing[i] = addVertex(x, y, z, nx, ny, nz, u, h2n);
            }
            
            for (let i = 0; i < this.#sectors; i++) {
               let v00 = prevRing[i], v01 = prevRing[i + 1], v10 = currRing[i], v11 = currRing[i + 1];
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
      }
      
      if (!isSecondCoat && this.#doubleCoated) this.#buildConeIndexedInternal(buffers, true);
   }
   
   #buildCylinderIndexedInternal(buffers, isSecondCoat) {
        let compiler = new ExpressionCompiler();
        let theta = { value: 0 };

        let exprR = compiler.compile(this.#formula);
        exprR.bind("theta", () => theta.value);

        let exprDR = simplify(exprR.derivative("theta"));
        exprDR.bind("theta", () => theta.value);

        const c = isSecondCoat ? this.#colorInner : this.#colorOuter;
        let domainRange = this.#domainEnd - this.#domainStart;

        let addVertex = (x, y, z, nx, ny, nz, u, v) => {
            let idx = buffers.verts.length / 3;
            buffers.verts.push(x, y, z);
            buffers.norms.push(nx, ny, nz);
            buffers.texCoords.push(u, v);
            buffers.colors.push(c[0], c[1], c[2], c[3]);
            return idx;
        };

        let baseX = [], baseY = [], baseNx = [], baseNy = [];

        // First ring at z=0
        let prevRing = [];
        for (let i = 0; i <= this.#sectors; i++) {
            theta.value = this.#domainStart + domainRange * i / this.#sectors;
            let u = i / this.#sectors;

            let r = exprR.eval();
            let dr = exprDR.eval();

            let x = exprR.cylX(theta.value);
            let y = exprR.cylY(theta.value);

            let cosT = Math.cos(theta.value);
            let sinT = Math.sin(theta.value);

            // Normal for cylinder: perpendicular to curve
            let nx = dr * sinT + r * cosT;
            let ny = -(dr * cosT - r * sinT);

            let len = Math.sqrt(nx * nx + ny * ny);
            if (len > 0.0001) { nx /= len; ny /= len; }

            if (isSecondCoat) { nx = -nx; ny = -ny; }

            baseX[i] = x;
            baseY[i] = y;
            baseNx[i] = nx;
            baseNy[i] = ny;

            prevRing[i] = addVertex(x, y, 0, nx, ny, 0, u, 0);
        }

        // Remaining rings
        for (let h = 1; h <= this.#slices; h++) {
            let t = h / this.#slices;
            let z = -t;
            let v = t;
            let currRing = [];

            for (let i = 0; i <= this.#sectors; i++) {
                let x, y, nx, ny;
                let u = i / this.#sectors;

                if (this.#turbo) {
                    x = baseX[i];
                    y = baseY[i];
                    nx = baseNx[i];
                    ny = baseNy[i];
                } else {
                    theta.value = this.#domainStart + domainRange * i / this.#sectors;

                    let r = exprR.eval();
                    let dr = exprDR.eval();

                    x = exprR.cylX(theta.value);
                    y = exprR.cylY(theta.value);

                    let cosT = Math.cos(theta.value);
                    let sinT = Math.sin(theta.value);
                    nx = dr * sinT + r * cosT;
                    ny = -(dr * cosT - r * sinT);

                    let len = Math.sqrt(nx * nx + ny * ny);
                    if (len > 0.0001) { nx /= len; ny /= len; }

                    if (isSecondCoat) { nx = -nx; ny = -ny; }
                }

                currRing[i] = addVertex(x, y, z, nx, ny, 0, u, v);
            }

            for (let i = 0; i < this.#sectors; i++) {
                let v00 = prevRing[i], v01 = prevRing[i + 1], v10 = currRing[i], v11 = currRing[i + 1];
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

        if (!isSecondCoat && this.#doubleCoated) this.#buildCylinderIndexedInternal(buffers, true);
    }
}

//========================================
// CartesianBuilder - Y=f(X)
//========================================
class CartesianBuilder {
   #formula = "x";
   #xStart = -1;
   #xEnd = 1;
   #sectors = 20;
   #slices = 3;
   #turbo = true;
   #smooth = true;
   #doubleCoated = false;
   #reversed = false;
   #colorOuter = [1, 1, 1, 1];
   #colorInner = [1, 1, 1, 1];
   #transformMatrix = null;
   
   formula(f) { this.#formula = f; return this; }
   domain(xStart, xEnd) { this.#xStart = xStart; this.#xEnd = xEnd; return this; }
   sectors(n) { this.#sectors = n; return this; }
   slices(n) { this.#slices = n; return this; }
   slices_sectors(sl, s) { this.#slices = sl; this.#sectors = s; return this; }
   sectors_slices(s, sl) { this.#sectors = s; this.#slices = sl; return this; }
   turbo(enabled = true) { this.#turbo = enabled; return this; }
   smooth(enabled = true) { this.#smooth = enabled; return this; }
   edged(enabled = true) { return this.smooth(!enabled); }
   doubleCoated(enabled = true) { this.#doubleCoated = enabled; return this; }
   singleCoated(enabled = true) { return this.doubleCoated(!enabled); }
   reversed(enabled = true) { this.#reversed = enabled; return this; }
   nonreversed(enabled = true) { return this.reversed(!enabled); }
   transform(matrix) { this.#transformMatrix = matrix; return this; }
   rotateX(angle) { this.#transformMatrix = rotationXMatrix(angle); return this; }
   rotateY(angle) { this.#transformMatrix = rotationYMatrix(angle); return this; }
   rotateZ(angle) { this.#transformMatrix = rotationZMatrix(angle); return this; }
   color(rgba, rgbaInner = null) {
      if (rgba.length === 3) rgba = [...rgba, 1];
      this.#colorOuter = rgba;
      this.#colorInner = rgbaInner ? (rgbaInner.length === 3 ? [...rgbaInner, 1] : rgbaInner) : rgba;
      return this;
   }
   
   #applyTransformIfNeeded(buffers) { if (this.#transformMatrix) buffers.applyTransform(this.#transformMatrix); }
   
   buildConeIndexed() { let b = new GeometryBuffers(); this.#buildConeIndexedInternal(b, false); this.#applyTransformIfNeeded(b); return { verts: b.verts, norms: b.norms, texCoords: b.texCoords, indices: b.indices }; }
   buildConeIndexedWithColor() { let b = new GeometryBuffers(); this.#buildConeIndexedInternal(b, false); this.#applyTransformIfNeeded(b); return { verts: b.verts, norms: b.norms, colors: b.colors, indices: b.indices }; }
   buildCylinderIndexed() { let b = new GeometryBuffers(); this.#buildCylinderIndexedInternal(b, false); this.#applyTransformIfNeeded(b); return { verts: b.verts, norms: b.norms, texCoords: b.texCoords, indices: b.indices }; }
   buildCylinderIndexedWithColor() { let b = new GeometryBuffers(); this.#buildCylinderIndexedInternal(b, false); this.#applyTransformIfNeeded(b); return { verts: b.verts, norms: b.norms, colors: b.colors, indices: b.indices }; }
   
   #buildConeIndexedInternal(buffers, isSecondCoat) {
      let compiler = new ExpressionCompiler();
      let xVar = { value: 0 };
      
      let exprY = compiler.compile(this.#formula);
      exprY.bind("x", () => xVar.value);
      
      let exprDY = simplify(exprY.derivative("x"));
      exprDY.bind("x", () => xVar.value);
      
      const zTip = this.#reversed ? 0 : -1;
      const zBase = this.#reversed ? -1 : 0;
      const c = isSecondCoat ? this.#colorInner : this.#colorOuter;
      const dx = (this.#xEnd - this.#xStart) / this.#sectors;
      
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
      
      let tipIndex = addVertex(tipX, tipY, zTip, 0, 0, 0, 0.5, 0);
      
      let baseX = [], baseY = [], baseNx = [], baseNy = [], baseNz = [];
      let baseRing = [];
      
      for (let i = 0; i <= this.#sectors; i++) {
         let x = this.#xStart + dx * i;
         let u = i / this.#sectors;
         
         xVar.value = x;
         let y = exprY.eval();
         let dy = exprDY.eval();
         
         // Normal: perpendicular to tangent (1, dy), so (-dy, 1)
         let nx = -dy;
         let ny = 1;
         let nz = -1;
         
         if (isSecondCoat) nz = 1;
         if (this.#reversed) {
            if (!isSecondCoat) { nx = -nx; ny = -ny; }
         } else {
            if (isSecondCoat) { nx = -nx; ny = -ny; }
         }
         
         let len = Math.sqrt(nx*nx + ny*ny + nz*nz);
         if (len > 0.0001) { nx /= len; ny /= len; nz /= len; }
         
         baseX[i] = x;
         baseY[i] = y;
         baseNx[i] = nx;
         baseNy[i] = ny;
         baseNz[i] = nz;
         
         baseRing[i] = addVertex(x, y, zTip + (this.#reversed ? 0 : -1), nx, ny, nz, u, 1 / this.#slices);
      }
      
      // Tip triangles
      for (let i = 0; i < this.#sectors; i++) {
         if (!isSecondCoat) buffers.indices.push(tipIndex, baseRing[i], baseRing[i + 1]);
         else buffers.indices.push(tipIndex, baseRing[i + 1], baseRing[i]);
      }
      
      if (this.#slices > 1) {
         let prevRing = baseRing;
         
         for (let h = 1; h < this.#slices; h++) {
            let scale = (h + 1) / this.#slices;
            let z = zTip + (zBase - zTip) * scale;
            let currRing = [];
            
            for (let i = 0; i <= this.#sectors; i++) {
               let x, y, nx, ny, nz;
               let u = i / this.#sectors;
               
               if (this.#turbo) {
                  x = tipX + (baseX[i] - tipX) * scale;
                  y = tipY + (baseY[i] - tipY) * scale;
                  nx = baseNx[i];
                  ny = baseNy[i];
                  nz = baseNz[i];
               } else {
                  let baseXVal = this.#xStart + dx * i;
                  xVar.value = baseXVal;
                  let baseYVal = exprY.eval();
                  let dy = exprDY.eval();
                  
                  x = tipX + (baseXVal - tipX) * scale;
                  y = tipY + (baseYVal - tipY) * scale;
                  
                  nx = -dy;
                  ny = 1;
                  nz = -1;
                  if (isSecondCoat) nz = 1;
                  if (this.#reversed) {
                     if (!isSecondCoat) { nx = -nx; ny = -ny; }
                  } else {
                     if (isSecondCoat) { nx = -nx; ny = -ny; }
                  }
                  
                  let len = Math.sqrt(nx*nx + ny*ny + nz*nz);
                  if (len > 0.0001) { nx /= len; ny /= len; nz /= len; }
               }
               
               currRing[i] = addVertex(x, y, z, nx, ny, nz, u, scale);
            }
            
            for (let i = 0; i < this.#sectors; i++) {
               let v00 = prevRing[i], v01 = prevRing[i + 1], v10 = currRing[i], v11 = currRing[i + 1];
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
      }
      
      if (!isSecondCoat && this.#doubleCoated) this.#buildConeIndexedInternal(buffers, true);
   }
   
   #buildCylinderIndexedInternal(buffers, isSecondCoat) {
      let compiler = new ExpressionCompiler();
      let xVar = { value: 0 };
      
      let exprY = compiler.compile(this.#formula);
      exprY.bind("x", () => xVar.value);
      
      let exprDY = simplify(exprY.derivative("x"));
      exprDY.bind("x", () => xVar.value);
      
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
      
      let baseX = [], baseY = [], baseNx = [], baseNy = [];
      
      // First ring at z=0
      let prevRing = [];
      for (let i = 0; i <= this.#sectors; i++) {
         let x = this.#xStart + dx * i;
         let u = i / this.#sectors;
         
         xVar.value = x;
         let y = exprY.eval();
         let dy = exprDY.eval();
         
         // Normal perpendicular to tangent (1, dy) -> (-dy, 1)
         let nx = -dy;
         let ny = 1;
         
         let len = Math.sqrt(nx*nx + ny*ny);
         if (len > 0.0001) { nx /= len; ny /= len; }
         
         if (isSecondCoat) { nx = -nx; ny = -ny; }
         
         baseX[i] = x;
         baseY[i] = y;
         baseNx[i] = nx;
         baseNy[i] = ny;
         
         prevRing[i] = addVertex(x, y, 0, nx, ny, 0, u, 0);
      }
      
      // Remaining rings
      for (let h = 1; h <= this.#slices; h++) {
         let t = h / this.#slices;
         let z = -t;
         let v = t;
         let currRing = [];
         
         for (let i = 0; i <= this.#sectors; i++) {
            let x, y, nx, ny;
            let u = i / this.#sectors;
            
            if (this.#turbo) {
               x = baseX[i];
               y = baseY[i];
               nx = baseNx[i];
               ny = baseNy[i];
            } else {
               x = this.#xStart + dx * i;
               xVar.value = x;
               y = exprY.eval();
               let dy = exprDY.eval();
               
               nx = -dy;
               ny = 1;
               
               let len = Math.sqrt(nx*nx + ny*ny);
               if (len > 0.0001) { nx /= len; ny /= len; }
               
               if (isSecondCoat) { nx = -nx; ny = -ny; }
            }
            
            currRing[i] = addVertex(x, y, z, nx, ny, 0, u, v);
         }
         
         for (let i = 0; i < this.#sectors; i++) {
            let v00 = prevRing[i], v01 = prevRing[i + 1], v10 = currRing[i], v11 = currRing[i + 1];
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
      
      if (!isSecondCoat && this.#doubleCoated) this.#buildCylinderIndexedInternal(buffers, true);
   }
}

//========================================
// Builder - factory class
//========================================
class Builder {
   static polar()       { return new PolarBuilder(); }
   static cylindrical() { return new PolarBuilder(); }
   static cartesian()   { return new CartesianBuilder(); }
}