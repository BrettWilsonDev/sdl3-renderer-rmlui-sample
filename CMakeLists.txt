cmake_minimum_required(VERSION 3.22)

# Get project name from the folder name
get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)
project(${PROJECT_NAME} CXX)

set(CMAKE_CXX_STANDARD 20)

##########################################################################################
# Configurable options
##########################################################################################

# library toggles
option(USE_SDL3 "Use SDL3" ON)
option(USE_RmlUi "USE_RmlUi" ON)


# build options
option(BUILD_SDL_SHARED_LIB "Build SDL as a shared library" OFF)

##########################################################################################
# MSVC runtime linking
##########################################################################################

# this is here to ensure the runtime is linked staticly for msvc
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebug")
    else()
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
    endif()
endif()

##########################################################################################
# Platform Configuration
##########################################################################################

if(NOT DEFINED PLATFORM)
    set(PLATFORM "default")
endif()

##########################################################################################
# Add dependencies with FetchContent
##########################################################################################

# libraries array to be linked
set(LIBRARIES ${PROJECT_NAME})

include(FetchContent)
function(add_git_dependency libName gitURL gitTag)

    FetchContent_Declare(${libName}
        GIT_REPOSITORY ${gitURL}
        GIT_TAG ${gitTag}
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
    )

    if(NOT ("${libName}" STREQUAL "libname"))
        FetchContent_MakeAvailable(${libName})
    endif()

endfunction()

##########################################################################################
# SDL3 Setup
##########################################################################################

if(USE_SDL3)
    set(SDL3IMAGE_INSTALL OFF)
    set(BUILD_SHARED_LIBS FALSE)
    set(SDL_LIB_TYPE static)

    set(SDL_RENDER_DEFAULT ON)

    if(BUILD_SDL_SHARED_LIB)
        set(BUILD_SHARED_LIBS TRUE)
        set(SDL_LIB_TYPE shared)
        option(SDL_SHARED "Build SDL as a shared library" ON)
    endif()

    set(dep SDL3)
    add_git_dependency(
        ${dep}
        https://github.com/libsdl-org/SDL.git
        head
    )

    FetchContent_GetProperties(${dep})
    if(NOT ${dep}_POPULATED)
    endif()

    list(APPEND LIBRARIES SDL3::SDL3-${SDL_LIB_TYPE})
endif()

if(USE_SDL3)
    set(SDL3IMAGE_INSTALL OFF)

    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        set(SDLIMAGE_AVIF OFF CACHE BOOL "Support loading AVIF images" FORCE)
    endif()

    set(dep SDL3_image)
    add_git_dependency(
        ${dep}
        https://github.com/libsdl-org/SDL_image.git
        head
    )

    FetchContent_GetProperties(${dep})
    if(NOT ${dep}_POPULATED)
    endif()

    include_directories(RECURSE ${CMAKE_CURRENT_LIST_DIR}/build/_deps/sdl3-src/include/SDL3/)

    list(APPEND LIBRARIES SDL3_image::SDL3_image-${SDL_LIB_TYPE})
endif()

##########################################################################################
# freetype Loader Setup
##########################################################################################

if(USE_RmlUi)
    set(dep freetype)

    add_git_dependency(
        ${dep}
        https://github.com/freetype/freetype.git
        master
    )

    FetchContent_GetProperties(${dep})

    # include_directories(RECURSE build/_deps/rmlui-src/include/)
    set(FREETYPE_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/build/_deps/freetype-src/include")
    include_directories(${FREETYPE_INCLUDE_DIR})

    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        if(CMAKE_BUILD_TYPE STREQUAL "Debug")
            set(FREETYPE_LIBRARY "${CMAKE_CURRENT_LIST_DIR}/build/_deps/freetype-build/Debug/freetyped.lib")
        else()
            set(FREETYPE_LIBRARY "${CMAKE_CURRENT_LIST_DIR}/build/_deps/freetype-build/Release/freetype.lib")
        endif()
    else()
        set(FREETYPE_LIBRARY "${CMAKE_CURRENT_LIST_DIR}/build/_deps/freetype-build/libfreetyped.a")
    endif()


    # Create an imported target so RmlUi can find it
    add_library(Freetype::Freetype STATIC IMPORTED)
    set_target_properties(Freetype::Freetype PROPERTIES
        IMPORTED_LOCATION ${FREETYPE_LIBRARY}
        INTERFACE_INCLUDE_DIRECTORIES ${FREETYPE_INCLUDE_DIR}
    )

    # add the dependency here to be linked
    list(APPEND LIBRARIES ${dep})
endif()

##########################################################################################
# rmlui Loader Setup
##########################################################################################

if(USE_RmlUi)
    set(dep rmlui)

    # set(RMLUI_FONT_ENGINE "none" CACHE STRING "Font engine to build RmlUi with. If set to \"none\", RmlUi won't build with any of the included font engines and some samples might be disabled." FORCE)
    set(RMLUI_FONT_ENGINE "freetype" CACHE STRING "Font engine to build RmlUi with. If set to \"none\", RmlUi won't build with any of the included font engines and some samples might be disabled." FORCE)

    set(RMLUI_CUSTOM_RTTI ON CACHE BOOL "Disable standard C++ RTTI" FORCE)
    set(RMLUI_SAMPLES OFF CACHE BOOL "")
    set(RMLUI_TESTS OFF CACHE BOOL "")
    set(RMLUI_LUA_BINDINGS OFF CACHE BOOL "")
    set(RMLUI_LOTTIE_PLUGIN OFF CACHE BOOL "")
    set(RMLUI_SVG_PLUGIN OFF CACHE BOOL "")
    set(RMLUI_HARFBUZZ_SAMPLE OFF CACHE BOOL "")
    set(RMLUI_PRECOMPILED_HEADERS OFF CACHE BOOL "")

    add_git_dependency(
        ${dep}
        https://github.com/mikke89/RmlUi.git
        master
    )

    FetchContent_GetProperties(${dep})

    include_directories(RECURSE ${CMAKE_CURRENT_LIST_DIR}/build/_deps/rmlui-src/include/)

    # set_target_properties(${dep} PROPERTIES
    #     INTERFACE_COMPILE_OPTIONS "-fexceptions;-frtti"
    #     INTERFACE_LINK_LIBRARIES "stdc++" # dynamic
    # )

    # add the dependency here to be linked
    list(APPEND LIBRARIES ${dep} Freetype::Freetype)
endif()


##########################################################################################
# Project executable setup
##########################################################################################

# Adding our source files
# Define PROJECT_SOURCES as a list of all source files
file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp")

# Define PROJECT_INCLUDE to be the path to the include directory of the project
set(PROJECT_INCLUDE "${CMAKE_CURRENT_LIST_DIR}/src/")

# Declaring our executable
add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDE})

