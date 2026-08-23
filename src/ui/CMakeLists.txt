add_library(SampleBox_UI
  AnimationClock.cpp
  ArtworkCache.cpp
  BrowserView.cpp
  CoverArtCard.cpp
  CoverArtCarousel.cpp
  LookAndFeel.cpp
  MainPanel.cpp
  PopoutWindow.cpp
  SettingsComponent.cpp
  SidebarBrowser.cpp
)

target_include_directories(SampleBox_UI PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(SampleBox_UI PUBLIC
  SampleBox_Core
  juce::juce_events
  juce::juce_graphics
  juce::juce_gui_basics
)
