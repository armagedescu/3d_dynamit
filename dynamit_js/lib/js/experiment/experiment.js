class NewPromise extends Promise
{
   constructor(executor)
   {
      super((resolve, reject) =>  executor(resolve, reject));
	}
   then(onFulfilled, onRejected)
   {
		 console.log(this.constructor.name);
       return super.then(onFulfilled, onRejected);
   }
}

class NewPromiseThis extends Promise
{
   constructor (executor)
   {
      super ((resolve, reject) =>  executor(resolve, reject));
	}
   then (onFulfilled, onRejected)
   {
		 sonsole.log ( typeof this);
       return super.then (onFulfilled, onRejected);
   }
}
// Main function

async function newPromiseExperiment()
{
	let res = "foo";
   let x = new NewPromise ((resolve, reject) => 
   {
      setTimeout (() => 
      {
         resolve(res);
      }, 3000);
      console.log("salut")
   });
	//x.then(value => console.log('Valoarea este: ' + value));
	x.then(console.log);
	console.log("follows " + res);
}
function promisize () {
   const promise1 = Promise.resolve(123);
   const promise2 = Promise.resolve(234);
   class someclass {};
   let a = new someclass();
   if   (promise1 instanceof Promise)  console.log ("promise1 is promise");
   if (!(promise1 instanceof Promise)) console.log ("promise1 is not promise");
   if   (a instanceof Promise)  console.log ("a is promise");
   if (!(a instanceof Promise)) console.log ("a is not promise");
   if   (5 instanceof Promise)  console.log ("5 is promise");
   if (!(5 instanceof Promise)) console.log ("5 is not promise");
   let nomatter = promise1;
   Promise.resolve(nomatter).then(value => console.log("nomatter = " + value));
   nomatter = 5678;
   Promise.resolve(nomatter).then(value => console.log("nomatter = " + value));
   console.log ("start resolving");
   async function asyncPromiseMain()
   {
      let x = await Promise.resolve(promise2).then(value => value);
      console.log("x = " + x);
      x = await Promise.resolve(12345).then(value => value);
      console.log("x = " + x);
      x = await 3;
      console.log("x = " + x);
   }
   asyncPromiseMain();

   class MyPromise extends Promise {}

   const mypromise = MyPromise.resolve(123);

   if (mypromise instanceof MyPromise) console.log("mypromise is MyPromise");
   if (mypromise instanceof Promise)   console.log("mypromise is Promise");

   mypromise.then (value => console.log(value));

};

//Experiments with LIB functions
//lib/heightmap, lib/imageread
async function makeImageReadExperiments()
{
   readImg ("./lib/heightmap/butuceni.png");
   readImg ("./lib/heightmap/tipova.png");
   readImg ("./3rdparty/texture/f-texture.png");
   document.body.appendChild(document.createElement("br") );
   document.body.appendChild(duplicateCanvas(makeImgCanvas("imgBasic")));    //Show a duplicate of invisible image canvas
   document.body.appendChild(await makeCanvasFromImg ("./3rdparty/texture/f-texture.png"));
   //makeCanvasFromImg ("../3rdparty/texture/f-texture.png").then( (canvas) => { document.body.appendChild(canvas);});
   document.body.appendChild(document.createElement("br") );
   document.body.appendChild(makeTextCanvas ("./3rdparty/texture/f-texture.png", 100, 26) ); //Show text canvas "texture/f-texture.png"
   document.body.appendChild(document.createElement("br"));
   document.body.appendChild(duplicateCanvas(makeTextCanvas ("hello", 100, 26))); //Show a duplicate of invisible text canvas

   let img  = makeImg("./3rdparty/texture/f-texture.png");
   document.body.appendChild(document.createElement("br"));
   makeOffscreenFromImg ("./3rdparty/texture/f-texture.png").then ( (cnv) => {document.body.appendChild (duplicateCanvas (cnv));});
   makeCanvasFromImg ("./3rdparty/texture/f-texture.png").then ( (cnv) => {document.body.appendChild (cnv);});
}
////let a = x => x * x;
function makeVideoExperiments()
{
   return;
   loadVideo ("./3rdparty/video/flower.mp4").then((vd) => {document.body.appendChild(vd)});
   loadVideo ("https://interactive-examples.mdn.mozilla.net/media/cc0-videos/flower.mp4").then(vd => {document.body.appendChild(vd)})
   let vd = makeVideo ("./3rdparty/video/flower.mp4");
   vd.height = 90;
   vd.width = 160;
   document.body.appendChild(vd);
}

