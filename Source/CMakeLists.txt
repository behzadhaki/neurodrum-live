cmake_minimum_required(VERSION 3.15)

project(NeuralDrumLive VERSION 0.0.1)

set(BaseTargetName NeuralDrumLive)

juce_add_plugin("${BaseTargetName}"
        COMPANY_NAME "MusicTechnologyGroup"
        IS_SYNTH FALSE
        NEEDS_MIDI_INPUT TRUE
        NEEDS_MIDI_OUTPUT FALSE
        IS_MIDI_EFFECT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        COPY_PLUGIN_AFTER_BUILD TRUE
        PLUGIN_MANUFACTURER_CODE Juce
        PLUGIN_CODE NPtm
        FORMATS VST3
        PRODUCT_NAME "NeuralDrumLive"
)

# ==============================================================================
# Platform Detection
# ==============================================================================
message(STATUS "System name: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Processor: ${CMAKE_SYSTEM_PROCESSOR}")

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    if(CMAKE_OSX_ARCHITECTURES MATCHES "x86_64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
        set(ONNX_ARCHIVE_NAME "onnxruntime-osx-x86_64-static_lib-1.22.0.zip")
        set(ONNX_SUBDIR_NAME "onnxruntime-osx-x86_64-static_lib-1.22.0")
    else()
        set(ONNX_ARCHIVE_NAME "onnxruntime-osx-arm64-static_lib-1.20.0.zip")
        set(ONNX_SUBDIR_NAME "onnxruntime-osx-arm64-static_lib-1.20.0")
    endif()
elseif(WIN32)
    set(ONNX_ARCHIVE_NAME "onnxruntime-win-x86-static_lib-1.22.0.tar.bz2")
    set(ONNX_SUBDIR_NAME "onnxruntime-win-x86-static_lib-1.22.0")
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

set(ONNX_ZIP_FILE "${CMAKE_SOURCE_DIR}/${ONNX_ARCHIVE_NAME}")
set(ONNX_UNPACK_DIR "${CMAKE_BINARY_DIR}/external/onnxruntime_prebuilt")
set(ONNX_ROOT_DIR "${ONNX_UNPACK_DIR}/${ONNX_SUBDIR_NAME}")

# ==============================================================================
# Unpack ONNX Runtime (once at configure time)
# ==============================================================================
if(NOT EXISTS "${ONNX_ROOT_DIR}/include/onnxruntime_c_api.h")
    message(STATUS "Unpacking ONNX Runtime...")

    file(MAKE_DIRECTORY "${ONNX_UNPACK_DIR}")

    if(WIN32)
        execute_process(
                COMMAND ${CMAKE_COMMAND} -E tar xjf "${ONNX_ZIP_FILE}"
                WORKING_DIRECTORY "${ONNX_UNPACK_DIR}"
                RESULT_VARIABLE UNPACK_RESULT
        )
    else()
        execute_process(
                COMMAND unzip -q "${ONNX_ZIP_FILE}" -d "${ONNX_UNPACK_DIR}"
                RESULT_VARIABLE UNPACK_RESULT
        )
    endif()

    if(NOT UNPACK_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to unpack ONNX Runtime archive.")
    endif()
endif()

# ==============================================================================
# Add Plugin Target Sources
# ==============================================================================
target_sources(${BaseTargetName} PRIVATE
        PluginProcessor.cpp
        PluginEditor.cpp
        InferenceThreadJob.cpp
        AudioBufferSampler.cpp
)

target_compile_definitions(${BaseTargetName}
        PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_include_directories(${PROJECT_NAME} PRIVATE
        "${ONNX_ROOT_DIR}/include"
)

if(WIN32)
    target_link_libraries(${BaseTargetName} PRIVATE
            "${ONNX_ROOT_DIR}/lib/onnxruntime.lib"
            shared_plugin_helpers
            juce_recommended_config_flags
            juce_recommended_lto_flags
            juce_recommended_warning_flags
    )
else()
    target_link_libraries(${BaseTargetName} PRIVATE
            "${ONNX_ROOT_DIR}/lib/libonnxruntime.a"
            shared_plugin_helpers
            juce_recommended_config_flags
            juce_recommended_lto_flags
            juce_recommended_warning_flags
    )
endif()

# ==============================================================================
# Embed ONNX Model
# ==============================================================================
set(MODEL_FILENAME "log_kicks_full.onnx")
set(MODEL_SRC "${CMAKE_SOURCE_DIR}/models/${MODEL_FILENAME}")
set(MODEL_RELATIVE_PATH "Contents/Resources/${MODEL_FILENAME}")
set(MODEL_DEST "${CMAKE_BINARY_DIR}/Source/${PROJECT_NAME}_artefacts/${CMAKE_BUILD_TYPE}/VST3/${PROJECT_NAME}.vst3/Contents/Resources")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${MODEL_DEST}"
        COMMAND ${CMAKE_COMMAND} -E copy "${MODEL_SRC}" "${MODEL_DEST}/${MODEL_FILENAME}"
)

# ==============================================================================
# Generate ModelPathConfig.h
# ==============================================================================
configure_file(
        "${CMAKE_SOURCE_DIR}/ModelPathConfig.h.in"
        "${CMAKE_BINARY_DIR}/ModelPathConfig.h"
        @ONLY
)

target_include_directories(${BaseTargetName} PRIVATE "${CMAKE_BINARY_DIR}")

# ==============================================================================
# Copy Plugin to Local VST3 Folder
# ==============================================================================
if(APPLE)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_BINARY_DIR}/Source/${PROJECT_NAME}_artefacts/${CMAKE_BUILD_TYPE}/VST3/${PROJECT_NAME}.vst3"
            "$ENV{HOME}/Library/Audio/Plug-Ins/VST3/${PROJECT_NAME}.vst3"
    )
endif()

if(WIN32)
    set(VST3_DESTINATION "$ENV{APPDATA}/VST3/${PROJECT_NAME}.vst3")

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_BINARY_DIR}/Source/${PROJECT_NAME}_artefacts/${CMAKE_BUILD_TYPE}/VST3/${PROJECT_NAME}.vst3"
            "${VST3_DESTINATION}"
    )
endif()

message(STATUS "ONNX Runtime headers: ${ONNX_ROOT_DIR}/include")
message(STATUS "ONNX model copied to: ${MODEL_DEST}/${MODEL_FILENAME}")
