var atarist = require('pcejs-atarist')
var utils = require('pcejs-util')

// add a load progress bar. not required, but good ux
var loadingStatus = utils.loadingStatus(document.getElementById('pcejs-loading-status'))

atarist({
  'arguments': ['-c','pce-config.cfg','-r','-v'],
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
          },
  
  canvas: document.getElementById('pcejs-canvas'),

  monitorRunDependencies: function (remainingDependencies) {
    loadingStatus.update(remainingDependencies)
  },
})

