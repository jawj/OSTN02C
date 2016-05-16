var OSTN02CFactory = (function(stdin, stdout, stderr) {

	Module = {
		noExitRuntime: true,
		noInitialRun:  true,
		preRun: function() {
		  FS.init(stdin, stdout, stderr);
		  ENV.OSTN02C_LINE_BUFFERED = 'Y';
		}
	};
