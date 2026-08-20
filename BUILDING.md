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
