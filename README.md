#openfx-yru

This is a personal project to realize OFX plugins.

##Plugins list
 
###FractalNoise (under developpement)

The aim of this plugin is to generate a fractal noise and adapt it for compositing. It can be use for smoke, fire effect and more. It use the libnoise library from https://github.com/qknight/libnoise the documentation can be found at http://libnoise.sourceforge.net/

##How to build

At this time only Windows 64 bit is supported using MSYS2. MSYS2 can be download at http://sourceforge.net/projects/msys2/ . you also need CMAKE (https://cmake.org/) to build libnoise library.

 1. go to libnoise and create a folder build_MINGW64 and use this directory as building directory for cmake
 2. go back to the root folder a type "make". this will create a build folder inside FractalNoise. 
  * CONFIG=debug will create debug version (default).
  * CONFIG=release will create an optimized version.
