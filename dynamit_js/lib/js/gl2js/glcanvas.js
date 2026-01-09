"use strict";

//class GlShader                       Shader Module Compiler API
//class GlBuffer                       Basic Wrapper for WebGL2 Buffer
//class GlApi                          Basic Buffer and Uniforms API
//class GlProgram  extends GlApi       Shader Program Linker API
//class GlVAObject extends GlApi       Basic VAO, semi automatic draw,
//                                         central, interchangeable with GlCanvas
//                                         encapsulates if can be done:
//                                                  automatic GlCanvas
//                                                  this lib class GlProgram
//                                                  WebGLProgram
//class GlCanvas                       Extracts info from canvas element and build programs
//                                         central
//class GlVAObjectAsync extends GlApi  Meant to be similar to GlVAObject, but async
//class GlCanvasAsync                  Meant to be similar to GlCanvasAsync, but async
//class GlInfo                         Helper structure, used by GlCanvasAsync
//class GlInfoGrabber                  Helper class to read paths and return GlInfo structures
//class GlTexture2D                    Texture 2D, async
//class GlVideoTexture2D               Video Texture 2D, async

const WebGL2S = WebGL2RenderingContext; //create alias for WebGL2RenderingContext, with shorter name
class GlShader
{
   //type supported values
   //this lib recommended values          "vertex-shader",     "fragment-shader"
   //external twgl compatibility values   "x-shader/x-vertex", "x-shader/x-fragment"
   static shaderMap = new Map(
   [
      ["vertex-shader",        "vertex-shader"],
      ["x-vertex",             "vertex-shader"],
      ["x-shader/x-vertex",    "vertex-shader"],
      ["fragment-shader",      "fragment-shader"],
      ["x-fragment",           "fragment-shader"],
      ["x-shader/x-fragment",  "fragment-shader"]
   ]);
   
   static guessType (str)    //doesn't throw on fail
   {
      if (typeof(str) != 'string' && (str == WebGL2S.VERTEX_SHADER || str == WebGL2S.FRAGMENT_SHADER))
         return str;
      if (!GlShader.shaderMap.has(str)) return null;
      switch ( GlShader.shaderMap.get(str) )
      {
      case "vertex-shader"   :  return WebGL2S.VERTEX_SHADER;
      case "fragment-shader" :  return WebGL2S.FRAGMENT_SHADER;
      }
      return null; // this code is unreachable
   }
   static translateType (str) //throws on fail
   {
      let type = GlShader.guessType (str);
      if (type) return type;
      throw "Unknown shader type: " + str;
   }

   //TODO: not used isAsynk
   isAsynk = false;
   #glType = null;
   constructor (gl, obj, type)
   {
      this.source = "";
      this.gl     = gl;
      this.shader = null; //create public variable //TODO: consider make getter/setter
      this.type   = type; //setter, do nothing if type is nothing
      this.#obj   = obj;  //setter, do nothing if obj is nothing, private
   }

   set #obj(obj)
   {
      if (!obj) return; //do nothing if obj is nothing
      if (typeof obj == "string")          this.#setString        (obj);
      else if (obj instanceof Element)     this.#setScriptElement (obj);
      else if (obj instanceof WebGLShader) this.shader = obj;
   }

