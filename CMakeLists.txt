cmake_minimum_required(VERSION 2.8)

project (oaml)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})

include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)


# Configuration types
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_definitions(-DDEBUG)
	message("Debug mode: Yes (Disable by param -DCMAKE_BUILD_TYPE=Release)")
else()
	message("Debug mode: No (Enable by param -DCMAKE_BUILD_TYPE=Debug)")
endif()

##
# Options
#
option(ENABLE_STATIC "Build static library" ON)
option(ENABLE_SHARED "Build shared library" ON)
option(ENABLE_UNITYPLUGIN "Build AudioPluginOAML plugin for Unity" ON)

if(ENABLE_STATIC)
	message("Build static: Yes (Disable by param -DENABLE_STATIC=OFF)")
else()
	message("Build static: No  (Enable by param -DENABLE_STATIC=ON)")
endif()

if(ENABLE_SHARED)
	message("Build shared: Yes (Disable by param -DENABLE_SHARED=OFF)")
else()
	message("Build shared: No  (Enable by param -DENABLE_SHARED=ON)")
endif()

if(ENABLE_UNITYPLUGIN)
	message("Build AudioPluginOAML: Yes (Disable by param -DENABLE_UNITYPLUGIN=OFF)")
else()
	message("Build AudioPluginOAML: No  (Enable by param -DENABLE_UNITYPLUGIN=ON)")
endif()


##
# Set CXX_FLAGS depending on compiler
#
if (MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra")
endif()


##
# Find VorbisFile lib
#
find_package(OggVorbis REQUIRED)
find_package(VorbisFile REQUIRED)

if (OGGVORBIS_FOUND AND VORBISFILE_FOUND)
	include_directories(${VORBISFILE_INCLUDE_DIR})
	set(OAML_LIBS ${OAML_LIBS} ${OGG_LIBRARY} ${VORBIS_LIBRARY} ${VORBISFILE_LIBRARIES})
endif()


##
# Build static and shared libraries
#
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/include/GitSHA1.h.in" "${CMAKE_CURRENT_BINARY_DIR}/include/GitSHA1.h" @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)

set(SOURCES src/audioFile.cpp src/ByteBuffer.cpp src/oaml.cpp src/oamlAudio.cpp src/oamlBase.cpp src/oamlCompressor.cpp src/oamlLayer.cpp src/oamlMusicTrack.cpp src/oamlSfxTrack.cpp src/oamlTrack.cpp src/oamlUtil.cpp src/aif.cpp src/ogg.cpp src/wav.cpp src/gettime.cpp src/tinyxml2.cpp)
if (ENABLE_STATIC)
	add_library(oaml STATIC ${SOURCES} src/oamlC.cpp)
endif()

if (ENABLE_SHARED)
	add_library(oaml_shared SHARED ${SOURCES} src/oamlC.cpp)
	target_link_libraries(oaml_shared ${OAML_LIBS})
endif()


##
# Build AudioPluginOAML (for Unity) if we ENABLE_UNITYPLUGIN is TRUE
#
if (ENABLE_UNITYPLUGIN)
	if (APPLE)
		add_library(AudioPluginOAML MODULE ${SOURCES} src/oamlUnityPlugin.cpp)
		set_target_properties(AudioPluginOAML PROPERTIES
			BUNDLE_EXTENSION bundle
			BUNDLE TRUE)
	else()
		add_library(AudioPluginOAML MODULE ${SOURCES} src/oamlUnityPlugin.cpp)
	endif()
	target_link_libraries(AudioPluginOAML ${OAML_LIBS})
endif()


##
# Install rules
#
if (ENABLE_STATIC)
	install(TARGETS oaml DESTINATION lib)
endif()

if (ENABLE_SHARED)
	install(TARGETS oaml_shared DESTINATION lib)
endif()

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/oaml.h DESTINATION include)
