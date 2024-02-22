//
// Purpose:
// 	    Copy the newest arma_rcon_dll.dll file out of build/ directory
//      into src/deps directory so we can use it in our node applications.
//

(async function() {
	const path = require('path');
	const fs = require('fs');
	
	// The base directory that we want to scan for our modules.
	const baseDir = path.join( __dirname, 'build' );
	
	// The working directory of our nodejs application.
	const workingDir = path.join( __dirname, 'src' );
	const depsDir = path.join( workingDir, 'deps' );
	
	// Firstly let's make sure the deps/ folder is created.
	if( !fs.existsSync( depsDir ) ) {
		
		// Create the directory.
		fs.mkdirSync( depsDir );
	}
	
	// Iterate the directory and find all files inside.
	const directories = fs.readdirSync(baseDir);
	for( let directory of directories ) {
		
		// Read into the current working directory.
		let subpath = path.join( baseDir, directory );
		let subdirs = fs.readdirSync( subpath );
	
		for( directory of subdirs ) {
			subpath = path.join( subpath, directory );
			subdirs = fs.readdirSync( subpath );

			// We should be deep enough to find our DLLs now.
			for( let file of subdirs ) {
				
				// Check to see if our DLL is here.
				if( path.extname( path.join( subpath, file ) ) === '.dll' ) {
					
					// If it is then we want to move it.
					fs.copyFileSync( path.join( subpath, file ), path.join( depsDir, file ) );
					
				}
			}
		}
	}
})();