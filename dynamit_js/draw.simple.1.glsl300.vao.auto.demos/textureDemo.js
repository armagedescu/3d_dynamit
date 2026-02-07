{
"use strict";
let canvas = document.currentScript.parentElement;

let glmain = () => {
   let gl = canvas.getContext('webgl2');
   
   // Load texture
   let textureId = loadTexture(gl, "bitmaps/crate.jpg");
   
   // Build geometry with texture coordinates using polar builder
   let geometry = Builder.polar()
      .formula("1")
      .domain(0, 2 * Math.PI)
      .sectors(20)
      .slices(10)
      .buildConeIndexed();
   
   let shape = new Dynamit(canvas)
      .withVertices3d(geometry.verts)
      .withNormals3d(geometry.norms)
      .withTexCoords2d(geometry.texCoords)
      .withIndices(geometry.indices, Uint32Array)
      .withTexture(textureId, 0, "diffuseMap")
      .withConstLightDirection([-0.577, -0.577, 0.577])
      .withTransformMatrix4f();
   
   shape.logStrategyShaders("Texture Demo:");
   
   gl.enable(gl.DEPTH_TEST);
   gl.clearColor(0.2, 0.2, 0.3, 1.0);
   
   let angle = 0;
   let animate = (time) => {
      angle += 0.01;
      
      // Create rotation matrix around Y axis
      let cos = Math.cos(angle), sin = Math.sin(angle);
      let matrix = [
         cos, 0, sin, 0,
         0, 1, 0, 0,
        -sin, 0, cos, 0,
         0, 0, 0, 1
      ];
      
      gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
      
      shape.transformMatrix4f(matrix);
      shape.drawTrianglesIndexed();
      
      requestAnimationFrame(animate);
   };
   requestAnimationFrame(animate);
};

// Simple texture loader
function loadTexture(gl, url) {
   let texture = gl.createTexture();
   gl.bindTexture(gl.TEXTURE_2D, texture);
   
   // Placeholder magenta pixel until image loads
   gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, 1, 1, 0, gl.RGBA, gl.UNSIGNED_BYTE,
                 new Uint8Array([255, 0, 255, 255]));
   
   let image = new Image();
   image.onload = () => {
      gl.bindTexture(gl.TEXTURE_2D, texture);
      gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, image);
      gl.generateMipmap(gl.TEXTURE_2D);
      gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR_MIPMAP_LINEAR);
      gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
   };
   image.src = url;
   
   return texture;
}

document.addEventListener('DOMContentLoaded', glmain);
}