"use strict";

//========================================
// 3DCalculator - Expression evaluator for parametric geometry
// JavaScript port - uses Function() for expression evaluation
//========================================

//========================================
// ExpressionContext - manages variable bindings
//========================================
class ExpressionContext {
   #bindings = new Map();
   
   bind(name, valueOrGetter) {
      // Can bind a value directly or a getter function
      this.#bindings.set(name.toLowerCase(), valueOrGetter);
      return this;
   }
   
   unbind(name) {
      this.#bindings.delete(name.toLowerCase());
      return this;
   }
   
   get(name) {
      return this.#bindings.get(name.toLowerCase());
   }
   
   has(name) {
      return this.#bindings.has(name.toLowerCase());
   }
   
   clear() {
      this.#bindings.clear();
      return this;
   }
   
   // Get all bindings as object for Function() scope
   toObject() {
      let obj = {};
      for (let [name, value] of this.#bindings) {
         obj[name] = typeof value === 'function' ? value() : value;
      }
      return obj;
   }
   
   get names() {
      return Array.from(this.#bindings.keys());
   }
}

//========================================
// Built-in constants
//========================================
const MathConstants = {
   PI:      Math.PI,
   E:       Math.E,
   TAU:     Math.PI * 2,
   M_PI:    Math.PI,
   M_E:     Math.E,
   M_PI_2:  Math.PI / 2,
   M_PI_4:  Math.PI / 4,
   M_SQRT2: Math.SQRT2,
   M_LN2:   Math.LN2,
   M_LN10:  Math.LN10
};

//========================================
// Expression - compiled expression with variable bindings
//========================================
class Expression {
   #formula = "";
   #compiledFn = null;
   #context = new ExpressionContext();
   #varNames = [];
   
   constructor(formula) {
      this.#formula = formula;
      this.#compile();
   }
   
