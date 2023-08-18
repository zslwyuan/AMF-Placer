rm -rf build
mkdir build
cd build
shopt -s extglob
rm !(_deps) -rf
cmake .. -DCMAKE_INSTALL_PREFIX=.. -DCMAKE_PREFIX_PATH=$CONDA_PREFIX -DPYTHON_EXECUTABLE=$(which python) -DPython3_EXECUTABLE=$(which python)  -DCMAKE_BUILD_TYPE=DEBUG 
# cmake .. -DCMAKE_INSTALL_PREFIX=.. -DCMAKE_PREFIX_PATH=$CONDA_PREFIX -DPYTHON_EXECUTABLE=$(which python) -DPython3_EXECUTABLE=$(which python)
cd ..
