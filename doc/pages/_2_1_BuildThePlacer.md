@page _2_1_BuildThePlacer Build the Placer
# Build the Placer

First of all, users need to download the project and the command below will download it from GitHub:
```
git clone https://github.com/zslwyuan/AMF-Placer.git
```
We provide a script to build the placer and users can run the following command in the project root directory:

```bash
sudo apt-get install qt5-default #for GUI
./build.sh
```
after the first initial building with "build.sh" script, for the latter building after your update of source code, you can directly go into the build directory and type:
```
make -j8
```
This command will not download and building the Eigen3 source code and PaToH again, which might save some building time.