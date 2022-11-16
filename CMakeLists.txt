cmake_minimum_required(VERSION 3.22)
project(Clustering)

set(CMAKE_CXX_STANDARD 14)


include_directories(./)
include_directories(operator)
include_directories(thread)
include_directories(algorithm)
include_directories(process)
include_directories(release-1930-x64-gdal-3-5-2-mapserver-8-0-0-libs/include)
include_directories(release-1930-x64-gdal-3-5-2-mapserver-8-0-0-libs/include/hdf4)
include_directories(sidx/include)
include_directories(sidx/include/spatialindex)
include_directories(sidx/include/spatialindex/capi)
include_directories(sidx/include/spatialindex/tools)

link_directories(release-1930-x64-gdal-3-5-2-mapserver-8-0-0-libs/lib)
ADD_LIBRARY(LibsGDAL
        operator/gdalOpt.cpp
        operator/hdfOpt.cpp
        operator/opt.cpp
        )
target_link_libraries(LibsGDAL gdal_i.lib mfhdf.lib)

link_directories(sidx/lib)
ADD_LIBRARY(LibsSpatialIndex
        algorithm/RTree.cpp)
target_link_libraries(LibsSpatialIndex spatialindex-64)
target_link_libraries(LibsSpatialIndex spatialindex_c-64)

set(SIDX_SRC_DIR "sidx/src")

add_executable(Clustering
        main.cpp
        operator/gdalOpt.cpp
        operator/hdfOpt.cpp
        operator/opt.cpp
        process/Postprocessor.cpp
        process/Vectorization.cpp
        process/AnomalyAnalysis.cpp
        process/Convertor.cpp
        algorithm/DcSTMC.cpp
        algorithm/DBSCAN.cpp algorithm/RTree.cpp algorithm/algo.h algorithm/RTree.h)

target_link_libraries(Clustering LibsGDAL LibsSpatialIndex)
