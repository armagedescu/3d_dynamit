"use strict";

//this is meant to generate shaders dynamically
//to be used in conjunction with:
//                                glmath.js
//                                glcanvas.js

//const WebGL2S = WebGL2RenderingContext; //create alias for WebGL2RenderingContext, defined in glcanvas.js
//wrapper to gl.ARRAY_BUFFER
class GlArrayBuffer {
   #attribLocation = 0;
   #buffer = null; //can be overwritten by this build/bufferData/withData
   #name   = null;
   constructor (gl, name, attribLocation, data) {
      this.gl                 = gl;
      this.#buffer            = data;
      this.#attribLocation    = attribLocation;
      this.bufferId           = null;
      this.glDrawType         = WebGL2S.STATIC_DRAW;
      this.#name              = name; //name of buffer in shader
   }
   withDrawType(glDrawType) {this.glDrawType = glDrawType; return this;}
   withData    (data)       {this.#buffer    = data;       return this;}
   build (data) {
      let gl = this.gl;
      this.bufferId = gl.createBuffer ();
      this.bindBuffer ();
      if (data) this.#buffer = data;
      if (this.#buffer) //bufferData can be called latter
         this.bufferData (this.#buffer);
      return this.bufferId;
   }

   static createAndInitBuffer (gl, location, name, data, arrayType, dimension) {
      let arrayBuffer = new GlArrayBuffer (gl, name, location);
      arrayBuffer.build  ();
      arrayBuffer.attrib (dimension, gl.FLOAT);
      arrayBuffer.bufferData (data, arrayType);
      return arrayBuffer;
   }
   //sets data only. 
   bufferData(data, arrayType = Float32Array) {
      if (!data) return; //this does not require data, it can be set any later
      let gl = this.gl;
      if(data) this.#buffer = ArrayUtil.toArrayType(data, arrayType);
      this.bindBuffer ();
      gl.bufferData   (gl.ARRAY_BUFFER, this.#buffer, this.glDrawType);
   }
   get length () {return this.#buffer.length;}
   get count  () {return this.#buffer.length / this.dimension;}
   get dimensionOk    () {if (this.#buffer.length % this.dimension) return false; return true;}
   get dimensionFail  () {!this.dimensionOk;}
   //dimension 1..4 (vec1..vec4), type FLOAT/INT..., normalized 0..1/-1..1 for unsigned/signed BYTE SHORT no effect on float
   //stride: start of struct, offset address relative to and in struct
   attrib (dimension, type, normalized = false, stride = 0, offset = 0)
   {
      console.assert (this.#attribLocation != null, "vertex location must exist!");
      let gl = this.gl;
      this.bindBuffer ();
      this.vertexAttribPointer   (dimension, type, normalized, stride, offset);
      gl.enableVertexAttribArray (this.#attribLocation); // auto enable
      return this.#attribLocation;
   }

   //base name and declaration for in buffers and shared varyings
   get name              () { return   this.#name; } //can be set only in sonstructor
   get nameVary          () { return `${this.name}Vary`; }
   get decl              () { return `vec${this.dimension} ${this.name}`; }
   get declVary          () { return `vec${this.dimension} ${this.nameVary}`; }
   get defaultVaryAssign () { return `${this.nameVary} = ${this.name}`; }; //default pass from VertexShader to FragmentShader
   vec4 (c) {
      if (this.dimension == 4) return this.name;
      let defs = [0, 0, 0, c == null ? 1 : c].slice(this.dimension);
      return `vec4(${this.name}, ${defs.join(", ")})`;
   }
   vec4vary (c) {
      if (this.dimension == 4) return this.nameVary;
      let defs = [0, 0, 0, c == null ? 1 : c].slice(this.dimension);
      return `vec4(${this.nameVary}, ${defs.join(", ")})`;
   }
   vertexAttribPointer (dimension, type, normalized = false, stride = 0, offset = 0) {
      this.dimension = dimension;
      this.type = type;
      this.normalized = normalized;
      this.stride = stride;
      this.offset = offset;
      this.bindBuffer ();
      console.assert  (this.#attribLocation != null);
      this.gl.vertexAttribPointer (this.#attribLocation, dimension, type, normalized, stride, offset);
   }
   bindBuffer  () { let gl = this.gl; gl.bindBuffer(gl.ARRAY_BUFFER, this.bufferId); }
   get attribLocation () {return this.#attribLocation;}
}

//========================================
// StrideAttribute - single attribute in stride layout
//========================================
class StrideAttribute {
   constructor(name, location, size, type, normalized, offset) {
      this.name = name;
      this.location = location;
      this.size = size;           // dimension (1-4)
      this.type = type;           // gl.FLOAT, etc.
      this.normalized = normalized;
      this.offset = offset;       // byte offset within stride
   }
   get decl     () { return `vec${this.size} ${this.name}`; }
   get declVary () { return `vec${this.size} ${this.name}Vary`; }
   get nameVary () { return `${this.name}Vary`; }
   get defaultVaryAssign () { return `${this.nameVary} = ${this.name}`; }
   vec4 (c = 1.0) {
      if (this.size == 4) return this.name;
      let defs = [0, 0, 0, c].slice(this.size);
      return `vec4(${this.name}, ${defs.join(", ")})`;
   }
   vec4vary (c = 1.0) {
      if (this.size == 4) return this.nameVary;
      let defs = [0, 0, 0, c].slice(this.size);
      return `vec4(${this.nameVary}, ${defs.join(", ")})`;
   }
}

//========================================
// StrideLayout - manages interleaved attribute layout
//========================================
class StrideLayout {
   #attributes = [];
   #currentOffset = 0;
   #totalStride = 0;

   static sizeOf(type) {
      switch (type) {
         case WebGL2S.FLOAT:          return 4;
         case WebGL2S.INT:            return 4;
         case WebGL2S.UNSIGNED_INT:   return 4;
         case WebGL2S.SHORT:          return 2;
         case WebGL2S.UNSIGNED_SHORT: return 2;
         case WebGL2S.BYTE:           return 1;
         case WebGL2S.UNSIGNED_BYTE:  return 1;
         default:                     return 4;
      }
   }

   addAttribute(name, location, size, type = WebGL2S.FLOAT, normalized = false) {
      let attr = new StrideAttribute(name, location, size, type, normalized, this.#currentOffset);
      this.#attributes.push(attr);
      this.#currentOffset += size * StrideLayout.sizeOf(type);
      return attr;
   }

   setOffset(bytes) { this.#currentOffset = bytes; }
   setStride(bytes) { this.#totalStride = bytes; }
   finalize()       { this.#totalStride = this.#currentOffset; }

   get attributes   () { return this.#attributes; }
   get stride       () { return this.#totalStride; }
   get currentOffset() { return this.#currentOffset; }

   getAttribute(name) {
      return this.#attributes.find(a => a.name === name) || null;
   }
   hasAttribute(name) {
      return this.getAttribute(name) !== null;
   }
}

class LightDirection {
   //structure: {name:"name",  data:[data],   location: uniform, type: "4f", normalize:true/false};
   #data = null; #name  = null; #location= null; #type= "3f";
   #normalize  = true;
   #const      = true;
   constructor (obj) {
      this.#data      = obj.data;
      this.#name      = obj.name;
      this.#location  = obj.location;
      this.#normalize = obj.normalize;
      this.#const     = obj.const;
      this.#type      = obj.type;

   }
   get location () {if (this.isConst) throw new Error ("const light direction can't have location"); return this.#location;}
   set location (val) {
      if (this.isConst) throw new Error ("const light direction can't have location");
      if (this.#location != null) throw new Error ("Location can be set only once");
      this.#location = val;
   }
   get name       ()    { return  this.#name; }
   set name       (val) { throw   new Error ("name can be set only in constructor");       }
   get data       ()    { return  this.#data; }
   set data       (val) { throw   new Error ("init data can be set only in constructor");  }
   get normalize  ()    { return  this.#normalize; }
   set normalize  (val) { throw   new Error ("normalize can be set only in constructor");  }
   get isConst    ()    { return  this.#const;  }
   get isNotConst ()    { return !this.#const;  }
   get vary       ()    { return  this.isConst ?  null : this; }
}

class GlSet {
   #precision           = "mediump float";
   #vertexBuffer        = null; //in array buffer only
   #normalsBuffer       = null; //in array buffer only
   //TODO: unify color and translation
   #colorsBuffer        = null; //in array buffer
   #constColor          = null;
   #translation         = null; //uniform
   #constTranslation    = null;
   #lightDirection      = null; //new LightDirection({}), unified const and vary
   #indexBuffer         = null; // NEW: index buffer id
   #indexCount          = 0;    // NEW: number of indices
   #indexType           = null; // NEW: gl.UNSIGNED_INT, gl.UNSIGNED_SHORT, gl.UNSIGNED_BYTE
   #primitiveType       = null; // NEW: gl.TRIANGLES, gl.TRIANGLE_FAN, etc.

   constructor () {
      Object.preventExtensions(this); //properties can't be added manually
   }
   //TODO: subject to interference with lights uniforms //{name: "constColor", value: [ 0.0,  0.0,  0.1]}
   requireColor (dvalue = [ 0.7,  0.7,  0.7,  1.0], dname = "constColor")  { 
       if (this.colorsBuffer)        return;
       if (this.constColor)          return;
       ArrayUtil.validateType (dvalue);
       this.constColor = {name: dname, data: dvalue};
   } //TODO: subject to interference with lights uniforms
   //TODO: subject to interference with lights uniforms
   requireLightDirection (dvalue = [0.0, 0.5, 1.0], normalize = true, dname = "lightDirection")  { 
       if (this.lightDirection)      return;
       ArrayUtil.validateType (dvalue);
       this.#lightDirection     = new LightDirection({name: dname, data: dvalue, const: true, normalize: normalize});
   }
   selectColor () {
      if (this.colorsBuffer)      return this.colorsBuffer;
      return this.constColor;
   }
   //getters/setters
   get precision           ()    { return this.#precision;    }
   set precision           (val) { this.#precision = val;     }
   get vertexBuffer        ()    { return this.#vertexBuffer; }
   set vertexBuffer        (arrayBuffer) {
      if(this.#vertexBuffer) throw new Error ("vertexBuffer can be set only once. Call updateVertices* to update data.");
      this.#vertexBuffer = arrayBuffer;
   }
   get colorsBuffer        ()    {return this.#colorsBuffer; }
   set colorsBuffer        (arrayBuffer) {
      if (this.#colorsBuffer) throw new Error ("colorsBuffer can be set only once. Call updateColors* to update data.");
      this.#constColor   = null; //disable const color
      this.#colorsBuffer = arrayBuffer;
   }
   get normalsBuffer       ()     { return this.#normalsBuffer; }
   set normalsBuffer       (arrayBuffer) { this.#normalsBuffer = arrayBuffer; }
   get translation         ()     { return this.#translation;   }
   set translation         (data) {
      if (this.#translation) throw new Error ("translation can be set only once, call translate* to update translation");
      this.#translation      = data; 
      this.#constTranslation = null;
   }
   
   get lightDirectionVary  ()    {if (!this.#lightDirection) return null; return this.#lightDirection.vary;}
   get lightDirection      ()    {return this.#lightDirection;}
   set lightDirection      (val) {
      if (this.lightDirection) throw new Error ("light direction can be added only once. Call lightDirection1/2/3/4fv to update light direction");
      ArrayUtil.validateType (val.data);
      this.#lightDirection = new LightDirection (val);
   }
   get constLightDirection ()    {
      if (!this.#lightDirection || this.#lightDirection.isNotConst) return null;
      return this.#lightDirection;
   };
   set constLightDirection (val) {
      if (this.#lightDirection) throw new Error ("Const light direction can be set only once");
      throw new Error ("Use lightDirection with {const:false}");
   }
   get constColor          ()    { return this.#constColor; }
   set constColor          (val) {
      if (this.#colorsBuffer) throw new Error ("Const color can't be set when color buffer is used");
      if (this.#constColor)   throw new Error ("Const color can be set only once");
      ArrayUtil.validateType (val.data); //throws error on fail
      this.#constColor = val;
   }
   get constTranslation    ()    { return this.#constTranslation; }
   set constTranslation    (val) { this.#constTranslation = val;  }

   // NEW: Index buffer getters/setters
   get indexBuffer   () { return this.#indexBuffer; }
   set indexBuffer   (val) { this.#indexBuffer = val; }
   get indexCount    () { return this.#indexCount; }
   set indexCount    (val) { this.#indexCount = val; }
   get indexType     () { return this.#indexType; }
   set indexType     (val) { this.#indexType = val; }
   get primitiveType () { return this.#primitiveType; }
   set primitiveType (val) { this.#primitiveType = val; }
}

//========================================
// Shader Strategy - Compositional Approach
//========================================
class ShaderStrategy {
   vsBuilder     = new VertexShaderGLSL300SourceBuilder   ();
   fsBuilder     = new FragmentShaderGLSL300SourceBuilder ();
   strideLayout  = null;
   
   constructor (vsSource = null, fsSource = null, glSet, strideLayout = null) {
      if (!vsSource && !fsSource) {} //no source to be set
      else if ( vsSource &&  fsSource) {
         this.vsBuilder.withSource (vsSource);
         this.fsBuilder.withSource (fsSource);
      } else //it can't be half automatic
         throw new Error ("Sources provided must be both or none");
      this.glSet = glSet;
      this.strideLayout = strideLayout;
   }

   setRequirements () {
      let glSet = this.glSet;
      glSet.requireColor ();
      
      let hasNormals = glSet.normalsBuffer != null;
      if (this.strideLayout)
         hasNormals = hasNormals || this.strideLayout.hasAttribute("normal");
      
      if (hasNormals) glSet.requireLightDirection();
   }
   
   build () {
      // For stride mode, we don't require traditional vertex buffer
      if (!this.strideLayout && !this.glSet.vertexBuffer)
         throw new Error ("Vertex buffer is required");
      let [vsBuilder, fsBuilder] = [ this.vsBuilder, this.fsBuilder ];
      if ( vsBuilder.source &&  fsBuilder.source)
         return {vertexShaderSource : vsBuilder.source, fragmentShaderSource : fsBuilder.source};
      if ( vsBuilder.source || fsBuilder.source)
         throw new Error ("Sources provided must be both not null, or both null");
      this.setRequirements ();
      return this.buildCompositional ();
   }

   //========================================
   // Compositional Building - No Strategy Codes!
   //========================================
   
   buildCompositional () {
      let [vsBuilder, fsBuilder] = [ this.vsBuilder, this.fsBuilder ];
      for (let builder of [vsBuilder, fsBuilder]) builder.reset();

      if (this.strideLayout && this.strideLayout.attributes.length > 0)
         this.addStrideDeclarations();
      else
         this.addDeclarations();
      
      this.composeVertexMain();
      this.composeFragmentMain();

      return {vertexShaderSource : vsBuilder.buildCandidate(), fragmentShaderSource : fsBuilder.buildCandidate()};
   }

   addStrideDeclarations () {
      let [vsBuilder, fsBuilder] = [ this.vsBuilder, this.fsBuilder];
      let glSet = this.glSet;
      
      // Fragment shader precision and output
      fsBuilder.addHead (`precision ${glSet.precision};`);
      fsBuilder.addHead (`out vec4 fragColor;`);
      
      // Add stride attributes
      for (let attr of this.strideLayout.attributes) {
         vsBuilder.addHead (`layout (location = ${attr.location}) in ${attr.decl};`);
         
         // Add varyings for normals and colors
         if (attr.name === "normal" || attr.name === "color") {
            vsBuilder.addHead (`out ${attr.declVary};`);
            fsBuilder.addHead (`in  ${attr.declVary};`);
         }
      }
      
      // Const color (optional)
      if (glSet.constColor) { 
         let colorStr = ArrayUtil.buildFloatStrings (glSet.constColor.data).join (", ");
         fsBuilder.addHead (`const vec4 ${glSet.constColor.name} = vec4 (${colorStr});`);
      }
      
      // Light direction (optional)
      if (glSet.lightDirection) {
         let lightDirection = glSet.lightDirection;
         if (lightDirection.isConst) {
            let strOut = ArrayUtil.buildFloatStrings(lightDirection.data).join(", ");
            fsBuilder.addHead (`const vec3 ${lightDirection.name} = vec3(${strOut});`);
         } else
            fsBuilder.addHead (`uniform vec3 ${lightDirection.name};`);
      }
      
      // Uniform translation (optional)
      if (glSet.translation) {
         vsBuilder.addHead (`uniform vec4 ${glSet.translation.name};`);
      }
   }

   addDeclarations () {
      let [vsBuilder, fsBuilder] = [ this.vsBuilder, this.fsBuilder];
      let glSet = this.glSet;
      
      // Fragment shader precision and output
      fsBuilder.addHead (`precision ${glSet.precision};`);
      fsBuilder.addHead (`out vec4 fragColor;`);
      
      // Vertex buffer (required)
      vsBuilder.addHead (`layout (location = ${glSet.vertexBuffer.attribLocation}) in ${glSet.vertexBuffer.decl};`);
      
      // Const color (optional)
      if (glSet.constColor) { 
         let colorStr = ArrayUtil.buildFloatStrings (glSet.constColor.data).join (", ");
         fsBuilder.addHead (`const vec4 ${glSet.constColor.name} = vec4 (${colorStr});`);
      }
      
      // Normals buffer (optional)
      if (glSet.normalsBuffer) {
         vsBuilder.addHead (`layout (location = ${glSet.normalsBuffer.attribLocation}) in ${glSet.normalsBuffer.decl};`);
         vsBuilder.addHead (`out ${glSet.normalsBuffer.declVary};`);
         fsBuilder.addHead (`in  ${glSet.normalsBuffer.declVary};`);
      }
      
      // Colors buffer (optional)
      if (glSet.colorsBuffer) {
         vsBuilder.addHead (`layout (location = ${glSet.colorsBuffer.attribLocation}) in ${glSet.colorsBuffer.decl};`);
         vsBuilder.addHead (`out ${glSet.colorsBuffer.declVary};`);
         fsBuilder.addHead (`in  ${glSet.colorsBuffer.declVary};`);         
      }
      
      // Light direction (optional)
      if (glSet.lightDirection) {
         let lightDirection = glSet.lightDirection;
         if (lightDirection.isConst) {
            let strOut = ArrayUtil.buildFloatStrings(lightDirection.data).join(", ");
            fsBuilder.addHead (`const vec3 ${lightDirection.name} = vec3(${strOut});`);
         } else
            fsBuilder.addHead (`uniform vec3 ${lightDirection.name};`);
      }
      
      // Uniform translation (optional)
      if (glSet.translation) {
         vsBuilder.addHead (`uniform vec4 ${glSet.translation.name};`);
      }
   }

   //========================================
   // Compose gl_Position from pieces
   //========================================
   buildPositionExpression () {
      let glSet = this.glSet;
      let pos;
      
      if (this.strideLayout && this.strideLayout.hasAttribute("vertex")) {
         let attr = this.strideLayout.getAttribute("vertex");
         pos = attr.vec4();
      } else if (glSet.vertexBuffer) {
         pos = glSet.vertexBuffer.vec4();
      } else {
         pos = "vec4(0.0, 0.0, 0.0, 1.0)";
      }

      // Add const translation if present
      if (glSet.constTranslation) {
         let strTrans = ArrayUtil.buildFloatStrings(glSet.constTranslation).join(", ");
         pos = `vec4(${strTrans}) + ${pos}`;
      }
      
      // Add uniform translation if present
      if (glSet.translation)
         pos = `vec4(${glSet.translation.name}) + ${pos}`;

      return pos;
   }

   composeVertexMain () {
      let vsBuilder = this.vsBuilder;
      let glSet = this.glSet;

      // Position (always required)
      vsBuilder.addMain (`   gl_Position = ${this.buildPositionExpression()};`);

      if (this.strideLayout) {
         // Stride mode: pass attributes via varyings
         if (this.strideLayout.hasAttribute("normal"))
            vsBuilder.addMain (`   normalVary = normal;`);
         if (this.strideLayout.hasAttribute("color"))
            vsBuilder.addMain (`   colorVary = color;`);
      } else {
         // Separate buffer mode
         if (glSet.normalsBuffer)
            vsBuilder.addMain (`   ${glSet.normalsBuffer.defaultVaryAssign};`);
         if (glSet.colorsBuffer)
            vsBuilder.addMain (`   ${glSet.colorsBuffer.defaultVaryAssign};`);
      }
   }

   //========================================
   // Compose fragColor from pieces
   //========================================
   buildColorExpression () {
      let glSet = this.glSet;
      
      // Stride mode: check for color attribute
      if (this.strideLayout && this.strideLayout.hasAttribute("color")) {
         let attr = this.strideLayout.getAttribute("color");
         return attr.vec4vary();
      }
      
      // Priority: per-vertex colors > const color
      if (glSet.colorsBuffer)
         return glSet.colorsBuffer.vec4vary();
      
      if (glSet.constColor)
         return glSet.constColor.name;

      return "vec4(1.0, 1.0, 1.0, 1.0)"; // fallback white
   }

   buildLightingFactor () {
      let glSet = this.glSet;
      
      let hasNormals = false;
      let normalVar = null;
      
      if (this.strideLayout && this.strideLayout.hasAttribute("normal")) {
         hasNormals = true;
         normalVar = "normalVary";
      } else if (glSet.normalsBuffer) {
         hasNormals = true;
         normalVar = glSet.normalsBuffer.nameVary;
      }
      
      if (!hasNormals || !glSet.lightDirection)
         return null; // No lighting

      let lightDirection = glSet.lightDirection;
      
      if (lightDirection.normalize)
         return `-dot(normalize(${lightDirection.name}), normalize(${normalVar}))`;
      else
         return `-dot(${lightDirection.name}, ${normalVar})`;
   }

   composeFragmentMain () {
      let fsBuilder = this.fsBuilder;
      
      let colorExpr = this.buildColorExpression();
      let lightingFactor = this.buildLightingFactor();

      if (lightingFactor) {
         // Apply lighting: color.rgb * lightingFactor
         fsBuilder.addMain (`   float prod = ${lightingFactor};`);
         fsBuilder.addMain (`   fragColor = vec4(${colorExpr}.rgb * prod, 1.0);`);
      } else {
         // No lighting, just output color
         fsBuilder.addMain (`   fragColor = vec4(${colorExpr});`);
      }
   }

   get snapshot          () { return this.build(); }
   get snapshotCandidate () { return this.buildCompositional(); }
}

class ShaderGLSL300SourceBuilder {
   #headStart    = "#version 300 es";
   #headDefs     = [];
   #mainDefs     = [];
   #mainStart    = "void main(void)\n" +
                   "{";
   #mainEnd      = "}\n";
   #source = null;

   constructor () {}
   withSource (source) {this.#source = source; return this;}

   addHead     (str) { this.#headDefs.push    (... ArrayUtil.toArray(str)); }
   addMain     (str) { this.#mainDefs.push    (... ArrayUtil.toArray(str)); }
   insertHead  (str) { this.#headDefs.unshift (... ArrayUtil.toArray(str)); }
   insertMain  (str) { this.#mainDefs.unshift (... ArrayUtil.toArray(str)); }

   build () {
      if (this.#source) return this.#source; //if source is explicitly assigned
      return this.buildCandidate ();
   }
   buildCandidate () {
      let source = "";
      source += this.#headStart;
      source += "\n" + this.#headDefs.join ("\n");
      source += "\n" + this.#mainStart;
      source += "\n" + this.#mainDefs.join ("\n");
      source += "\n" + this.#mainEnd;
      this.reset(); //to be able to get intermediary snapthots, build alwais starts from scratch
      return source;
   }

   set source (val) {this.#source = val;}
   get source () {return this.#source;}
   get snapshot          () {return this.build();}
   get snapshotCandidate () {return this.buildCandidate ();}
   reset() {
      this.#headDefs = [];
      this.#mainDefs = [];
   }
}

//Same builder, just different classes, yet identic
class VertexShaderGLSL300SourceBuilder   extends ShaderGLSL300SourceBuilder {   constructor () { super(); }   }
class FragmentShaderGLSL300SourceBuilder extends ShaderGLSL300SourceBuilder {   constructor () { super(); }   }

class ShaderProgramBuilder {
   #gl = null;
   #programExternal = null;
   #shaderSources   = null;
   #uniforms        = [];
   #program         = null;
   get gl () {return this.#gl;}
   constructor (gl) { this.#gl = gl; }
   withProgramExternal (programExternal) { this.#programExternal = programExternal; return this; }
   //required format{vertexShaderSource: "vsSource", fragmentShaderSource: "vsSource"}
   withShaderSources (shaderSources) {
      if (this.#programExternal != null) return this; 
      this.#shaderSources = shaderSources;
      return this;
   }
   //required format [{location: null, name: "name" ...}, ...]
   withUniforms (uniforms) {
      this.#uniforms = [... this.#uniforms, ...ArrayUtil.toArray (uniforms)]; //accumulate
      return this;
   }
   //build program and get uniforms locations
   buildProgram () {
      let gl = this.gl;
      if (this.#program != null) return; //everything is already done
      this.#buildInternal ();
      this.#getUniformLocations ();
      gl.useProgram (null);
      return this.#program;
   }
   #buildInternal () {
      let gl = this.gl;
      this.#program = this.#programExternal;
      if (this.#program != null) return this.#program;
      let cprogram = new GlProgram (gl);
      let shaderSources = this.#shaderSources;
      
      cprogram.addVertexShader   (shaderSources.vertexShaderSource);
      cprogram.addFragmentShader (shaderSources.fragmentShaderSource);

      this.#program = cprogram.linkProgram ();
      return this.#program;
   }
   get uniforms () {if (this.#uniforms && this.#uniforms.length) return this.#uniforms; return null;}
   #getUniformLocations () {
      if (!this.uniforms) return; //nothing to do
      let gl = this.gl;
      gl.useProgram (this.#program);
      for (let uniform of this.#uniforms)
         uniform.location = gl.getUniformLocation(this.#program, uniform.name);
      this.#uniforms = null; //never deal with it again
   }
}

// this wraps OpenGL VAO, VBO, Uniforms
class Dynamit
{
   locationCount = 0;

   #strategy        = null;
   glSet            = new GlSet();
   #uniforms        = [];
   #program         = null;
   #programExternal = null;
   #canvas          = null;
   //chain
   #prev            = null;
   #next            = null;
   
   // Stride support
   #strideMode      = false;
   #strideLayout    = new StrideLayout();
   #strideBuffer    = null;
   #vertexCount     = 0;
   
   get prev       () { return  this.#prev; } 
   get next       () { return  this.#next; } 
   get first      () { return  this.#prev ? this.#prev.first : this; }
   get last       () { return  this.#next ? this.#next.last  : this; }
   get isLast     () { return !this.#next; }
   get isNotLast  () { return  this.#next; }

   get strategyAuto() {
      if (this.#strategy) return this.#strategy;
      if (this.#strideMode)
         this.#strategy = new ShaderStrategy(null, null, this.glSet, this.#strideLayout);
      else
         this.#strategy = new ShaderStrategy(null, null, this.glSet);
      return this.#strategy;
   }

   constructor (obj) {
      this.#obj = obj;
   }
   requireCanvas () {
      if (this.#prev || this.#next) throw new Error ("this is part of VAO chain");
      if (this.canvas) throw new Error ("<canvas> already set");
      if (this.gl)     throw new Error ("WebGL2RenderingContext already set");
      this.#obj = document.createElement ("canvas");
   }
   //complexity of initialization here
   set #obj(obj) {
      if (this.#prev || this.#next) throw new Error ("this is part of VAO chain");
      if (this.canvas) throw new Error ("<canvas> already set");
      if (this.gl)     throw new Error ("WebGL2RenderingContext already set");
      if (!obj) return;
      let  gl  = null;
      if (obj instanceof HTMLCanvasElement) {
         this.#canvas = obj;
         gl = obj.getContext('webgl2');
      }
      if (obj instanceof WebGL2RenderingContext) { gl = obj; }
      if (obj instanceof Dynamit) {
         if (obj.isNotLast) throw new Error ("new VAO can be only appended to last VAO in chain");
         gl = obj.gl;
         this.#prev = obj;
         obj.#next = this;
         this.#canvas = obj.canvas;
         this.#programExternal = obj.programAuto;
         this.locationCount = obj.locationCount;
         // Inherit stride layout from parent
         this.#strideMode = obj.#strideMode;
         this.#strideLayout = obj.#strideLayout;
      }
      this.gl  = gl;
      this.vao = gl.createVertexArray();
   }
   get canvas () {return this.#canvas;}

   withShaderSources (vs, fs) {
      this.#strategy = new ShaderStrategy(vs, fs, this.glSet);
      return this;
   }
   withConstColor    (value = [ 0.0,  0.0,  0.0,  0.1], name = "constColor") {
      this.glSet.constColor = {name: name, data: value};  
      return this;
   }

   withPrecision     (dvalue = "mediump float") { this.glSet.precision = dvalue; return this; }
   withTranslation4f (data = null, name = "translate") {
      this.glSet.translation =  {data:data, name: name, location: null, type: "4f", const:false};
      return this;
   }
   // Keep old name for compatibility
   withTransnation4f (data = null, name = "translate") {
      return this.withTranslation4f(data, name);
   }
   translate4f  (x, y, z, w) { 
      this.useProgram();
      this.gl.uniform4f  (this.glSet.translation.location, x, y, z, w); 
   }
   translate4fv (data) { 
      this.useProgram();
      this.gl.uniform4fv (this.glSet.translation.location, data); 
   }

   withConstLightDirection (data = [0.0, 0.5, 1.0], normalize=true, name = "lightDirection") {
      this.glSet.lightDirection  = {data:data, name: name, type: "3f", normalize:normalize, const:true};
      return this;
   }

   withLightDirection3f (data = null, normalize=true, name = "lightDirection") {
      this.glSet.lightDirection = {data:data, name: name, location: null, type: "3f", normalize:normalize, const:false};
      return this;
   }
   lightDirection3f (x, y, z) { 
      this.useProgram();
      this.gl.uniform3fv (this.glSet.lightDirection.location, ArrayUtil.selectArray(x, y, z)); 
   }

   //========================================
   // Stride support methods
   //========================================
   
   withStride(data, strideBytes) {
      let gl = this.gl;
      this.bindVertexArray();
      
      // If chained and parent has stride layout, reuse it
      if (this.#prev && this.#prev.#strideMode) {
         this.#strideLayout = this.#prev.#strideLayout;
      }
      
      this.#strideLayout.setStride(strideBytes);
      this.#strideMode = true;
      
      // Create stride buffer
      this.#strideBuffer = gl.createBuffer();
      gl.bindBuffer(gl.ARRAY_BUFFER, this.#strideBuffer);
      
      let typedData = ArrayUtil.toArrayType(data, Float32Array);
      gl.bufferData(gl.ARRAY_BUFFER, typedData, gl.STATIC_DRAW);
      
      // Calculate vertex count
      this.#vertexCount = (typedData.byteLength) / strideBytes;
      
      // If chained, apply parent's layout to this buffer
      if (this.#prev && this.#prev.#strideMode) {
         this._applyStrideLayout();
      }
      
      return this;
   }
   
   _applyStrideLayout() {
      let gl = this.gl;
      gl.bindBuffer(gl.ARRAY_BUFFER, this.#strideBuffer);
      
      for (let attr of this.#strideLayout.attributes) {
         gl.vertexAttribPointer(attr.location, attr.size, attr.type, attr.normalized,
                                this.#strideLayout.stride, attr.offset);
         gl.enableVertexAttribArray(attr.location);
      }
   }
   
   withStrideOffset(bytes) {
      this.#strideLayout.setOffset(bytes);
      return this;
   }
   
   #getLocationFor(attribName) {
      // Check if parent has this attribute
      if (this.#prev && this.#prev.#strideLayout.hasAttribute(attribName)) {
         return this.#prev.#strideLayout.getAttribute(attribName).location;
      }
      return this.locationCount++;
   }
   
   #addStrideAttribute(name, size, type = WebGL2S.FLOAT, normalized = false) {
      let gl = this.gl;
      let location = this.#getLocationFor(name);
      let attr = this.#strideLayout.addAttribute(name, location, size, type, normalized);
      
      // Apply attribute pointer
      gl.bindBuffer(gl.ARRAY_BUFFER, this.#strideBuffer);
      gl.vertexAttribPointer(attr.location, attr.size, attr.type, attr.normalized, 
                             this.#strideLayout.stride, attr.offset);
      gl.enableVertexAttribArray(attr.location);
      
      return this;
   }
   
   withStrideVertices(size, normalized = false, type = WebGL2S.FLOAT) {
      return this.#addStrideAttribute("vertex", size, type, normalized);
   }
   
   withStrideNormals(size, normalized = false, type = WebGL2S.FLOAT) {
      return this.#addStrideAttribute("normal", size, type, normalized);
   }
   
   withStrideColors(size, normalized = false, type = WebGL2S.FLOAT) {
      return this.#addStrideAttribute("color", size, type, normalized);
   }
   
   withStrideTexCoords(size, normalized = false, type = WebGL2S.FLOAT) {
      return this.#addStrideAttribute("texCoord", size, type, normalized);
   }

   //don't call directly, called from withVertices1/2/3/4d()
   #withVertices (data, arrayType, dimension) {
      let gl = this.gl, glSet = this.glSet;
      this.bindVertexArray();
      
      // If chained, reuse parent's vertex location
      let location;
      if (this.#prev && this.#prev.glSet.vertexBuffer) {
         location = this.#prev.glSet.vertexBuffer.attribLocation;
      } else {
         location = this.locationCount++;
         // Update location counter in chain
         if (this.#prev) this.#prev.locationCount = this.locationCount;
      }
      
      glSet.vertexBuffer = GlArrayBuffer.createAndInitBuffer (gl, location, "vertex", data, arrayType, dimension);
      return this;
   }
   updateVertices(data, arrayType = Float32Array) {
      let vertexBuffer = this.glSet.vertexBuffer;
      console.assert (vertexBuffer, "vertex buffer not initialized. Call withVertices first");
      console.assert (!(data.length % vertexBuffer.dimension), `vertices length ${data.length} is not ${vertexBuffer.dimension}D`);
      this.bindVertexArray();
      vertexBuffer.bufferData (data, arrayType);
   }
   withVertices1d(value, arrayType = Float32Array) { return this.#withVertices (value, arrayType, 1); }
   withVertices2d(value, arrayType = Float32Array) { return this.#withVertices (value, arrayType, 2); }
   withVertices3d(value, arrayType = Float32Array) { return this.#withVertices (value, arrayType, 3); }
   withVertices4d(value, arrayType = Float32Array) { return this.#withVertices (value, arrayType, 4); }

   //don't call directly, called from withNormals1/2/3/4d
   #withNormals (data, arrayType, dimension) {
      if(this.glSet.normalsBuffer) throw new Error ("withNormals can be called only once. Call updateNormals* instead.");
      let gl = this.gl, glSet = this.glSet;
      this.bindVertexArray();
      
      // If chained, reuse parent's normal location
      let location;
      if (this.#prev && this.#prev.glSet.normalsBuffer) {
         location = this.#prev.glSet.normalsBuffer.attribLocation;
      } else {
         location = this.locationCount++;
         if (this.#prev) this.#prev.locationCount = this.locationCount;
      }
      
      glSet.normalsBuffer = GlArrayBuffer.createAndInitBuffer (gl, location, "normal", data, arrayType, dimension);
      return this;
   }

   // called from withColors1/2/3/4d
   #withColors (data, arrayType, dimension) {
      if(this.glSet.colorsBuffer) throw new Error ("withColors can be called only once. Call updateColors* instead.");
      let gl = this.gl, glSet = this.glSet;
      this.bindVertexArray();
      
      // If chained, reuse parent's color location
      let location;
      if (this.#prev && this.#prev.glSet.colorsBuffer) {
         location = this.#prev.glSet.colorsBuffer.attribLocation;
      } else {
         location = this.locationCount++;
         if (this.#prev) this.#prev.locationCount = this.locationCount;
      }
      
      glSet.colorsBuffer = GlArrayBuffer.createAndInitBuffer (gl, location, "color", data, arrayType, dimension);
      return this;
   }

   updateNormals(data, arrayType = Float32Array) {
      this.bindVertexArray();
      this.glSet.normalsBuffer.bufferData( data, arrayType);
   }

   // called from withColors1/2/3/4d
   updateColors (value, arrayType = Float32Array) {
      let [colorsBuffer, vertexBuffer] = [this.glSet.colorsBuffer, this.glSet.vertexBuffer];
      let gl = this.gl, glSet = this.glSet;
      console.assert (colorsBuffer.dimensionOk, `Color count ${colorsBuffer.length} is not ${colorsBuffer.dimension}D`);
      console.assert (vertexBuffer.count == colorsBuffer.count, `Color count ${colorsBuffer.count}, but vertex count is ${vertexBuffer.count}` );
      this.bindVertexArray();
      glSet.colorsBuffer.bufferData (ArrayUtil.toArrayType (value, arrayType));
   }

   withNormals3d (data, arrayType = Float32Array) { return this.#withNormals (data, arrayType, 3); }
   withNormals4d (data, arrayType = Float32Array) { return this.#withNormals (data, arrayType, 4); }
   withColors3d  (data, arrayType = Float32Array) { return this.#withColors  (data, arrayType, 3); }
   withColors4d  (data, arrayType = Float32Array) { return this.#withColors  (data, arrayType, 4); }

   withConstTranslation (vec4) {
      this.glSet.constTranslation = vec4;
      return this;
   }

   // NEW: Index buffer support
   withIndices(data, arrayType = Uint32Array) {
      let gl = this.gl, glSet = this.glSet;
      this.bindVertexArray();
      
      let indexBuffer = gl.createBuffer();
      gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, indexBuffer);
      
      let typedData = ArrayUtil.toArrayType(data, arrayType);
      gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, typedData, gl.STATIC_DRAW);
      
      glSet.indexBuffer = indexBuffer;
      glSet.indexCount = typedData.length;
      
      // Determine index type from array type
      if (arrayType === Uint32Array)      glSet.indexType = gl.UNSIGNED_INT;
      else if (arrayType === Uint16Array) glSet.indexType = gl.UNSIGNED_SHORT;
      else if (arrayType === Uint8Array)  glSet.indexType = gl.UNSIGNED_BYTE;
      else                                glSet.indexType = gl.UNSIGNED_INT;
      
      return this;
   }

   // NEW: Primitive type per VAO
   withPrimitive(primitiveType) {
      this.glSet.primitiveType = primitiveType;
      return this;
   }

   //build program and get uniforms locations
   buildProgram () {
      let gl = this.gl, glSet = this.glSet;
      if (this.#program != null) return; //everything is already done
      let programBuilder = new ShaderProgramBuilder (gl);
      if (this.#programExternal)
         programBuilder.withProgramExternal (this.#programExternal);
      else
         programBuilder.withShaderSources (this.strategyAuto.build ());

      programBuilder.withUniforms ([glSet.translation, glSet.lightDirectionVary, ... this.#uniforms].filter(a => a));

      this.#program  = programBuilder.buildProgram ();
      this.#uniforms = null;
      return this.#program;
   }

   logStrategyShaders (msg = null) {
      let shaderSources = this.strategyAuto.build ();
      if (msg) console.log (`${msg}`);
      console.log (`${shaderSources.vertexShaderSource}\n${shaderSources.fragmentShaderSource}`);
   }

   //triggers program build if none
   get programAuto () { if (this.#program == null) this.buildProgram (); return this.#program; }

   //bind context
   bindVertexArray () { this.gl.bindVertexArray (this.vao); }
   useProgram      () { this.gl.useProgram (this.programAuto); }
   bindAll         () { this.useProgram (); this.bindVertexArray(); }
   //bind context aliases
   bindVAO = this.bindVertexArray;
   bind    = this.bindAll;

   // MODIFIED: Drawing with chain support
   drawTriangleFan (start = 0) { this._drawArraysChain (this.gl.TRIANGLE_FAN, start); }
   drawTriangles   (start = 0) { this._drawArraysChain (this.gl.TRIANGLES,    start); }
   
   _drawArraysChain(defaultPrimitive, start = 0) {
      let gl = this.gl, glSet = this.glSet;
      this.bindAll();
      
      let primitive = glSet.primitiveType || defaultPrimitive;
      let count = this.#strideMode ? this.#vertexCount : (glSet.vertexBuffer ? glSet.vertexBuffer.count : 0);
      
      if (count > 0) {
         gl.drawArrays(primitive, start, count);
      }
      
      // Draw next in chain
      if (this.next) {
         this.next._drawArraysChain(defaultPrimitive, start);
      }
   }

   drawArrays (drawType, start, vertsCount) {
      let gl = this.gl;
      this.bindAll();
      gl.drawArrays (drawType, start, vertsCount);
   }

   // Indexed drawing
   drawTrianglesIndexed() {
      this._drawIndexedChain(this.gl.TRIANGLES);
   }

   _drawIndexedChain(defaultPrimitive) {
      let gl = this.gl, glSet = this.glSet;
      this.bindAll();
      
      let primitive = glSet.primitiveType || defaultPrimitive;
      
      gl.drawElements(primitive, glSet.indexCount, glSet.indexType, 0);
      
      // Draw next in chain
      if (this.next) {
         this.next._drawIndexedChain(defaultPrimitive);
      }
   }

   drawElements(drawType, count, type, offset = 0) {
      let gl = this.gl;
      this.bindAll();
      gl.drawElements(drawType, count, type, offset);
   }
}