#openfx-yru

This is a personal project to realize OFX plugins.

##Plugins list
 
###FractalNoise (under developpement)

The aim of this plugin is to generate a fractal noise and adapt it for compositing. It can be use for smoke, fire effect and more. It use the libnoise library from https://github.com/qknight/libnoise the documentation can be found at http://libnoise.sourceforge.net/

##How to build

###MSYS makefile (deprecated, prefer CMake method)
At this time only Windows 64 bit is supported using MSYS2. MSYS2 can be download at http://sourceforge.net/projects/msys2/ . you also need CMAKE (https://cmake.org/) to build libnoise library.

1.  go to libnoise and create a folder build_MINGW64 and use this directory as building directory for cmake
2.  go back to the root folder a type "make". this will create a build folder inside FractalNoise. 
    *  CONFIG=debug will create debug version (default).
    *  CONFIG=release will create an optimized version.

###CMake method
At this time, only Windows build have been tested. For beginner with CMake, i recommand to use *cmake-gui* for configuration. If you use a MinGWin compiler, i **highly recommand** to always add *-static* flag to CMAKE_EXE_LINKER_FLAGS, CMAKE_MODULE_LINKER_FLAGS and CMAKE_SHARED_LINKER_FLAGS (check "advanced" to see this variable in cmake-gui).

1.  build libnoise
    1. create a dir, and got into it. this will be your libnoise building dir
	2. launch *cmake* or *cmake-gui*, to configure project and choose generator. Be sure to use libnoise dir as source dir and  use your libnoise building dir as binaries building dir.
	3. build a Release version of the lib
2.  build FractalNoise
    1. create a dir, and got into it. this will be your FractalNoise building dir
	2. configure your project using cmake or cmake-gui. Be sure to use FractalNoise dir as source dir and your FractalNoise building dir as binaries building dir. Especially you need to :
	    *   check if LIBNOISE_INCLUDE_PATH is poiting to the path containing the *noise* dir, where the different header are stored, and change this value if needed.
		*   check if LIBNOISE_LIB is poiting to libnoise or noise library. For windows, i recommand to use static version of the library (.a or .lib)
		*   check if OFX_ARCHITECTURE correspond to the target OS and architecture. See [Installation Directory Hierarchy](http://openfx.sourceforge.net/Documentation/1.3/ofxProgrammingReference.html#id438794) in the OFX documentation for more details
	3. build a Release version of the plugin

	