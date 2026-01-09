{
let canvas = document.currentScript.parentElement;
document.addEventListener('DOMContentLoaded', () =>
{
   let context = canvas.getContext('2d');
   context.font      = '20pt Calibri';
   context.fillStyle = 'green';
   context.fillText('Welcome to Tutorialspoint!', 10, 30);
})
}