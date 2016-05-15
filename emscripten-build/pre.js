var OSTN02C = (function() {

	Module = {
		noExitRuntime: true,
		noInitialRun:  true,
		preRun: function() {
		  FS.init(null, stdoutCallback, null);
		  ENV.OSTN02C_LINE_BUFFERED = 'Y';
		}
	};
