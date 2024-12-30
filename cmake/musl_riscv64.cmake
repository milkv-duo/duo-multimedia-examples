# toolchain.cmake

# 设置目标系统
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR riscv64)

set(TOP_PROJECT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(ARCH_PARTY "${TOP_PROJECT_PATH}/3rd_party/musl_riscv64")

# Fixme replace with your appropriate path.
set(CROSS_CHAIN_PATH  "XXX/host-tools/gcc/riscv64-linux-musl-x86_64")

# 设置交叉编译工具链路径
set(CMAKE_CXX_COMPILER ${CROSS_CHAIN_PATH}/bin/riscv64-unknown-linux-musl-g++)

# 设置编译标志
set(CMAKE_CXX_FLAGS "-mcpu=c906fdv -march=rv64imafdcv0p7xthead -mcmodel=medany -mabi=lp64d -latomic")

add_definitions(-D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64)

# where is the target environment
SET(CMAKE_FIND_ROOT_PATH  ${CROSS_CHAIN_PATH}/sysroot)

# search for programs in the build host directories (not necessary)
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)