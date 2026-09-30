add_executable(SampleBox_ScannerTests ScannerTests.cpp)
target_link_libraries(SampleBox_ScannerTests PRIVATE
  SampleBox_Core
  SampleBox_Indexing
)
add_test(NAME SampleBox.Scanner COMMAND SampleBox_ScannerTests)

add_executable(SampleBox_StateTests StatePersistenceTests.cpp)
target_link_libraries(SampleBox_StateTests PRIVATE
  SampleBox_Core
  juce::juce_core
  juce::juce_audio_processors
)
add_test(NAME SampleBox.State COMMAND SampleBox_StateTests)

add_executable(SampleBox_BrowseStateTests BrowseStateTests.cpp)
target_link_libraries(SampleBox_BrowseStateTests PRIVATE
  SampleBox_Core
)
add_test(NAME SampleBox.BrowseState COMMAND SampleBox_BrowseStateTests)
