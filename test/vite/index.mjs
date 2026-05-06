var params = {
  print: (function() {
    var element = document.getElementById('output');
    return function(text) {
      console.log(text);
      element.innerHTML += text.replace('\n', '<br>', 'g') + '<br>';
    };
  })(),
  canvas: document.getElementById('canvas'),
};

params.print("testing..");

import Module from './hello.mjs';
const instance = await Module(params);
console.log('loaded');
