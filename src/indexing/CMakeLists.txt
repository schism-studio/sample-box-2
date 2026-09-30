add_library(SampleBox_Indexing
  CoverArtResolver.cpp
  LibraryScanner.cpp
)

target_include_directories(SampleBox_Indexing PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# juce_audio_formats is deliberately absent: the scan no longer opens files, so
# it has no use for AudioFormatManager. Keeping the link would also drag
# juce_audio_basics into the unit test build for nothing.
target_link_libraries(SampleBox_Indexing PUBLIC
  SampleBox_Core
  juce::juce_core
  juce::juce_events
)
