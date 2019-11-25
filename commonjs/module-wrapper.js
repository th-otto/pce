module.exports = function(config) {
  // emscripten expects an object called Module to be defined
  var Module = opts || {};

  // hide node/commonjs globals so environment is not detected as node
  var process = null; 
  var require = null; 


  // [emscripten output goes here]


  // expose stuff which emscripten defines in this scope
  Module.FS = FS;
  Module.PATH = PATH;
  Module.IDBFS = IDBFS;
  Module.NODEFS = NODEFS;

  return Module;
}