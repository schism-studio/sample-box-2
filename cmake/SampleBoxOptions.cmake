option(SAMPLEBOX_BUILD_TESTS "Build Sample Box tests" ON)

set(SAMPLEBOX_JUCE_DIR "${CMAKE_SOURCE_DIR}/external/JUCE" CACHE PATH "Path to the JUCE checkout")

if(NOT EXISTS "${SAMPLEBOX_JUCE_DIR}/CMakeLists.txt")
  message(FATAL_ERROR "JUCE was not found at ${SAMPLEBOX_JUCE_DIR}. Initialize external/JUCE before configuring.")
endif()

add_subdirectory("${SAMPLEBOX_JUCE_DIR}" JUCE)
