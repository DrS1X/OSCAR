cmake_minimum_required(VERSION 3.22)
project(Clustering)

set(CMAKE_CXX_STANDARD 14)

include_directories(./)
include_directories(operator)
include_directories(thread)
include_directories(pkg/GDAL-x64-2-2-2/include)
include_directories(pkg/HDF4-x64-4-2-13/include)
include_directories(pkg/HDF5 x64-1-82-0/include)
include_directories(pkg/NETCDF-x64/include)
include_directories(pkg/Matrix/include)

link_directories(pkg/GDAL-x64-2-2-2/lib)

add_executable(Clustering
        operator/gdalOpt.cpp
        operator/opt.cpp
        operator/hdfOpt.cpp
        thread/AnomalyAnalysis.cpp
        thread/AnomalyAnalysis.h
        thread/Convertor.cpp
        thread/Convertor.h
        _const.h
        algorithm/DBSCAN.cpp
        algorithm/DBSCAN.h
        algorithm/DcSTMC.cpp
        algorithm/DcSTMC.h
        main.cpp
        process/Postprocessor.cpp
        process/Postprocessor.h
        operator/ThreadPool.h
        process/Vectorization.cpp
        process/Vectorization.h operator/opt.h)

target_link_libraries(Clustering gdal_i.lib)

