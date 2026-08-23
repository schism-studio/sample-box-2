add_library(SampleBox_Indexing
  CoverArtResolver.cpp
  LibraryScanner.cpp
)

target_include_directories(SampleBox_Indexing PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(SampleBox_Indexing PUBLIC
  SampleBox_Core
  juce::juce_core
  juce::juce_events
  juce::juce_audio_formats
)
