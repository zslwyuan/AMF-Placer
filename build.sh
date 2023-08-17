rm -rf build
mkdir build
cd build
shopt -s extglob
rm !(_deps) -rf
# cmake ../src -DCMAKE_INSTALL_PREFIX=. -DPYTHON_EXECUTABLE=$(python3 -c "import sys; print(sys.executable)") coredump
cmake ../src -DCMAKE_INSTALL_PREFIX=. -DPYTHON_EXECUTABLE=$(python3 -c "import sys; print(sys.executable)")
cd ..
#bash make.sh
# load MKL for parallelism
# execute the line below before you run the placer in the terminal
# source /opt/intel/compilers_and_libraries_2020.4.304/linux/mkl/bin/mklvars.sh intel64
