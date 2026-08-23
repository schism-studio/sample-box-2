add_library(SampleBox_Audio
  PreviewEngine.cpp
  WaveformCache.cpp
)

target_include_directories(SampleBox_Audio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(SampleBox_Audio PUBLIC
  SampleBox_Core
  juce::juce_audio_basics
  juce::juce_audio_formats
  juce::juce_audio_utils
)
