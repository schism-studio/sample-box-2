juce_add_gui_app(SampleBox_Standalone
  PRODUCT_NAME "Sample Box"
)

target_sources(SampleBox_Standalone PRIVATE
  Main.cpp
  MainWindow.cpp
)

target_link_libraries(SampleBox_Standalone PRIVATE
  SampleBox_Core
  SampleBox_Indexing
  SampleBox_Audio
  SampleBox_UI
  juce::juce_gui_basics
  juce::juce_data_structures
)
