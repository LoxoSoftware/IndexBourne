# Building
**NOTE**: Qt5 builds are only intended for legacy OSes and are missing some quality-of-life features, please use Qt6 if possible.

## Ubuntu / Debian Linux
1. Install the dependencies  
**Qt6:**
```
sudo apt install build-essential cmake qt6-base-dev libqt6openglwidgets6 libquazip1-qt6-dev
```  
**Qt5:**
```
sudo apt install build-essential cmake qtbase5-dev libquazip1-qt5-dev
```  
2. Setup build directory  
```
mkdir build && cd build  
cmake ..
```  
3. Compile  
```
make -j$(nproc)
```  

## Windows
**WARNING**: This guide is experimental, it's likely not the correct way to do it

### Prerequisites:
- MinGW
- Cmake
- Qt6
- Qt5 compatibility module (required for compiling quazip)
- Git (optional)

**It's suggested to install all the prerequisites (minus Qt) from chocolatey**.  
You should download Qt from the online installer, downloadable from Qt website.  
[Install chocolatey](https://chocolatey.org/)  
[Download Qt](https://www.qt.io/development/download-qt-installer-oss)

### First step: compiling Zlib
1. [Download Zlib source](https://zlib.net/) from the offical website.  
2. Unpack the tarball  
3. Make a build folder  
4. Go in the Zlib folder with Powershell, then run
```
cmake -B path-to-the-build-folder -G "MinGW Makefiles"
``` 
 5. To compile run
```
cmake --build path-to-the-build-folder
```  
6. (recommended) Run
```
cmake --build path-to-the-build-folder --target install
```
to install the compiled files in _C:\Program Files (x86)\zlib_  

### Second Step: compiling Quazip
1. Open Qt maintenance tool and add the Qt 5 compatibility module.  
2. Clone the quazip repository:  
```
git clone https://github.com/stachenov/quazip.git
```
or [download from here](https://github.com/stachenov/quazip)  
4. Make a build folder  
5. Go in the quazip folder with Powershell, then run:
```
cmake -B path-to-the-build-folder -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH="qt-installation-path\x.x.x\compiler\" -DZLIB_LIBRARY="path-to-zlib-libzs.a-file"
```  
**NOTE**: the _path-to-zlib-libzs.a-file_ needs to be written with double backslash ("\\\\") in place of the single backslash ("\\")  
**NOTE**: If you compiled zlib with _--target install_ then you do not need the argument "-D ZLIB_LIBRARY="_path-to-zlib-libzs.a-file_"  
5. Build QuaZip:
```
cmake --build path-to-the-build-folder --config Release
```  
6. (recommended) You can install QuaZip with
```
cmake --install path-to-the-build-folder
```  
It will put the compiled files in _C:\Program Files (x86)\quazip_  

### Third Step: compiling the project
1. Clone the project repository  
2. Rename the quazip folder in QuaZip-Qt6 and move it in _qt-installation-path\x.x.x\compiler\cmake_  
3. Open the project in QtCreator  
4. Click "Configure", it will give errors, it's normal  
5. Go in _Projects -> Build settings_ then _Current configuration_  
6. Scroll to _QuaZip-Qt6_DIR_, double click on the field next to it and click "Browse".  Navigate to _qt-installation-path\x.x.x\compiler\cmake\QuaZip-Qt6\lib\cmake_ and select the _QuaZip-Qt6-x.x.x_ folder  
7. Run Cmake and compile  
