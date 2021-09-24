
cmake_minimum_required(VERSION 3.10) 

aux_source_directory(. curDirectory)

include_directories(./ ../../utils/ ../deviceInfo/)

add_library(DesignInfo ${curDirectory})