##########################################################################################
# Compiler and linker flags
##########################################################################################

if(NOT ${PLATFORM} STREQUAL "Web")
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        if(CMAKE_BUILD_TYPE STREQUAL "Debug")
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++ -pthread")
            target_link_libraries(${LIBRARIES} -static-libgcc -static-libstdc++ -Wl,-Bstatic -lpthread -lwinpthread -Wl,-Bdynamic -lpthread -lstdc++)
        else()
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++ -static -pthread")
            target_link_libraries(${LIBRARIES} -static-libgcc -static-libstdc++ -static -lpthread -Wl,-Bstatic -lstdc++)
        endif()

        if(CMAKE_BUILD_TYPE STREQUAL "Debug")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
        endif()
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_link_libraries(${LIBRARIES} -static-libgcc -static-libstdc++ -static -pthread -Wl,-Bstatic -lstdc++)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
        target_link_libraries(${LIBRARIES} -static-intel -lpthread -Wl,-Bstatic -lstdc++)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_link_libraries(${LIBRARIES})

        if(OFF)
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
        endif()

        # dont forget to set  "cmake.configureSettings": { "CMAKE_BUILD_TYPE": "${buildType}" } in settings.json for vs code
        if(CMAKE_BUILD_TYPE STREQUAL "Debug")
            message("-- console enabled")
            set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebug")
        else()
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
        endif()
    else()
        target_link_libraries(${LIBRARIES})
    endif()
else()
    target_link_libraries(${LIBRARIES})
endif()

##########################################################################################
# Build flags and additional settings
##########################################################################################

# if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/assets)
#     message("copying assets in build")
#     set(destination_dir ${CMAKE_CURRENT_BINARY_DIR}/assets)
#     if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
#         if(CMAKE_BUILD_TYPE STREQUAL "Debug")
#             set(destination_dir ${CMAKE_CURRENT_BINARY_DIR}/debug/assets)
#         else()
#             set(destination_dir ${CMAKE_CURRENT_BINARY_DIR}/release/assets)
#         endif()
#     endif()
#     add_custom_target(copyAssetsOver
#         COMMAND ${CMAKE_COMMAND} -E copy_directory
#         ${CMAKE_CURRENT_SOURCE_DIR}/assets
#         ${destination_dir}
#         COMMENT "Copying assets to build directory"
#     )
#     add_dependencies(${PROJECT_NAME} copyAssetsOver)
# endif()

add_definitions(-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})

# for web build when using emscripten
if(ASYNCIFY)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s ASYNCIFY")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
endif()

# Setting ASSETS_PATH
if(CMAKE_BUILD_TYPE MATCHES "Debug")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -DDEBUG")
    message("-- console enabled")
    if(NOT noAssets AND NOT ${PLATFORM} STREQUAL "Web")
        # Set the asset path macro to the absolute path on the dev machine
        target_compile_definitions(${PROJECT_NAME} PUBLIC ASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/")
    endif()
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -DNDEBUG")
    if(NOT noAssets AND NOT ${PLATFORM} STREQUAL "Web")
        # Set the asset path macro in release mode to a relative path that assumes the assets folder is in the same directory as the game executable
        target_compile_definitions(${PROJECT_NAME} PUBLIC ASSETS_PATH="./assets/")
    endif()
    if(WIN32)
        if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mwindows")
        endif()
        add_compile_definitions(NDEBUG)
    endif()
endif()

# Set common compiler flags
# SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wswitch")
# Set common compiler flags
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -frtti")

if(${PLATFORM} STREQUAL "Web")
    # Tell Emscripten to build an .html file.
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os")
    set(CMAKE_EXECUTABLE_SUFFIX ".html") # This line is used to set your executable to build with the emscripten html template so that you can directly open it.
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html")
    # if(NOT noAssets)
        # if(USE_SDL3)
        #     set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s ALLOW_MEMORY_GROWTH=1 -s GL_UNSAFE_OPTS=0 --use-preload-plugins --preload-file assets/ --shell-file ../vendor/minshell.html")
        # endif(USE_SDL3)
        # target_compile_definitions(${PROJECT_NAME} PUBLIC ASSETS_PATH="/assets/")
    # else()
        if(USE_SDL3)
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s ALLOW_MEMORY_GROWTH=1 -s GL_UNSAFE_OPTS=0 --use-preload-plugins --shell-file ../vendor/minshell.html")
        endif()
    # endif()
endif()

# Ensure that hot-reload is enabled for VS
if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /ZI")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /SAFESEH:NO")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")
endif()
