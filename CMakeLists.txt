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

link_directories(release-1930-x64-gdal-3-5-2-mapserver-8-0-0-libs/lib)
link_directories(release-1930-x64-gdal-3-5-2-mapserver-8-0-0/bin)

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
        algorithm/DBSCAN.cpp algorithm/BFS.cpp algorithm/algo.h)

target_link_libraries(Clustering gdal_i.lib mfhdf.lib)


