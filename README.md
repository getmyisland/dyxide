# dyxide

Dyxide is to be a faithful remake of the game "IRONFALL Invasion", which was released for the Nintendo 3DS in 2015.

The project is still at a very early stage and it may take a while for it to take shape.

I am in no way affiliated with VD-dev or Nintendo. I'm just a fan of the game who wants to recreate parts of it for the PC.

## Setup

```
# Clone the repository
$ git clone https://github.com/getmyisland/dyxide.git

# Navigate into the dyxide folder
$ cd dyxide

# Clone all submodules as well
$ git submodule update --init --recursive
```

### CMake

```
# Generate build files/folders
$ cmake -B build

# Build the project
$ cmake --build build
```