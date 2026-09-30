option(SAMPLEBOX_BUILD_TESTS "Build Sample Box tests" ON)

# TEMPORARY. Shows the drag-out diagnostic panel (src/ui/DragDropSpike) in
# both the standalone and the VST3 editor. On by default only while the
# question "will a DAW accept a file drag started from a plug-in editor?" is
# open; see the header comment in DragDropSpike.h. Turn off with
# -DSAMPLEBOX_DRAG_SPIKE=OFF, and delete the option along with the component
# once the answer is recorded.
option(SAMPLEBOX_DRAG_SPIKE "Show the temporary drag-out diagnostic panel" ON)

set(SAMPLEBOX_JUCE_DIR "${CMAKE_SOURCE_DIR}/external/JUCE" CACHE PATH "Path to the JUCE checkout")

if(NOT EXISTS "${SAMPLEBOX_JUCE_DIR}/CMakeLists.txt")
  message(FATAL_ERROR "JUCE was not found at ${SAMPLEBOX_JUCE_DIR}. Initialize external/JUCE before configuring.")
endif()

add_subdirectory("${SAMPLEBOX_JUCE_DIR}" JUCE)
