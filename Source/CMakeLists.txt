project(NeuroDrumLive VERSION 0.0.1)

set (BaseTargetName NeuroDrumLive)

add_definitions(-DPROJECT_NAME="${BaseTargetName}")

juce_add_plugin(${BaseTargetName}
        COMPANY_NAME "MusicTechnologyGroup"
        IS_SYNTH TRUE
        NEEDS_MIDI_INPUT TRUE
        NEEDS_MIDI_OUTPUT TRUE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        COPY_PLUGIN_AFTER_BUILD TRUE
        PLUGIN_MANUFACTURER_CODE Juce
        PLUGIN_CODE NRDR
        FORMATS VST3
        PRODUCT_NAME "NeuroDrumLive"
)
#
## ONLY NEEDED FOR LINUX and MACOS (for installing libtorch in root directory)
## You don't need this if you are not planning to distribute the plugin
#set(SUDO_PASSWORD "your_password_here" CACHE STRING "Sudo password")

# DO NOT REMOVE THIS LINE
if (MSVC)
    include(../CMake/CMakeLists.MSVC.txt)
elseif (APPLE)
   include(../CMake/CMakeLists.ARM.txt)
   # include(../CMakeLists.UNIX.txt)
elseif (UNIX)
    include(../CMake/CMakeLists.UNIX.txt)
endif()

message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")