   #compile() {
      // Transform formula for JavaScript compatibility
      let jsFormula = this.#formula
         // Replace ** with Math.pow for older compatibility (optional, modern JS supports **)
         // .replace(/\*\*/g, ',')  // temporarily mark
         // Handle functions - wrap with Math.
         .replace(/\b(sin|cos|tan|asin|acos|atan|atan2|sinh|cosh|tanh|sqrt|abs|exp|log|log10|log2|pow|floor|ceil|round|min|max|sign)\s*\(/gi, 
                  (match, fn) => `Math.${fn.toLowerCase()}(`)
         // Handle PI, E as constants
         .replace(/\bPI\b/gi, 'PI')
         .replace(/\bE\b/gi, 'E')
         .replace(/\bTAU\b/gi, 'TAU');
      
      // Extract variable names (identifiers that aren't Math functions or constants)
      let identifierRe = /\b([a-zA-Z_][a-zA-Z0-9_]*)\b/g;
      let match;
      let varSet = new Set();
      
      while ((match = identifierRe.exec(this.#formula)) !== null) {
         let name = match[1].toLowerCase();
         // Skip Math functions and known constants
         if (['sin', 'cos', 'tan', 'asin', 'acos', 'atan', 'atan2', 'sinh', 'cosh', 'tanh',
              'sqrt', 'abs', 'exp', 'log', 'log10', 'log2', 'pow', 'floor', 'ceil', 'round',
              'min', 'max', 'sign', 'pi', 'e', 'tau', 'math'].includes(name)) {
            continue;
         }
         varSet.add(name);
      }
      
      this.#varNames = Array.from(varSet);
      
      // Build function with constants and variables as parameters
      let allNames = [...Object.keys(MathConstants), ...this.#varNames];
      
      try {
         // Create function that takes all variables as arguments
         this.#compiledFn = new Function(...allNames, `return ${jsFormula};`);
      } catch (e) {
         throw new Error(`Failed to compile expression "${this.#formula}": ${e.message}`);
      }
   }
   
   get formula() { return this.#formula; }
   get context() { return this.#context; }
   get variableNames() { return [...this.#varNames]; }
   
   bind(name, valueOrGetter) {
      this.#context.bind(name, valueOrGetter);
      return this;
   }
   
   unbind(name) {
      this.#context.unbind(name);
      return this;
   }
   
   eval() {
      // Build arguments array: constants first, then variables
      let args = [
         ...Object.values(MathConstants),
         ...this.#varNames.map(name => {
            let val = this.#context.get(name);
            if (val === undefined) {
               throw new Error(`Unbound variable: ${name}`);
            }
            return typeof val === 'function' ? val() : val;
         })
      ];
      
      return this.#compiledFn(...args);
   }
   
   // Cylindrical coordinate helpers
   // r = eval(), returns r * cos(theta)
   cylX(theta) {
      return this.eval() * Math.cos(theta);
   }
   
   // r = eval(), returns r * sin(theta)
   cylY(theta) {
      return this.eval() * Math.sin(theta);
   }
}

//========================================
// ExpressionCompiler - factory for Expression objects
//========================================
class ExpressionCompiler {
   compile(formula) {
      return new Expression(formula);
   }
}

//========================================
// Geometry Builders using expressions
//========================================

// Cross product of vectors from 3 points
function cross3p(p0, p1, p2) {
   let ax = p1[0] - p0[0], ay = p1[1] - p0[1], az = p1[2] - p0[2];
   let bx = p2[0] - p0[0], by = p2[1] - p0[1], bz = p2[2] - p0[2];
   return [
      ay * bz - az * by,
      az * bx - ax * bz,
      ax * by - ay * bx
   ];
}

// Normalize vector
function normalize3(v) {
   let len = Math.sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
   if (len < 0.0001) return [0, 0, 0];
   return [v[0]/len, v[1]/len, v[2]/len];
}

//========================================
// buildConePolarIndexed - creates indexed cone geometry from polar formula
//========================================
function buildConePolarIndexed(formula, domainStart = 0, domainEnd = 2 * Math.PI, nsectors = 20, nslices = 3) {
   let compiler = new ExpressionCompiler();
   
   let verts = [];
   let norms = [];
   let indices = [];
   
   // Variable for theta
   let theta = { value: 0 };
   
   // Compile r(theta) expression
   let exprR = compiler.compile(formula);
   exprR.bind("theta", () => theta.value);
   
   const zTip = -1.0;
   
   let addVertex = (x, y, z, nx, ny, nz) => {
      let idx = verts.length / 3;
      verts.push(x, y, z);
      norms.push(nx, ny, nz);
      return idx;
   };
   
   // Add tip vertex (index 0)
   verts.push(0, 0, zTip);
   norms.push(0, 0, 0);
   let tipIndex = 0;
   
   // Build first ring (base) with nsectors+1 vertices
   let baseRing = [];
   for (let i = 0; i <= nsectors; i++) {
      theta.value = domainStart + (domainEnd - domainStart) * i / nsectors;
      
      let r = exprR.eval();
      let x = exprR.cylX(theta.value) / nslices;
      let y = exprR.cylY(theta.value) / nslices;
      
      let cosT = Math.cos(theta.value);
      let sinT = Math.sin(theta.value);
      
      // Approximate normal (simplified)
      let nx = r * cosT;
      let ny = r * sinT;
      let nz = zTip;
      
      baseRing.push(addVertex(x, y, 0, nx, ny, nz));
   }
   
   // Generate tip triangles (fan from tip to base ring)
   for (let i = 0; i < nsectors; i++) {
      indices.push(tipIndex, baseRing[i], baseRing[i + 1]);
   }
   
   // Build remaining rings and quads for slices
   if (nslices > 1) {
      let prevRing = baseRing;
      
      for (let h = 1; h < nslices; h++) {
         let h2n = (h + 1) / nslices;
         let currRing = [];
         
         for (let i = 0; i <= nsectors; i++) {
            theta.value = domainStart + (domainEnd - domainStart) * i / nsectors;
            
            let r = exprR.eval();
            let x = exprR.cylX(theta.value) * h2n;
            let y = exprR.cylY(theta.value) * h2n;
            
            let cosT = Math.cos(theta.value);
            let sinT = Math.sin(theta.value);
            let nx = r * cosT;
            let ny = r * sinT;
            let nz = -1;
            
            // Normalize
            let len = Math.sqrt(nx*nx + ny*ny + nz*nz);
            if (len > 0.0001) {
               nx /= len; ny /= len; nz /= len;
            }
            
            currRing.push(addVertex(x, y, h2n, nx, ny, nz));
         }
         
         // Generate quads between prevRing and currRing
         for (let i = 0; i < nsectors; i++) {
            let v00 = prevRing[i];
            let v01 = prevRing[i + 1];
            let v10 = currRing[i];
            let v11 = currRing[i + 1];
            
            // Triangle 1
            indices.push(v00, v10, v01);
            // Triangle 2
            indices.push(v01, v10, v11);
         }
         
         prevRing = currRing;
      }
   }
   
   return { verts, norms, indices };
}

//========================================
// buildConePolar - creates flat (non-indexed) cone geometry
//========================================
function buildConePolar(formula, domainStart = 0, domainEnd = 2 * Math.PI, nsectors = 20, nslices = 3) {
   let indexed = buildConePolarIndexed(formula, domainStart, domainEnd, nsectors, nslices);
   
   let verts = [];
   let norms = [];
   
   for (let idx of indexed.indices) {
      let offset = idx * 3;
      verts.push(indexed.verts[offset], indexed.verts[offset + 1], indexed.verts[offset + 2]);
      norms.push(indexed.norms[offset], indexed.norms[offset + 1], indexed.norms[offset + 2]);
   }
   
   return { verts, norms };
}