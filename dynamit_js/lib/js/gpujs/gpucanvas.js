"use strict";

//General class to handle browser
//all async
class GpuAdapter {
   #adapter   = null;
   #gpu       = null;
   // navigator injection required, not retained
   constructor (navigatorArg) {
      if (navigatorArg instanceof Navigator) this.#gpu = navigatorArg.gpu;
      if (navigatorArg instanceof GPU)       this.#gpu = navigatorArg;
      if (!this.#gpu) throw Error('WebGPU not supported.');
   }
   async initAdapter () {
      if (this.#adapter) { return this.#adapter;}
      return this.#gpu.requestAdapter().then (
         adapter => {this.#adapter = adapter; return adapter;}, //resolved
         error   => {throw new Error("Couldn't request WebGPU adapter.", { cause: error });}                   //rejected
      );
   }
   async requestDevice() {
      return this.adapter.then ( adapter => { return adapter.requestDevice();} );
   }
   get adapter () {return this.initAdapter (); } //promise async
   get gpu     () {return this.#gpu; } //sync

}

//General class to handle browser
//all async
class StaticGpuAdapter {
   static #adapter   = null;
   static #gpu       = null;
   // navigator injection, retained, global navigator will be used implicitly
   constructor (navigatorArg) {
      if (StaticGpuAdapter.#gpu) return;
      if (!navigatorArg) navigatorArg = navigator;
      if (navigatorArg instanceof Navigator) StaticGpuAdapter.#gpu = navigatorArg.gpu;
      if (navigatorArg instanceof GPU)       StaticGpuAdapter.#gpu = navigatorArg;
      if (!StaticGpuAdapter.#gpu) throw Error('WebGPU not supported.');

   }
   static async initAdapter () {
      if (this.adapter) return this.adapter;
      let gpuadapter = new GpuAdapter (StaticGpuAdapter.#gpu);
      return gpuadapter.adapter.then (
         adapter => {StaticGpuAdapter.#adapter = adapter; return adapter;}, //resolved
         error   => {throw new Error("Couldn't request WebGPU adapter.", { cause: error });}                   //rejected
      );
   }
   async requestDevice() {
      return this.adapter.then ( adapter => { return adapter.requestDevice();} );
   }
   get adapter () {return StaticGpuAdapter.initAdapter (); } //promise async
   get gpu     () {return StaticGpuAdapter.#gpu; } //sync

}
class GpuShader
{
   static translateType (type) {return type;}
}
class GpuCanvas
{
   #gpuAdapter           = null;
   #device               = null;
   #webgpuObj            = null;
   #canvasObj            = null;
   #initParams           = null;
   #shaderCodes          = "";

   constructor (initParams) {
      if (!initParams.navigator) initParams.navigator = navigator;
      this.#initParams = initParams;
      this.#canvas = initParams.canvas; //# private setter
      this.#gpuAdapter = initParams.newAdapter ? new GpuAdapter(initParams.navigator) : new StaticGpuAdapter(initParams.navigator);
   }
   async ready (additionals) {
      if (this.device) return this;
      return this.adapter.then (
         adapter => { //resolve
            return adapter.requestDevice().then (
            //   return this.requestDevice().then (
               device => {
                  this.#device = device;
                  this.#webgpu = this.canvas.getContext ('webgpu'); //get context after all
                  this.configureWebGpu (additionals);
                  return this.#extractShaderCodes ().then (
						   codes => {
							   this.#shaderCodes = codes;
							   return this;
							});
               })
         }, //resolved
         error   => {throw new Error("Couldn't request WebGPU adapter.", { cause: error });} //rejected
      )
   }
   
   configureWebGpu (additionals)
   {
      if (!additionals) additionals = {};
      if (!additionals.getGpuConfig)
         additionals.getGpuConfig = (gpuCanvas) => {
            return {
               device: gpuCanvas.device,
               format: gpuCanvas.gpu.getPreferredCanvasFormat(),
               alphaMode: 'premultiplied'
            };
         };
      this.webgpu.configure (additionals.getGpuConfig(this));
   }
   get adapter () {return this.#gpuAdapter.adapter; } //promise
   async requestDevice() { //always return new request device
      return this.adapter.then ( adapter => { return adapter.requestDevice();} );
   }
   get device () {return this.#device; }      //#sync getter
   get gpu () {return this.#gpuAdapter.gpu; } //#sync getter


   // any external recognisable type, translate to internal
   #prepareElement (elArg)
   {
      let el = elArg;
      if (typeof el == "string") el = document.getElementById (elArg);
      if (!el) return elArg; //Not element but source string
      if (!el instanceof HTMLScriptElement) throw new Error ("Not an instance of HTMLScriptElement: " + elArg);
      if (!el.hasAttribute ("data-gpu-type"))
         if (el.hasAttribute ("type") && (el.getAttribute ("type") != "text/wgsl-shader")  )
         {
            let type = GpuShader.translateType (el.getAttribute("type"), this.webgpu);
            if (type) el.setAttribute ("data-gpu-type", type);
         }
      if (el.hasAttribute ("data-gpu-type")) return el;
      return null; //since now a recognisable attribute "data-gpu-type" is a must, otherwise it is not gpu shader
   }
   #extractCode (elVar)
   {
      let el = this.#prepareElement (elVar);
      if (typeof el == "string") return el; //Not element but source string
      let dataGpuType = el.getAttribute ("data-gpu-type"); //must have this element after #prepareELement
      if (dataGpuType != "webgpu-shader") throw new Error ("Unknow data-gpu-type: " + dataGpuType);

      return el.textContent;
   }
   async #extractShaderCodes ()
   {
      const shaderElements = document.evaluate ("./script[@type='text/wgsl-shader']", this.canvas);
		let shaderCodes = "";

      for (let el = shaderElements.iterateNext (); el; el = shaderElements.iterateNext ())
         shaderCodes += this.#extractCode (el);
         //this.#shaderCodes += this.#extractCode (el);
      //console.log (this.#shaderCodes);
      return shaderCodes;
   }

   set #webgpu (webgpuObj) {this.#webgpuObj = webgpuObj;}
   set #canvas (canvasVar)
   {
      let canvas = canvasVar;
      if (typeof canvasVar == "string")
         canvas = document.getElementById (canvasVar);
      if (!canvas instanceof HTMLCanvasElement) throw new Error ("Not an instance of HTMLCanvasElement:" + canvasVar);
      this.#canvasObj = canvas;
   }

   //publig getter private setter
   get webgpu   () { return this.#webgpuObj;  }
   get canvas   () { return this.#canvasObj;  }
   get shaders  () { return this.#shaderCodes;  }


}
function logShader (canvas)
{
   let selectSingleNode = (xpathStr, element, resolver) =>
      document.evaluate(xpathStr, element, resolver, XPathResult.FIRST_ORDERED_NODE_TYPE).singleNodeValue;
   let selectSingleNodeText = (xpathStr, element, resolver) =>
      selectSingleNode (xpathStr, element, resolver).textContent;
   console.log(selectSingleNodeText("./script[@type='text/wgsl-shader']", canvas));
}