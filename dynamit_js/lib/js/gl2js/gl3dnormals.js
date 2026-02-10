"use strict";

//========================================
// NormalsHighlighter - Visualizes normals as colored lines
//========================================
class NormalsHighlighter {
   #gl = null;
   #program = null;
   #vao = null;
   #vbo = null;
   #endpointVbo = null;
   #transformLoc = null;
   #colorStartLoc = null;
   #colorEndLoc = null;
   #lineCount = 0;
   
   #normalLength = 0.1;
   #colorStart = [1.0, 0.0, 1.0]; // magenta
   #colorEnd = [1.0, 1.0, 0.0];   // yellow
   
   constructor(gl, length = 0.1) {
      this.#gl = gl;
      this.#normalLength = length;
      this.#initShaders();
   }
   
   #initShaders() {
      const gl = this.#gl;
      
      const vsSource = `#version 300 es
         layout(location = 0) in vec3 aPos;
         layout(location = 1) in uint aEndpoint;
         out vec3 vColor;
         uniform mat4 uTransform;
         uniform vec3 uColorStart;
         uniform vec3 uColorEnd;
         void main() {
            gl_Position = uTransform * vec4(aPos, 1.0);
            vColor = (aEndpoint == 0u) ? uColorStart : uColorEnd;
         }
      `;
      
      const fsSource = `#version 300 es
         precision mediump float;
         in vec3 vColor;
         out vec4 FragColor;
         void main() {
            FragColor = vec4(vColor, 1.0);
         }
      `;
      
      const vs = gl.createShader(gl.VERTEX_SHADER);
      gl.shaderSource(vs, vsSource);
      gl.compileShader(vs);
      if (!gl.getShaderParameter(vs, gl.COMPILE_STATUS)) {
         console.error("NormalsHighlighter VS error:", gl.getShaderInfoLog(vs));
      }
      
      const fs = gl.createShader(gl.FRAGMENT_SHADER);
      gl.shaderSource(fs, fsSource);
      gl.compileShader(fs);
      if (!gl.getShaderParameter(fs, gl.COMPILE_STATUS)) {
         console.error("NormalsHighlighter FS error:", gl.getShaderInfoLog(fs));
      }
      
      this.#program = gl.createProgram();
      gl.attachShader(this.#program, vs);
      gl.attachShader(this.#program, fs);
      gl.linkProgram(this.#program);
      if (!gl.getProgramParameter(this.#program, gl.LINK_STATUS)) {
         console.error("NormalsHighlighter link error:", gl.getProgramInfoLog(this.#program));
      }
      
      gl.deleteShader(vs);
      gl.deleteShader(fs);
      
      this.#transformLoc = gl.getUniformLocation(this.#program, "uTransform");
      this.#colorStartLoc = gl.getUniformLocation(this.#program, "uColorStart");
      this.#colorEndLoc = gl.getUniformLocation(this.#program, "uColorEnd");
   }
   
   withLength(length) {
      this.#normalLength = length;
      return this;
   }
   
   withColorStart(r, g, b) {
      this.#colorStart = [r, g, b];
      return this;
   }
   
   withColorEnd(r, g, b) {
      this.#colorEnd = [r, g, b];
      return this;
   }
   
   build(verts, norms) {
      const gl = this.#gl;
      const length = this.#normalLength;
      
      let lineData = [];
      let endpointData = [];
      
      for (let i = 0; i < verts.length; i += 3) {
         let vx = verts[i], vy = verts[i+1], vz = verts[i+2];
         let nx = norms[i], ny = norms[i+1], nz = norms[i+2];
         
         // Start point
         lineData.push(vx, vy, vz);
         endpointData.push(0);
         
         // End point
         lineData.push(vx + nx * length, vy + ny * length, vz + nz * length);
         endpointData.push(1);
      }
      
      this.#lineCount = lineData.length / 3;
      
      // Create VAO
      this.#vao = gl.createVertexArray();
      gl.bindVertexArray(this.#vao);
      
      // Position buffer
      this.#vbo = gl.createBuffer();
      gl.bindBuffer(gl.ARRAY_BUFFER, this.#vbo);
      gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(lineData), gl.STATIC_DRAW);
      gl.vertexAttribPointer(0, 3, gl.FLOAT, false, 0, 0);
      gl.enableVertexAttribArray(0);
      
      // Endpoint buffer
      this.#endpointVbo = gl.createBuffer();
      gl.bindBuffer(gl.ARRAY_BUFFER, this.#endpointVbo);
      gl.bufferData(gl.ARRAY_BUFFER, new Uint8Array(endpointData), gl.STATIC_DRAW);
      gl.vertexAttribIPointer(1, 1, gl.UNSIGNED_BYTE, 0, 0);
      gl.enableVertexAttribArray(1);
      
      gl.bindVertexArray(null);
      
      return this;
   }
   
   draw(transform) {
      const gl = this.#gl;
      
      gl.useProgram(this.#program);
      gl.uniformMatrix4fv(this.#transformLoc, false, transform);
      gl.uniform3fv(this.#colorStartLoc, this.#colorStart);
      gl.uniform3fv(this.#colorEndLoc, this.#colorEnd);
      
      gl.bindVertexArray(this.#vao);
      gl.drawArrays(gl.LINES, 0, this.#lineCount);
      gl.bindVertexArray(null);
   }
   
   dispose() {
      const gl = this.#gl;
      if (this.#vao) gl.deleteVertexArray(this.#vao);
      if (this.#vbo) gl.deleteBuffer(this.#vbo);
      if (this.#endpointVbo) gl.deleteBuffer(this.#endpointVbo);
      if (this.#program) gl.deleteProgram(this.#program);
   }
}