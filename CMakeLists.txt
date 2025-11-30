# Original Version created by Xu Chen 
# Upgrade by 星灿长风v(StarWindv) at 2025/11/29 


cmake_minimum_required(VERSION 3.5)
project(brainfuck)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(Boost 1.50.0 REQUIRED)
find_package(LLVM REQUIRED)


add_compile_options(${LLVM_COMPILE_FLAGS})
link_directories(${LLVM_LIB_DIR})


include_directories(
    ${Boost_INCLUDE_DIRS}
    ${LLVM_INCLUDE_DIR}
)


add_executable(
    bfc1 
    src/bfcodegen.cpp 
    src/bfcompiler.cpp 
    src/bfc1.cpp
)
target_link_libraries(bfc1
    PRIVATE
    ${LLVM_LIBS_CORE}
    ${LLVM_LIBS_JIT}
    ${LLVM_LDFLAGS}
    Boost::boost
    dl pthread
)

add_executable(
    bf 
    src/bfcodegen.cpp 
    src/bfjit.cpp 
    src/bf.cpp
)
target_link_libraries(bf
    PRIVATE
    ${LLVM_LIBS_CORE}
    ${LLVM_LIBS_JIT}
    ${LLVM_LDFLAGS}
    Boost::boost
    dl pthread
)

target_compile_options(bf
  PRIVATE
  ${LLVM_COMPILE_FLAGS}
)

install(
  TARGETS bf
  RUNTIME
  DESTINATION bin
)