async function testAsyncClassTest ()
{
   //TODO: separate this test
   class BaseTest 
   {
      #p  = null;
      constructor (){this.#p = Promise.resolve(this);}
      async _bthen (func)
      {
         return this.#p.then ( (ths) =>
         {
            this.#p = null;
            if (func) func (ths);
            return this;
         });
      }
      doHello(a) {console.log("hello: " + a);}
   }
   //TODO: separate this test
   class Test_BaseText extends BaseTest 
   {
      constructor(context)
      {
         super(context);
         this.msg = "test: ";
      }
      doTest(a) {console.log(this.msg + a);}
   }
   //TODO: separate _bthen code with above tests
   (await new Test_BaseText()._bthen()).doHello("await"); //invoking synchronously
   (new Test_BaseText())._bthen(a => {a.doHello("then"); a.doTest("then");} ); //invoking asynchronously

}

////old style ajax, the modern style uses fetch
//var req = new XMLHttpRequest();
//req.addEventListener("load", (event) => 
//{
//   console.log( event.target.getAllResponseHeaders() );
//});
//req.open("GET", "./lib/js/general/api.js");
//req.send();


////DONE: separate this test
//class basetest 
//{
//   #p  = null;
//   constructor (){this.#p = Promise.resolve(this);}
//   async _bthen (func)
//   {
//      return this.#p.then ( (ths) =>
//      {
//         this.#p = null;
//         if (func) func (ths);
//         return this;
//      });
//   }
//   doHello(a) {console.log("hello: " + a);}
//}
//TODO: this is mostly nonsense
////DONE: separate this test
//class test extends basetest 
//{
//   constructor(context)
//   {
//      super(context);
//      this.msg = "test: ";
//   }
//   doTest(a) {console.log(this.msg + a);}
//}
//   //DONE: separate _bthen code with above tests
//   (await new test()._bthen()).doHello("await"); //invoking synchronously
//   (new test())._bthen(a => {a.doHello("then"); a.doTest("then");} ); //invoking asynchronously


// // knowledbe base
// // https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Statements/function*
// // https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Statements/async_function*
// // https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Operators/yield*
// // function* / from yield to next yeld 
// //    value attribute holds the yelded value (returned)
// function* generator(i) {
//    console.log("x here");
//    let   x  = 5; console.log("yield 0");  yield i + x; 
//          x *= 3; console.log("yield 1");  yield i + 10 + x;
//    console.log("yield 2");
// }
// console.log ("--   start    --");
// let gen = generator(10);
// console.log ("--   generator log next   -- yield1: " + gen.next().value);
// console.log ("--   generator next   --"); gen.next();
// console.log ("--   generator log next next   -- yield2: " + gen.next().value);
// function* generator1()
// {
//    console.log("yield 0");
//    yield; console.log("yield 1");
//    yield; console.log("yield 2");
// }
// console.log ("---------------------------------");
// gen = generator1(); gen.next(); gen.next(); gen.next(); 
// //nothing done here anymore
// gen.next(); gen.next();
// gen.next().done gen.next().value;
//class aaa
//{
//   constructor(){}
//   *xxx()
//   {
//      yield 1;
//      yield 2;
//   }
//   yyy = function*()
//   {
//      yield 11;
//      yield 22;
//   }();
//}
//
//async function gpumain (gpuCanvas)
//{
//   let a = new aaa();
//   let b =  a.xxx();
//   console.log (b.next());
//   console.log (b.next());
//   console.log (a.yyy.next());
//   console.log (a.yyy.next());
//}
//const foo = function* () {
//  yield "a";
//  yield "b";
//  yield "c";
//};
//
//let str = "";
//for (const val of foo()) {
//  str += val;
//}