var atarist = require('pcejs-atarist')
var utils = require('pcejs-util')

// add a load progress bar. not required, but good ux
var loadingStatus = utils.loadingStatus(document.getElementById('pcejs-loading-status'))

atarist((function() {
  var toggle = document.getElementById('togglecolor');
  var mono = toggle === 'undefined' ? "0" : toggle.checked ? "0" : "1";
  return {
  'arguments': ['-c','pce-config.cfg','-r','-v','-I','system.mono='+mono],
  autoloadFiles: [
    'tos-1.00-us.rom',
    'tos-1.04-us.rom',
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
