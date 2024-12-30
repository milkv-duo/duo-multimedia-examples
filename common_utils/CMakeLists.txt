#编译静态库
file(GLOB SRC_LIST *.cpp)
add_library(common_utils STATIC ${SRC_LIST})