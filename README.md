# SPACE (working title)
**ETA:** Dec. 2013 - Jan. 2014

A cross-platform space game being written in C.


## Platforms supported:
1. **iOS**
2. **Android**
3. Mac OS X
4. Windows 7
5. Linux (currently missing SDL2 libs)


## This repository should not contain:
* Textures
* Sounds
* Music
* Eclipse project files


## Shortcuts:
* SPACE planning board at [Trello](https://trello.com/b/p0dbPsik/space)
* Project [Milestones](about:blank)
* Space [development plan](https://docs.google.com/document/d/1Dtj3eIV7wXX-JbMaPKmj9ORMLq9SSRsVYANchJQ3Ihg/edit)
* Space dropbox
* tzer0's music, CodeBlocked: [CodeBlocked](https://dl.dropboxusercontent.com/u/1184173/renoise/CodeBlocked/index.html)


## Libraries:
- [SDL](http://www.libsdl.org/)
- [SDL_image](http://www.libsdl.org/projects/SDL_image)
- [SDL_mixer](http://www.libsdl.org/projects/SDL_mixer)
- [Chipmunk](http://chipmunk-physics.net/)
- mini-xml (mxml)
- cjson
- zzip
- zlib
- inih (not really needed anymore)


## Tools & Editors:
- Eclipse
- Tiled (TODO: add link)
- (insert particle-editor here)
- physics-body-editor (TODO: add link)
- Blender
- Photoshop

# on windows 11 WSL ubuntu 22
## dependencies
```
sudo apt install libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev libmxml-dev  libinih-dev  libglew-dev cmake libzzip-dev libcjson-dev
```
## chipmunk
```
git clone https://github.com/slembcke/Chipmunk2D.git
cd Chipmunk2D/
git checkout Chipmunk-6.2.2
mkdir build
cd build/
ccmake ..
// disable demos
cmake ..
make
sudo make install
```

## build space
```
mkdir build
cd build
cmake ..
make
cd ..
./build/space
```
