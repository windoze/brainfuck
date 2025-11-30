#!/bin/sh

# Copyright (c) 2025 星灿长风v(StarWindv) 
# Created by 星灿长风v(StarWindv) at 2025/11/29
# LICENSE: MIT


set -e

if command -v tput >/dev/null 2>&1; then
    RED=$(tput setaf 1)
    GREEN=$(tput setaf 2)
    YELLOW=$(tput setaf 3)
    BLUE=$(tput setaf 4)
    MAGENTA=$(tput setaf 5)
    CYAN=$(tput setaf 6)
    BOLD=$(tput bold)
    RESET=$(tput sgr0)
else
    RED=''
    GREEN=''
    YELLOW=''
    BLUE=''
    MAGENTA=''
    CYAN=''
    BOLD=''
    RESET=''
fi

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)"/.."
PROJECT_NAME=$(basename "$(cd "$(dirname "$SCRIPT_DIR/..")" && pwd -P)/$(basename "$relative_path")")
DEFAULT_PREFIX="$HOME/.local"
BUILD_DIR="$SCRIPT_DIR/build"
BIN_DIR="$BUILD_DIR/bin"

BUILD_TYPE="Release"
CLEAN_BUILD=false
INSTALL=false
VERBOSE=false
JOBS=$(nproc 2>/dev/null || echo 1)
CLEAN_ONLY=false

INSTALLE_TO="${INSTALL_PREFIX:-$DEFAULT_PREFIX}/bin"



print_info() {
    echo "${CYAN} - $1${RESET}"
}

print_complete() {
    echo "${GREEN} - $1${RESET}"
}

print_progressive() {
    echo "${MAGENTA} * $1${RESET}"
}

print_warning() {
    echo "${YELLOW} △ $1${RESET}"
}

print_error() {
    echo "${RED} X $1${RESET}" >&2
}


check_dependency() {
    if ! command -v "$1" >/dev/null 2>&1; then
        print_error "Required dependencies '$1' is not installed"
        return 1
    fi
}

check_path() {
    local target="$1"
    target_trimmed=$(printf '%s' "$target" | xargs)
    printf '%s' "$PATH" | tr ':' '\n' | while read -r dir; do
        dir_trimmed=$(printf '%s' "$dir" | xargs)
        if [ "$dir_trimmed" = "$target_trimmed" ]; then
            exit 0
        fi
    done
    if [ $? -eq 0 ]; then
        return 0
    else
        return 1
    fi
}



clean_build() {
    print_progressive "Cleaning Build Directory"
    if [ -d "$BUILD_DIR" ]; then
        rm -rf "$BUILD_DIR"
        print_complete "Cleaning Complete"
    else
        print_warning "Build Directory Not Found, Skip"
    fi
}

configure_project() {
    print_progressive "Configuring Project"
    
    CMAKE_ARGS="-DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX:-$DEFAULT_PREFIX} -DCMAKE_BUILD_TYPE=$BUILD_TYPE"
    
    if [ "$VERBOSE" = "true" ]; then
        CMAKE_ARGS="$CMAKE_ARGS -DCMAKE_VERBOSE_MAKEFILE=ON"
    fi
    
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    
    print_info "CMake Args: $CMAKE_ARGS"
    cmake $CMAKE_ARGS ..
    
    print_complete "Configure Completed"
}

build_project() {
    print_progressive "Start Compiling"

    MAKE_ARGS="-j$JOBS"
    if [ "$VERBOSE" = "true" ]; then
        MAKE_ARGS="$MAKE_ARGS VERBOSE=1"
    fi
    
    cd "$BUILD_DIR"
    make $MAKE_ARGS

    print_complete "Build Successful"
}

install_project() {
    print_progressive "Start Installation"
    cd "$BUILD_DIR"
    make install
    print_complete "Installed To: ${INSTALL_PREFIX:-$DEFAULT_PREFIX}"
}


