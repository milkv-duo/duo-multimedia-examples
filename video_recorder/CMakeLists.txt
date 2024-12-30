# 设置 CMake 最低版本
cmake_minimum_required(VERSION 3.20)

# 定义项目名称
set(projectname videoRecorder)
project(${projectname} VERSION 1.0 LANGUAGES CXX)


# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_options(-Wno-stringop-overflow) # 添加禁用特定警告的选项

# 根据构建类型设置 CMAKE_CXX_FLAGS
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -g -Wall -D_MIDDLEWARE_V2_ -D__CV181X__ -DCV181X")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -O3 -DNDEBUG -D_MIDDLEWARE_V2_ -D__CV181X__ -DCV181X")
endif()

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR})

set(CVI_MPI_PATH "${ARCH_PARTY}/cvi_mpi")
set(USER_PATH "${ARCH_PARTY}/user")
message(${USER_PATH})
set(SRC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src")

#add cvi_mpi header directory
include_directories("${CVI_MPI_PATH}/include")
include_directories("${CVI_MPI_PATH}/include/isp/cv181x")
include_directories("${CVI_MPI_PATH}/sample/common")

#add user  header directory
include_directories("${USER_PATH}/include")
include_directories("${USER_PATH}/include/ffmpeg")

#add common_utils header directory
include_directories("${TOP_PROJECT_PATH}/common_utils")

# src header directory 
include_directories("${SRC_PATH}")

add_subdirectory("${TOP_PROJECT_PATH}/common_utils" build)

file(GLOB_RECURSE SOURCES "${SRC_PATH}/*.cpp")
add_executable(${projectname} main.cpp ${SOURCES})

#add cvi_mpi lib
target_link_directories(${projectname} PUBLIC "${CVI_MPI_PATH}/cv181x/lib_musl_riscv64")
target_link_libraries(${projectname} PUBLIC cvi_ive ini sns_full sample isp vdec venc awb ae af cvi_bin cvi_bin_isp misc isp_algo sys vpu)
target_link_directories(${projectname} PUBLIC "${CVI_MPI_PATH}/cv181x/lib_musl_riscv64/3rd")
target_link_libraries(${projectname} PUBLIC ini)

#add user lib
target_link_directories(${projectname} PUBLIC "${USER_PATH}/lib")
target_link_libraries(${projectname} PUBLIC spdlog)
target_link_directories(${projectname} PUBLIC "${USER_PATH}/lib/Poco")
target_link_libraries(${projectname} PUBLIC PocoNet PocoFoundation PocoUtil PocoJSON PocoXML)

#添加ffmpeg第三方库
target_link_directories(${projectname} PUBLIC "${USER_PATH}/lib/ffmpeg")
target_link_libraries(${projectname} PUBLIC avformat avcodec swresample avutil z)

target_link_libraries(${projectname} PUBLIC 
        common_utils
)

install(TARGETS ${projectname} RUNTIME DESTINATION out)
