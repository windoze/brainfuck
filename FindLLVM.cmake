# Detect LLVM and set various variable to link against the different component of LLVM
#
# NOTE: This is a modified version of the module originally found in the OpenGTL project
# at www.opengtl.org
#
# LLVM_BIN_DIR : directory with LLVM binaries
# LLVM_LIB_DIR : directory with LLVM library
# LLVM_INCLUDE_DIR : directory with LLVM include
#
# LLVM_COMPILE_FLAGS : compile flags needed to build a program using LLVM headers
# LLVM_LDFLAGS : ldflags needed to link
# LLVM_LIBS_CORE : ldflags needed to link against a LLVM core library
# LLVM_LIBS_JIT : ldflags needed to link against a LLVM JIT
# LLVM_LIBS_JIT_OBJECTS : objects you need to add to your source when using LLVM JIT

# Upgrade to LLVM 18
# by 星灿长风v(StarWindv) on 2025/11/29


if (MSVC)
  set(LLVM_ROOT "C:/Program Files/LLVM")
  if (NOT IS_DIRECTORY ${LLVM_ROOT})
    message(FATAL_ERROR "Could NOT find LLVM")
  endif ()

  message(STATUS "Found LLVM: ${LLVM_ROOT}")
  set(LLVM_BIN_DIR ${LLVM_ROOT}/bin)
  set(LLVM_LIB_DIR ${LLVM_ROOT}/lib)
  set(LLVM_INCLUDE_DIR ${LLVM_ROOT}/include)

  set(LLVM_COMPILE_FLAGS "")
  set(LLVM_LDFLAGS "")
  set(LLVM_LIBS_CORE 
    LLVMLinker LLVMArchive LLVMBitWriter LLVMBitReader 
    LLVMInstrumentation LLVMScalarOpts LLVMIPO LLVMTransformUtils 
    LLVMAnalysis LLVMTarget LLVMMC LLVMCore LLVMSupport
  )
  set(LLVM_LIBS_JIT 
    LLVMX86AsmParser LLVMX86AsmPrinter LLVMX86CodeGen 
    LLVMSelectionDAG LLVMAsmPrinter LLVMX86Info 
    LLVMExecutionEngine LLVMOrcJIT LLVMCodeGen 
    LLVMScalarOpts LLVMTransformUtils LLVMAnalysis 
    LLVMTarget LLVMMC LLVMCore LLVMSupport
  )
  set(LLVM_LIBS_JIT_OBJECTS "")
endif (MSVC)

if (LLVM_INCLUDE_DIR)
  set(LLVM_FOUND TRUE)
