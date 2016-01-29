#openfx-yru

This is a personal project to realize OFX plugins. It require cmake (and cmake-gui) and compile with g++ and Visual Studio. It should also compile with clang++, but i didn't test it. It work under windows and linux.

##Getting the source

	git clone https://github.com/YruamaLairba/openfx-yru.git
	cd openfx-yru
	git submodule update

##OFX-libnoise

This plugin adapt some noise generator from libnoise (https://github.com/qknight/libnoise). It provide "Perlin", "Billow" and "RidgedMulti"  noise generator effect. This effetcs can be used for smoke, fire effects and more. libnoise documentation can be found at http://libnoise.sourceforge.net/.

###How to build, based on makefiles
1.  Open a shell with your building tools enabled
	*   If you use Visual Studio, you should have a shell with "nmake","cl" and "link" provided by visual studio.
	*   If you use MinGWin GCC, you should have "mingw32-make" and "g++" command enabled.
	*   If you use a MSYS shell or an Unix/Linux shell, you should have "make" and "g++" command enabled.
2.  Build libnoise (not needed if you want to use the shared library)
	1. Create a directory where you want, and go into it. this will be your building dir for libnoise.
	2. Launch "cmake-gui <libnoise path>" where <libnoise path> is a path to go to the libnoise content from you building directory.
	3. Click on configure, select the right generator, and select "Use default native compiler". the generator should be :
		*   "NMake Makefiles" when you use Visual Studio and nmake.
		*   "MinGW Makefiles" when you use MinGWin g++ and mingw32-make.
		*   "MSYS Makefiles" when you use a MSYS shell.
		*   "Unix Makefiles" when you are under Unix or Linux.
	4.  Search "CMAKE_BUILD_TYPE" and put "Release" (be careful, it's case sensitive).
	5.  **For Unix/Linux only :**, check "Advanced", search "CMAKE_CXX_FLAGS" and add "-fPIC".
	6.  Click "generate" and once generating is done you can quit cmake-gui.
	7.  In your shell type "make", "nmake" or "mingw32-make" depending the building tools you use. 
2.  Build OFX-libnoise
	1.  Create a directory where you want, and go into it. this will be your building directory for OFX-libnoise.
	2.  Launch "cmake-gui <OFX-libnoise path>" where <OFX-libnoise path> is a path to go to the OFX-libnoise content.
	3.  Click on configure, select the right generator, and select "Use default native compiler". the generator should be :
		*   "NMake Makefiles" when you use Visual Studio and nmake.
		*   "MinGW Makefiles" when you use MinGWin g++ and mingw32-make.
		*   "MSYS Makefiles" when you use a MSYS shell.
		*   "Unix Makefiles" when you are under Unix or Linux.
	Don't worry about configuration the error.
	4.  Set the following variables :
		*   Set CMAKE_BUILD_TYPE to "Release"
		*   Set CMAKE_INSTALL_PREFIX to a path where you want to install the bundle (only required if you want to use install target).
		*   Set LIBNOISE_INCLUDE_PATH to "libnoise/src/noise" in a full path form from the root.
		*   Set LIBNOISE_LIB to the fullname (absolute path + filename) of the static version of libnoise library (should be "libnoise.a" when using g++ and "noise.lib" when using visual studio.)
		*   **Highly recommanded for windows :** if you are using g++, add "-static" to CMAKE_MODULE_LINKER_FLAGS (advanced variable). This will statically link gcc libraries in the plugin.
		*   **For Unix/Linux only :** add "-fPIC" to CMAKE_CXX_FLAGS (advanced).
	5.  If you want to use a shared version of libnoise :
		*   Set LIBNOISE_INCLUDE_PATH to the include folder. On Unix/Linux, it should be "/usr/include/libnoise" or "/usr/local/include/libnoise".
		*   Set LIBNOISE_LIB to the fullname (absolute path + filename) of the shared version of libnoise. On Unix/Linux it should be "/usr/lib/libnoise.so" or "/usr/local/lib/libnoise.so".
	6.  Click "generate" and once generating is done you can quit cmake-gui.
	7.  In your shell type "make", "nmake" or "mingw32-make" depending the building tools you use. You can also use the "install" target to install the plugin.
	8.  Normally, In your OFX host, you have a new effect group with "Perlin", "Billow" and "RidgedMulti" effect.

