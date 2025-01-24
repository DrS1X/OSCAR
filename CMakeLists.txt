cmake_minimum_required(VERSION 3.22)
project(Clustering)

set(CMAKE_CXX_STANDARD 17)

#MATH(EXPR stack_size "1024 * 1024 * 1024 * 1024") # Mb
#set(CMAKE_EXE_LINKER_FLAGS "-Wl,--stack,${stack_size}")

include_directories(./)
include_directories(file-operator)
include_directories(algorithm)
include_directories(preprocess)
include_directories(util)
include_directories(pkg/release-1930-x64-gdal-3-5-2-mapserver-8-0-0-libs/include)
include_directories(pkg/sidx/include)
include_directories(pkg/sidx/include/spatialindex)
include_directories(pkg/sidx/include/spatialindex/capi)
include_directories(pkg/sidx/include/spatialindex/tools)

link_directories(pkg/release-1930-x64-gdal-3-5-2-mapserver-8-0-0-libs/lib)
ADD_LIBRARY(LibsGDAL
        file-operator/Tif.cpp
        file-operator/Hdfv.cpp
        )
target_link_libraries(LibsGDAL gdal_i)
target_link_libraries(LibsGDAL mfhdf)
target_link_libraries(LibsGDAL hdf5)
target_link_libraries(LibsGDAL hdf5_cpp)
target_link_libraries(LibsGDAL hdf5_hl_cpp)

link_directories(pkg/sidx/lib)
ADD_LIBRARY(LibsSpatialIndex
        algorithm/RTree.cpp)
target_link_libraries(LibsSpatialIndex spatialindex-64)
target_link_libraries(LibsSpatialIndex spatialindex_c-64)


add_executable(Clustering
        main.cpp
        file-operator/Tif.cpp
        file-operator/Hdfv.cpp
        algorithm/DcSTCA.cpp
        algorithm/RTree.cpp
        preprocess/Reader.cpp
        file-operator/Shp.cpp
        DataModel.cpp
        algorithm/Batch.cpp file-operator/Csv.cpp file-operator/Csv.h algorithm/Evaluation.cpp)

target_link_libraries(Clustering LibsGDAL LibsSpatialIndex)
