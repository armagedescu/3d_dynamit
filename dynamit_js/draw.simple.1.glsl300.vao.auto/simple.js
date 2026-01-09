{
let canvas = document.currentScript.parentElement;
document.addEventListener('DOMContentLoaded', () =>
{
   let gl = canvas.getContext('webgl2');
   gl.clearColor(0.9,0.5,0.5,1);
   gl.clear(gl.COLOR_BUFFER_BIT);
});
}