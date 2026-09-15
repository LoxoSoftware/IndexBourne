## Building
**NOTE**: Qt5 builds are only intended for legacy OSes and are missing some quality-of-life features, please use Qt6 if possible.

### Ubuntu / Debian Linux
1. Install the dependencies  
**Qt6:** `sudo apt install build-essential cmake qt6-base-dev libqt6openglwidgets6 libquazip1-qt6-dev`  
**Qt5:** `sudo apt install build-essential cmake qtbase5-dev libquazip1-qt5-dev`
2. Setup build directory  
`mkdir build && cd build`  
`cmake ..`
3. Compile  
`make -j$(nproc)`

## Building on Windows 10/11

_WARING: THIS GUIDE MAY NOT WORK_

## Prerequisites:

- MinGW

- Cmake

- Qt6
    - Qt 5 compatibility module (required for compiling quazip)

- Git (optional)

**It's suggested to install all the prerequisites (minus Qt) from chocolatey**
_(donwload Qt from the online installer, donwloadable from Qt website)_

![Download from here](https://chocolatey.org/)

# First Step: compiling Zlib

Download zlib source from the offical website.

![Download from here](https://zlib.net/)

Unpack it

Make build folder wherever you want.

Go in the zlib folder with powershell, run:

```

cmake -B _path to the build folder_ -G "MinGW Makefiles"

```
Then run:

```

cmake --build _path to the build folder_

```

If you run (**recommended**):

```

cmake --build _path to the build folder_ --target install

```
It will put the compiled files in _C:\Program Files (x86)\zlib_


# Second Step: compiling Quazip

Open Qt maintenance tool and add Qt 5 compatibility module.

Clone the quazip repository:

```

git clone https://github.com/stachenov/quazip.git

```

Or ![Download from here](https://github.com/stachenov/quazip)

Make build folder wherever you want.

Go in the quazip folder with powershell, run:

**Note: the _path to zlib libzs.a file_ needs to be written with "\\" in place of the single "\"**

**Note: If you compiled zlib with _--target install_ then you do not need the argument "-D ZLIB_LIBRARY="_path to zlib libzs.a file_"**

```

cmake -B _path to the build folder_ -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH="_qt installation path\x.x.x\compiler\_" -D ZLIB_LIBRARY="_path to zlib libzs.a file_"

```

Then run:

```

cmake --build _path to the build folder_ --config Release

```

If you want to install, run: (**recommended**)

```

cmake --install _path to the build folder_

```
It will put the compiled files in _C:\Program Files (x86)\quazip_

# Third Step: compiling the project

Clone the project repository:

```

git clone https://github.com/LoxoSoftware/IndexBourne.git

```

Or ![Download from here](https://github.com/LoxoSoftware/IndexBourne)


Rename the quazip folder in QuaZip-Qt6 and move it in _qt installation path\x.x.x\compiler\cmake_.

Open Qt and clik open the project. After navigate to the donwloaded project and add the CmakeLists file.

Click configure, it will give errors, it's normal.

Go in Projects>Build settings then Current configuration.

Scroll to QuaZip-Qt6_DIR, double clik on the field next to it and click Browse. Navigate to _qt installation path\x.x.x\compiler\cmake\QuaZip-Qt6\lib\cmake_ and select the QuaZip-Qt6-1.7.2 folder.

Then run cmake and compile.
