var atarist = require('pcejs-atarist')
var utils = require('pcejs-util')

// add a load progress bar. not required, but good ux
var loadingStatus = utils.loadingStatus(document.getElementById('pcejs-loading-status'))

atarist((function() {
  var toggle = document.getElementById('togglecolor');
  var mono = toggle.checked ? "0" : "1";
  var speed = document.getElementById('speedsel');
  var factor = speed.value;
  var tos = document.getElementById('tossel');
  return {
  'arguments': ['-c','pce-config.cfg','-r','-v','-I','system.mono='+mono,'-I','emu.cpu.speed='+factor,'-I','rom.file="'+tos.value+'"'],
  // ,'-I','rom.file="'+tos.value+'"'
  autoloadFiles: [
    'tos100us.rom',
    'tos104us.rom',
    'etos256us.img',
    'etos256de.img',
    'pce-config.cfg',
    'fd0.st',
    'fd1.psi',
    'hd0.img',
  ],

  print: function(text) {
            var element = document.getElementById('output');
            text = Array.prototype.slice.call(arguments).join(' ');
            element.value += text + "\n";
            element.scrollTop = 99999; // focus on bottom
         },
  
  printErr: function(text) {
            var element = document.getElementById('output');
            text = Array.prototype.slice.call(arguments).join(' ');
            element.value += text + "\n";
            element.scrollTop = 99999; // focus on bottom
            console.log(text + "\n");
          },
  
  canvas: document.getElementById('pcejs-canvas'),

  monitorRunDependencies: function (remainingDependencies) {
    loadingStatus.update(remainingDependencies)
  },
};})())
