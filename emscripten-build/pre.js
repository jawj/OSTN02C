var OSTN02CFactory = (function(stdin, stdout, stderr, onRuntimeInitialized) {

	Module = {
		noExitRuntime: true,
		noInitialRun:  true,
		onRuntimeInitialized: onRuntimeInitialized,
		preRun: function() {
		  FS.init(stdin, stdout, stderr);
		  ENV.OSTN02C_LINE_BUFFERED = 'Y';
		}
	};