   //internal automatic full compile
   #compile ()
   {
      this.shader = this.gl.createShader (this.#glType);
      this.gl.shaderSource  (this.shader, this.source);
      this.compileShader();
   }
   //private:
   #setString (str) //type = go.VERTEX_SHADER/gl.VERTEX_SHADER
   {
      if (!str) {console.log ("no string for shader to compile"); return;}
      this.source = str.trimStart ();
      if (!this.type) return;
      if (this.type.length <= 0) return;
      this.#compile ();
      this.#showResult ();
   }
   #setScriptElement (obj)
   {
      this.script = obj;
      let type = obj.dataset.glType; //same as getAttribute("data-gl-type");
      if (type) { if (type.length > 0) this.type = type; }
      let text = obj.textContent;
      if (!text) text = "";
      text = text.trimStart ();
      this.#setString (text);
   }
   #showResult ()
   {
      let shader = this.shader, gl = this.gl;  //shortcuts
      if (gl.getShaderParameter (shader, gl.COMPILE_STATUS)) return;
      let msg = gl.getShaderInfoLog (shader);
      console.log ("SHADER ERROR: " + msg);
   }

   get type () { return this.#glType; }
   set type (type) //type ="vertex-shader"/"fragment-shader" or gl VERTEX_SHADER/FRAGMENT_SHADER
   {
      if (type) //setter, do nothing if type is nothing
         this.#glType = GlShader.translateType (type); //set to VERTEX_SHADER/VERTEX_SHADER
   }
   compileShader ()
   {
      this.gl.compileShader (this.shader);
      this.#showResult ();
   }
   deleteShader ()
   {
      this.gl.deleteShader (this.shader);
      this.#glType = null;
      this.source = "";
      this.shader = null;
   }
}


class GlBuffer
{
   constructor (gl, type, program, buffer, glDrawType = WebGL2S.STATIC_DRAW) {
      this.gl       = gl;
      this.type     = type;
      this.program  = program;
      this.buffer   = buffer;
      this.location = null;
      this.bufferId = gl.createBuffer ();
      if (this.buffer) this.arrayBuffer (this.buffer, glDrawType);
   }
   withAttribLocation (attribLoc) { this.location = attribLoc; this.attrib = this.attrib2; return this; }

   bindBuffer  () { this.gl.bindBuffer(this.type, this.bufferId); }
   arrayBuffer (buffer, glDrawType = WebGL2S.STATIC_DRAW)
   {
      let gl = this.gl;
      this.bindBuffer ();
      gl.bufferData (this.type, buffer, glDrawType);
      return this.bufferId;
   }
   attrib = (name, size, type, normalized = false, stride = 0, offset = 0) =>
   {
      console.assert (this.location == null, "don't use this path twice, or if location exists");
      let gl = this.gl, program = this.program;
      this.bindBuffer();
      this.location = gl.getAttribLocation (program, name);
      gl.vertexAttribPointer     (this.location, size, type, normalized, stride, offset);
      gl.enableVertexAttribArray (this.location);
      this.attrib = this.attrib2; //attrib function subst
      return this.location;
   }
   //TODO: this interferes with above function
   attrib2 (size, type, normalized = false, stride = 0, offset = 0)
   {
      console.assert (this.location != null, "vertex location must exist!");
      let gl = this.gl;
      this.bindBuffer();
      gl.vertexAttribPointer     (this.location, size, type, normalized, stride, offset);
      gl.enableVertexAttribArray (this.location);
      return this.location;
   }
}

//TODO: review
//base class for GlProgram, GlVAObject, GlVAObjectAsync
class GlApi
{
   constructor (gl) {this.gl = gl;}
   arrayBuffer (buffer, glDrawType = WebGL2S.STATIC_DRAW)
   {
      let gl = this.gl;
      let glBuffer = new GlBuffer (gl, gl.ARRAY_BUFFER, this.program);//, buffer, glDrawType); ok
      glBuffer.arrayBuffer (buffer, glDrawType);
      return glBuffer;
   }
   indexBuffer (buffer, glDrawType = WebGL2S.STATIC_DRAW)
   {
      let gl = this.gl;
      let glBuffer = new GlBuffer(gl, gl.ELEMENT_ARRAY_BUFFER, this.program, buffer, glDrawType);
      //glBuffer.arrayBuffer (buffer, glDrawType); //two parameter above passed
      return glBuffer;
   }
   useProgram () {this.gl.useProgram (this.program);}
   uniformMatrix4fv (name)
   {
      this.useProgram (); //not a buffer
      let locationId = this.gl.getUniformLocation (this.program, name);
      //this.gl.uniformMatrix4fv     (id, size, transpose, value);
      this.gl.enableVertexAttribArray (locationId);
      return locationId;
   }
}

class GlProgram extends GlApi
{
   #shaders = [];
   constructor (gl)
   {
      super(gl);
      this.program = this.gl.createProgram ();
   }
   addVertexShader   (shader) {this.add (shader, this.gl.VERTEX_SHADER);}
   addFragmentShader (shader) {this.add (shader, this.gl.FRAGMENT_SHADER);}
   add (shader, type)
   {
      if (!shader)
      {
         console.log("null shader");
         return;
      }
      if (shader instanceof GlShader)         this.#shaders.push (shader);  //by this lib class GlShader
      else if (shader instanceof WebGLShader) this.#shaders.push (new GlShader (this.gl, shader));       //by compiled shader
      else if (shader instanceof Element)     this.#shaders.push (new GlShader (this.gl, shader, type)); //by element //TODO: reviw type
      else if (typeof shader == "string")     this.#shaders.push (new GlShader (this.gl, shader, type)); //by source
   }
   #showResult ()
   {
      let gl = this.gl, program = this.program; //shortcuts
      if (gl.getProgramParameter (program, gl.LINK_STATUS)) return;
      let msg = gl.getProgramInfoLog (program);
      if (this.name)
         console.log ("PROGRAM ERROR [" + this.name + "]: " + msg);
      else
         console.log ("CPROGRAM ERROR: " + msg);
   }
   linkProgram ()
   {
      let gl = this.gl, program = this.program; //shortcuts
      for (let shader of this.#shaders) gl.attachShader (program, shader.shader);
      gl.linkProgram (program);
      this.#showResult (program);
      for (let shader of this.#shaders) shader.deleteShader ();
      return this.program;
   }
   useProgram (){this.gl.useProgram (this.program);}
}

class GlVAObject extends GlApi
{
   //program WebGLProgram gl WebGL2RenderingContext
   constructor(context, program)
   {
      //TODO: constructor is too intelligent and prone to fragilities
      if (context instanceof WebGLProgram)
      {
         super (context);
         this.program = program;
      }
      else if (context instanceof WebGL2RenderingContext)
      {
          super (context);
          if (program instanceof WebGLProgram) this.program = program;
      }
      else if (context instanceof GlCanvas)
      {
          super (context.gl);
          this.glCanvas  = context;
          this.program = context.program;
      }
      else if (context instanceof GlProgram)
      {
         super (context.gl);
         this.program  = context.program;
      }
      else
      {
         let shaders = program;
         let canvas  = new GlCanvas (context, shaders);
         super (canvas.gl);
         this.glCanvas = canvas;
         this.program  = canvas.program;
         //throw "GlSurface constructor: unknown context";
      }
      this.vao = this.gl.createVertexArray ();
      //console.log ("this vao created" + this.vao);
   }
   init (){}
   drawVao (){}
   bindVertexArray ()
   {
      this.useProgram();
      this.gl.bindVertexArray (this.vao);
   }
   draw ()
   {
      this.bindVertexArray ();
      this.drawVao ();
   }
}

//TODO: separate logic
//Central functionality of canvas
//  To be used by instanciating directly or by
//                instanciating GlVAObject
//  not for polymorphism
class GlCanvas
{
   #glObj                = null;
   #canvasObj            = null;   //must be HTMLCanvasElement
   #defaultProgramName   = "___DEFAULT_PROGRAM___";
   #programMap           = new Map(); //TODO: decouple program logic from canvas

   constructor (canvasVar, elementVars)
   {
      if  (!canvasVar) throw new Error ("GlCanvas::constructor (canvasVar) must be HTMLCanvasElement or HTMLCanvasElement id string");
      this.#canvas = canvasVar;
      if (canvasVar instanceof  GlInfo) throw new Error ("GlInfo to be handled by offscrfeen");
      this.#extractShaderCodes  ();
      this.#extractElementCodes (elementVars);
      
      for (let program of this.programs)
         program[1].linkProgram ();
   }

   // add missing attribute data-gl-type if known element type
   // if el is string is treated as id
   #prepareElement (el)
   {
      if (typeof el == "string") el = document.getElementById (el);
      if (el.hasAttribute ("data-gl-type")) return el; //nothing to do
      if (!el.hasAttribute ("type")) return null; //not a shader
      //this is a ill formed glcanvas.js lib shader //TODO: consider throw an error
      if (el.getAttribute ("type") == "text/glsl-shader") return null;

      // check if any known type, which might come from other libraries
      // assign attribute type to data-gl-type
      let type = GlShader.translateType (el.getAttribute("type")); //throws if unknown type
      if (!type) return null; //not a shader //TODO: consider throw an error
      el.setAttribute ("data-gl-type", type); //
      return el;
   }

   #extractProgramInfo (el)
   {
      el = this.#prepareElement (el);
      let programName = this.#defaultProgramName;
      if (el.hasAttribute ("data-gl-program")) //if there is a named program
          programName = el.getAttribute ("data-gl-program");
      return {id: programName, shader: new GlShader (this.gl, el)};
   }
   #addScriptShader (el)
   {
      let info = this.#extractProgramInfo (el);
      if (! this.programs.has (info.id) )
            this.programs.set (info.id, new GlProgram (this.gl));
      if (info.shader)
          this.programs.get (info.id).add (info.shader);
      //console.log(info.id + ": " + el.textContent);
   }
   
   #extractElementCodes (elementVars)
   {
      if (!elementVars) return;
      for (let el of elementVars)
         this.#addScriptShader (el);
   }
   #extractShaderCodes ()
   {
      const shaderElements = document.evaluate ("./script[@type='text/glsl-shader']", this.canvas); //xpath inside canvas element
      //const shaderElements = document.evaluate("./script[@type='text/glsl-shader']", this.canvas); //Add one more condition, for external script
      for (let el = shaderElements.iterateNext (); el; el = shaderElements.iterateNext ())
         this.#addScriptShader (el);
   }

   set #gl (glObj){
      if (!glObj instanceof WebGL2RenderingContext) throw new Error ("Not a WebGL2 rendering context");
      this.#glObj = glObj;
   }
   set #canvas (canvasVar)
   {
      if (typeof canvasVar == "string")
          this.#canvasObj = document.getElementById (canvasVar); //yet can be something else than canvas
      else if (canvasVar instanceof HTMLCanvasElement)
          this.#canvasObj = canvasVar;
      if  (!this.canvas)  throw new Error ("GlCanvas::canvas must be NOT NULL HTMLCanvasElement");
      if  (!this.canvas instanceof HTMLCanvasElement) throw new Error ("GlCanvas::canvas must be HTMLCanvasElement");
      this.#gl = this.canvas.getContext ('webgl2');
      this.gl.viewport (0, 0, this.canvas.width, this.canvas.height);
   }

   //publig getter private setter
   get gl       () { return this.#glObj;      }
   get canvas   () { return this.#canvasObj;  }
   get programs () { return this.#programMap; }
   //default context and default program

   #ProgName (progName) {if (progName) return progName; return this.#defaultProgramName;}
   get glProgram  () { return this.programs.get( this.#ProgName () ); }
   get program    () { return this.glProgram.program; }
   getGlProgram   (progName) { return this.programs.get (this.#ProgName (progName)); } //defaults to
   getProgram     (progName) { return this.getGlProgram (progName).program; }          //defaults to
   useProgram     (progName) { this.getGlProgram (progName).useProgram(); }            //defaults to

}

class GlVAObjectAsync extends GlApi //meant to be inherited
{
   #p = null;
   constructor (context, program) //TODO: what is program for?
   {
      super (context.canvas.getContext ('webgl2'));
      this.#p = new GlCanvasAsync (context);
   }
   async ready () //looks good
   {
      if (!this.#p) return this; //TODO: never null, redundant
      return this.#p.ready ().then ( (ref) =>
      {
         this.vao = this.gl.createVertexArray ();
         this.program  = ref.program;
         return;
      } );
   }
   init    (){} //meant to be overriden
   drawVao (){} //meant to be overriden, call it by draw()
   bindVertexArray ()
   {
      this.useProgram();
      this.gl.bindVertexArray (this.vao);
   }
   draw ()
   {
      this.bindVertexArray ();
      this.drawVao ();
   }
}


//TODO: is this used only in GlVAObjectAsync?
class GlCanvasAsync //to be used by instanciating, not to be inherited
{
   #glObj                = null;
   #canvasObj            = null;
   #defaultProgramName   = "___DEFAULT_PROGRAM___";
   #programMap           = new Map();
   #context = null;
   #downloadable = []; //shaders by URL
   #bysource     = []; //shaders by Source
   constructor (context)
   {
      this.#canvas = context.canvas;
      this.#context = context;
      this.#bysourceShaders ();
      this.#downloadShaders ();
   }
   async ready () //looks good
   {
      return Promise.all (this.#downloadable).then( (values) =>
      {
         //works ok when empty, works ok with sync objects
         for (let val of [... values, ...this.#bysource] )
         {
            if (! this.programs.has (val.program) )
               this.programs.set (val.program, new GlProgram (this.gl));
            this.programs.get    (val.program).add (new GlShader (this.gl, val.text, val.type));
         }
         for (let program of this.programs)
            program[1].linkProgram (); //0 key, 1 value
         return this;
      });
   }
   #downloadShaders ()
   {
      for (let info of this.#context.byUrl)
      {
         //TODO: separate progInfo
         let progInfo =
            {
               program : this.#defaultProgramName,
               type    : GlShader.translateType (info.type),
               text    : null,
            };
         if (info.program) progInfo.program = info.program;
         this.#downloadable.push
         (
            fetch(info.href, {cache: "no-store"}).then ((resp) =>
            {
               return resp.text();
            }).then( (text) =>
            {
               progInfo.text = text;
               return progInfo;
            })
         );
      }
   }
   #bysourceShaders ()
   {//{program: glProgramName, type: glType,            src: src}
      //TODO: separate progInfo
      for (let info of this.#context.bySrc)
      {
         let progInfo =
            {
               program : this.#defaultProgramName,
               type    : GlShader.translateType (info.type),
               text    : info.src,
            };
         if (info.program) progInfo.program = info.program;
         //TODO: this.#downloadable??? should be this.#bysource.push (progInfo)
         this.#downloadable.push (progInfo);
      }
   }

   set #gl (glObj){
      if (!glObj instanceof WebGL2RenderingContext) throw new Error ("Not a WebGL2 rendering context");
      this.#glObj = glObj;
   }
   //TODO: make it more restrictive to canvas
   set #canvas (canvasVar)
   {
      if (typeof canvasVar == "string")
          this.#canvasObj = document.getElementById (canvasVar);
      else if (typeof canvasVar == "object")
          this.#canvasObj = canvasVar;
      this.#gl = this.canvas.getContext ('webgl2');
      this.gl.viewport (0, 0, this.canvas.width, this.canvas.height);
   }

   //publig getter private setter
   get gl (){return this.#glObj;}
   get canvas   () { return this.#canvasObj;  }
   get programs () { return this.#programMap; }
   //default context and default program
   get glProgram  () { return this.programs.get(this.#defaultProgramName); }
   get program    () { return this.glProgram.program; }
   getGlProgram   (progName) { return this.programs.get (progName); }
   getProgram     (progName) { return this.getGlProgram (progName).program; }
}


class GlInfo
{
   byUrl  = []; //{program: glProgramName, type: glType, href: href}
   bySrc  = []; //{program: glProgramName, type: glType,            src: src}
   canvas = null;
}

class GlInfoGrabber
{
   glinfo = new GlInfo();
   constructor (canvas)
   {
      this.url = window.location;
      this.currentdir = new URL(this.url.pathname.replace( /\/[^\/]*$/, '/'), this.url.origin);
      if (canvas) this.grabInfo(canvas);
   }
   grabInfo (canvas)
   {
      let elements = document.evaluate ("./script[@type='text/glsl-shader' and @data-gl-src]", canvas);
      for (let el = elements.iterateNext (); el; el = elements.iterateNext ())
         this.glinfo.byUrl.push ({program: el.dataset.glProgram, type: el.dataset.glType, href:  this.buildUrl (el.dataset.glSrc)  } );
      elements = document.evaluate ("./script[@type='text/glsl-shader' and not (@data-gl-src)]", canvas);
      for (let el = elements.iterateNext (); el; el = elements.iterateNext ())
         this.glinfo.bySrc.push ({program: el.dataset.glProgram, type: el.dataset.glType, src: el.textContent.trim ()});

   }
   buildUrl (ref) { return new URL(ref, this.currentdir).href; }
   
}


class GlTexture2D //Async
{
   #p = null;
   constructor(gl, dataPromise)
   {
      this.gl = gl;
      this.#p = dataPromise;
      this.texture = gl.createTexture ();
      this.type = gl.TEXTURE_2D;
      this.init ();
   }
   init (){}
   async ready ()
   {
      if (!this.#p) return this;
      return this.#p.then( (img) =>
         {
            this.image = img;
            this.bindTexture ();
            this.texImage2D ();
            this.gl.generateMipmap (this.type);
            return this
         });
   }
   texImage2D()
   {
      let gl = this.gl;
      gl.texImage2D     (gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, this.image);
   }
   bindTexture (){this.gl.bindTexture (this.type, this.texture);}
}


class GlVideoTexture2D //Async
{
   #p = null;
   constructor (gl, dataPromise)
   {
      this.gl = gl;
      this.#p = dataPromise;

      this.texture = gl.createTexture ();
      this.type = gl.TEXTURE_2D;
      this.init ();
   }
   init (){}
   async ready ()
   {
      if (!this.#p) return this;
      return this.#p.then ( (vd) =>
      {
         this.video = vd;
         this.texImage2DInit ();
         this.gl.generateMipmap (this.type);
         return this;
      });
   }
   texImage2DInit ()
   {
      this.bindTexture ();
      let gl = this.gl;
      gl.texImage2D ( gl.TEXTURE_2D, 0, gl.RGBA, 1, 1, 0, gl.RGBA, gl.UNSIGNED_BYTE, new Uint8Array([0, 0, 255, 255]));

      gl.texParameteri (gl.TEXTURE_2D, gl.TEXTURE_WRAP_S,     gl.CLAMP_TO_EDGE);
      gl.texParameteri (gl.TEXTURE_2D, gl.TEXTURE_WRAP_T,     gl.CLAMP_TO_EDGE);
      gl.texParameteri (gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
   }
   texImage2DUpdate ()
   {
      this.bindTexture ();
      let gl = this.gl;
      gl.texImage2D     (gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, this.video);
   }
   bindTexture (){this.gl.bindTexture (this.type, this.texture);}
   update ()
   {
      this.texImage2DUpdate ();
   }
}
