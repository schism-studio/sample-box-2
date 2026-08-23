add_library(SampleBox_Export
  AbletonHierarchyExporter.cpp
  CuratedSampleSelector.cpp
  FileDragExporter.cpp
)

target_include_directories(SampleBox_Export PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(SampleBox_Export PUBLIC
  SampleBox_Core
  juce::juce_gui_basics
)
