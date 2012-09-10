cmake_minimum_required(VERSION 2.8.3)
project(brainfuck)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR})

find_package(Boost 1.50.0 REQUIRED)
find_package(LLVM 3.1 REQUIRED)
set_property(GLOBAL PROPERTY COMPILE_FLAGS ${LLVM_COMPILE_FLAGS})
set_property(GLOBAL PROPERTY LINK_FLAGS ${LLVM_LDFLAGS})
set(CMAKE_EXE_LINKER_FLAGS ${LLVM_LDFLAGS})

INCLUDE_DIRECTORIES(
    ${Boost_INCLUDE_DIRS}
    ${LLVM_INCLUDE_DIR}
)

add_executable(bfc1 bfast.h bfcodegen.h bfcodegen.cpp bfparser.h bfcompiler.h bfcompiler.cpp bfc1.cpp)
TARGET_LINK_LIBRARIES(bfc1 ${LLVM_LDFLAGS} -L${LLVM_LIB_DIR} ${LLVM_LIBS_CORE} ${LLVM_LIBS_JIT} dl pthread)

file(COPY ${CMAKE_SOURCE_DIR}/bfc DESTINATION ${EXECUTABLE_OUTPUT_PATH})

add_executable(bf bfast.h bfcodegen.h bfcodegen.cpp bfparser.h bfjit.h bfjit.cpp bf.cpp)
TARGET_LINK_LIBRARIES(bf ${LLVM_LDFLAGS} -L${LLVM_LIB_DIR} ${LLVM_LIBS_JIT} dl pthread)