else (LLVM_INCLUDE_DIR)

  find_program(LLVM_CONFIG_EXECUTABLE
    NAMES llvm-config
    PATHS
    /opt/local/bin
    /opt/llvm/bin
    /usr/bin
    /usr/local/bin
    )

  find_program(LLVM_GCC_EXECUTABLE
    NAMES llvm-gcc llvmgcc
    PATHS
    /opt/local/bin
    /opt/llvm/bin
    /Developer/usr/bin
    /usr/lib/llvm/llvm/gcc-4.2/bin
    )

  find_program(LLVM_GXX_EXECUTABLE
    NAMES llvm-g++ llvmg++
    PATHS
    /opt/local/bin
    /opt/llvm/bin
    /Developer/usr/bin
    /usr/lib/llvm/llvm/gcc-4.2/bin
    )

  if(LLVM_GCC_EXECUTABLE)
    MESSAGE(STATUS "LLVM llvm-gcc found at: ${LLVM_GCC_EXECUTABLE}")
  endif(LLVM_GCC_EXECUTABLE)

  if(LLVM_GXX_EXECUTABLE)
    MESSAGE(STATUS "LLVM llvm-g++ found at: ${LLVM_GXX_EXECUTABLE}")
  endif(LLVM_GXX_EXECUTABLE)
  
  if(LLVM_CONFIG_EXECUTABLE)
    MESSAGE(STATUS "LLVM llvm-config found at: ${LLVM_CONFIG_EXECUTABLE}")
  else(LLVM_CONFIG_EXECUTABLE)
    MESSAGE(FATAL_ERROR "Could NOT find LLVM")
  endif(LLVM_CONFIG_EXECUTABLE)


  MACRO(FIND_LLVM_LIBS LLVM_CONFIG_EXECUTABLE _libname_ LIB_VAR OBJECT_VAR)
    set(${LIB_VAR} "")
    set(${OBJECT_VAR} "")
    execute_process(
      COMMAND ${LLVM_CONFIG_EXECUTABLE} --libs ${_libname_}
      OUTPUT_VARIABLE ${LIB_VAR}
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_QUIET
    )
    if(NOT ${LIB_VAR} STREQUAL "")
      STRING(REGEX MATCHALL "[^ ]*[.]o[ $]"  ${OBJECT_VAR} ${${LIB_VAR}})
      SEPARATE_ARGUMENTS(${OBJECT_VAR})
      STRING(REGEX REPLACE "[^ ]*[.]o[ $]" ""  ${LIB_VAR} ${${LIB_VAR}})
    endif()
  ENDMACRO(FIND_LLVM_LIBS)

  function(TRANSFORM_VERSION numerical_result version)
    string(REGEX REPLACE "^([0-9.]+).*$" "\\1" internal_version ${version})
    string(REGEX REPLACE "^([0-9]*).+$" "\\1" major ${internal_version})
    string(REGEX REPLACE "^[0-9]*\\.([0-9]*).*$" "\\1" minor ${internal_version})
    set(patch 0)
    
    if(NOT minor MATCHES "[0-9]+")
      set(minor 0)
    endif(NOT minor MATCHES "[0-9]+")
    
    if(NOT major MATCHES "[0-9]+")
      set(major 0)
    endif(NOT major MATCHES "[0-9]+")
    
    math(EXPR internal_numerical_result
      "${major}*1000000 + ${minor}*1000"
      )
    set(${numerical_result} ${internal_numerical_result} PARENT_SCOPE)
  endfunction(TRANSFORM_VERSION)
  

  execute_process(
    COMMAND ${LLVM_CONFIG_EXECUTABLE} --version
    OUTPUT_VARIABLE LLVM_STRING_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  MESSAGE(STATUS "LLVM version: " ${LLVM_STRING_VERSION})
  transform_version(LLVM_VERSION ${LLVM_STRING_VERSION})
  
  execute_process(
    COMMAND ${LLVM_CONFIG_EXECUTABLE} --bindir
    OUTPUT_VARIABLE LLVM_BIN_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  execute_process(
    COMMAND ${LLVM_CONFIG_EXECUTABLE} --libdir
    OUTPUT_VARIABLE LLVM_LIB_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  execute_process(
    COMMAND ${LLVM_CONFIG_EXECUTABLE} --includedir
    OUTPUT_VARIABLE LLVM_INCLUDE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  
  execute_process(
    COMMAND ${LLVM_CONFIG_EXECUTABLE} --cxxflags
    OUTPUT_VARIABLE LLVM_COMPILE_FLAGS
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  MESSAGE(STATUS "LLVM CXX flags: " ${LLVM_COMPILE_FLAGS})
  
  execute_process(
    COMMAND ${LLVM_CONFIG_EXECUTABLE} --ldflags
    OUTPUT_VARIABLE LLVM_LDFLAGS
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  MESSAGE(STATUS "LLVM LD flags: " ${LLVM_LDFLAGS})

  execute_process(
    COMMAND ${LLVM_CONFIG_EXECUTABLE} --libs core IPO instrumentation bitreader bitwriter linker
    OUTPUT_VARIABLE LLVM_LIBS_CORE
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
  )
  MESSAGE(STATUS "LLVM core libs: " ${LLVM_LIBS_CORE})

  IF(APPLE AND UNIVERSAL)
    FIND_LLVM_LIBS( ${LLVM_CONFIG_EXECUTABLE} "orcjit mcjit executionengine x86codegen x86asmparser x86asmprinter x86targetmc" LLVM_LIBS_JIT LLVM_LIBS_JIT_OBJECTS )
  ELSE(APPLE AND UNIVERSAL)
    FIND_LLVM_LIBS( ${LLVM_CONFIG_EXECUTABLE} "orcjit mcjit executionengine x86codegen x86asmparser x86asmprinter x86targetmc" LLVM_LIBS_JIT LLVM_LIBS_JIT_OBJECTS )
  ENDIF(APPLE AND UNIVERSAL)
  
  MESSAGE(STATUS "LLVM JIT libs: " ${LLVM_LIBS_JIT})
  MESSAGE(STATUS "LLVM JIT objs: " ${LLVM_LIBS_JIT_OBJECTS})
  
  if(LLVM_INCLUDE_DIR)
    set(LLVM_FOUND TRUE)
  endif(LLVM_INCLUDE_DIR)
  
  if(LLVM_FOUND)
    message(STATUS "Found LLVM: ${LLVM_INCLUDE_DIR}")
  else(LLVM_FOUND)
    if(LLVM_FIND_REQUIRED)
      message(FATAL_ERROR "Could NOT find LLVM")
    endif(LLVM_FIND_REQUIRED)
  endif(LLVM_FOUND)

endif (LLVM_INCLUDE_DIR)