show_build_info() {
    print_complete "Build Completed"

    if [ -d "$BIN_DIR" ]; then
        executables=$(find "$BIN_DIR" -type f -executable 2>/dev/null)
        if [ -n "$executables" ]; then
            echo "${GREEN} - Executable File:${RESET}"
            IFS='
'
            indented_files=""
            for file in $executables; do
                if [ -n "$file" ]; then
                    indented_files="${indented_files}   ${file}\n"
                fi
            done
            echo $indented_files
            echo "\033[1A\033[1A"
        fi
    fi
    
    if [ "$INSTALL" = "true" ]; then
        echo " - ${GREEN}Installed to: ${INSTALL_PREFIX:-$DEFAULT_PREFIX}${RESET}"
        if ! check_path "$INSTALLE_TO"; then
            echo ""
            echo "$ - {YELLOW}Tips:${RESET}"
            echo " • You can add installed path to PATH: ${BOLD}export PATH=\"${INSTALL_PREFIX:-$DEFAULT_PREFIX}/bin:\$PATH\"${RESET}"
        fi
    fi
}

usage() {
    cat << EOF
Usage: ${YELLOW}$0 [Options]${RESET}

Automatical BrainFuck Compiler Build and Install Script.

${CYAN}OPTIONS${RESET}:
    -h, --help          Show this message and exit
    -c, --clean         Clean up the build directory
    -i, --install       Install binary executable files (Default: $HOME/.local)
    -v, --verbose       Show verbose build infomation
    -d, --debug         Use debug mode to build
    -j, --jobs NUM      Specify the number of parallel compilation tasks (Default: $JOBS)
    -p, --prefix DIR    Specified installation directory (Default: $HOME/.local)

${CYAN}EXAMPLE${RESET}:
    $0                   # Build Release
    $0 -c                # Cleaning Build Directory
    $0 -c -i             # Cleaning Build Directory and Install
    $0 -d -v             # Use debug mode to build and Output Verbose Information
    $0 -p /usr/local     # Install to "/usr/local"

EOF
}



main() {
    while [ $# -gt 0 ]; do
        case $1 in
            -h|--help)
                usage
                exit 0
                ;;
            -c|--clean)
                CLEAN_BUILD=true
                CLEAN_ONLY=true
                shift
                ;;
            -i|--install)
                INSTALL=true
                CLEAN_ONLY=false
                shift
                ;;
            -v|--verbose)
                VERBOSE=true
                CLEAN_ONLY=false
                shift
                ;;
            -d|--debug)
                BUILD_TYPE="Debug"
                CLEAN_ONLY=false
                shift
                ;;
            -j|--jobs)
                JOBS="$2"
                CLEAN_ONLY=false
                shift 2
                ;;
            -p|--prefix)
                INSTALL_PREFIX="$2"
                CLEAN_ONLY=false
                shift 2
                ;;
            *)
                print_error "Unknown Args: $1"
                usage
                exit 1
                ;;
        esac
    done
    
    if [ "$CLEAN_ONLY" = "true" ] && [ "$CLEAN_BUILD" = "true" ]; then
        print_progressive "Cleaning $PROJECT_NAME"
        clean_build
        print_complete "Clean Complete"
        exit 0
    fi
    
    print_progressive "Start Build $PROJECT_NAME"

    print_info "Project   : $PROJECT_NAME"
    print_info "Parallel  : $JOBS"
    print_info "Build Type: $BUILD_TYPE"
    print_info "Install To: ${INSTALL_PREFIX:-$DEFAULT_PREFIX}"

    print_progressive "Checking Dependency"
    check_dependency cmake
    check_dependency make
    if ! check_dependency gcc && ! check_dependency clang; then
        print_error "You need GCC or CLANG to build this project."
        exit 1
    fi

    if [ "$CLEAN_BUILD" = "true" ]; then
        clean_build
    fi

    configure_project
    build_project

    if [ "$INSTALL" = "true" ]; then
        install_project
    fi
    
    show_build_info
    
    print_complete "$PROJECT_NAME Build Complete"
}

main "$@"
