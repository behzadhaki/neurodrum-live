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
# ONNX Runtime Prebuilt
# ==============================================================================
set(ONNX_ZIP_FILE "${CMAKE_SOURCE_DIR}/onnxruntime-osx-arm64-static_lib-1.20.0.zip")
set(ONNX_UNPACK_DIR "${CMAKE_BINARY_DIR}/external/onnxruntime_prebuilt")
set(ONNX_ROOT_DIR "${ONNX_UNPACK_DIR}/onnxruntime-osx-arm64-static_lib-1.20.0")

# ==============================================================================
# Unzip ONNX Runtime Archive (once at configure time)
# ==============================================================================
if(NOT EXISTS "${ONNX_ROOT_DIR}/include/onnxruntime_c_api.h")
    message(STATUS "Unzipping ONNX Runtime...")
    file(MAKE_DIRECTORY "${ONNX_UNPACK_DIR}")
    execute_process(
            COMMAND unzip -q "${ONNX_ZIP_FILE}" -d "${ONNX_UNPACK_DIR}"
            RESULT_VARIABLE UNZIP_RESULT
    )
    if(NOT UNZIP_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to unzip ONNX Runtime archive.")
    endif()
endif()

# ==============================================================================
# Add Plugin Target
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

target_link_libraries(${BaseTargetName} PRIVATE
        "${ONNX_ROOT_DIR}/lib/libonnxruntime.a"
        shared_plugin_helpers
        juce_recommended_config_flags
        juce_recommended_lto_flags
        juce_recommended_warning_flags
)

# ==============================================================================
# Embed ONNX Model
# ==============================================================================
set(MODEL_SRC "${CMAKE_SOURCE_DIR}/models/log_kicks_full.onnx")
set(MODEL_DEST "${CMAKE_BINARY_DIR}/PluginCode/${PROJECT_NAME}_artefacts/${CMAKE_BUILD_TYPE}/VST3/${PROJECT_NAME}.vst3/Contents/Resources")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${MODEL_DEST}"
        COMMAND ${CMAKE_COMMAND} -E copy "${MODEL_SRC}" "${MODEL_DEST}/log_kicks_full.onnx"
)

# ==============================================================================
# Copy Plugin to Local VST3 Folder
# ==============================================================================
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_BINARY_DIR}/PluginCode/${PROJECT_NAME}_artefacts/${CMAKE_BUILD_TYPE}/VST3/${PROJECT_NAME}.vst3"
        "$ENV{HOME}/Library/Audio/Plug-Ins/VST3/${PROJECT_NAME}.vst3"
)

message(STATUS "ONNX Runtime headers: ${ONNX_ROOT_DIR}/include")
message(STATUS "ONNX model copied to: ${MODEL_DEST}/log_kicks_full.onnx")
