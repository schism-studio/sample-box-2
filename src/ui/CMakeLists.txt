add_library(SampleBox_UI
  AnimationClock.cpp
  ArtworkCache.cpp
  BrowserView.cpp
  CoverArtCard.cpp
  CoverArtCarousel.cpp
  DragDropSpike.cpp
  LookAndFeel.cpp
  MainPanel.cpp
  SettingsComponent.cpp
  SidebarBrowser.cpp
)

target_include_directories(SampleBox_UI PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# PUBLIC because MainPanel.h declares the guarded member, so every target that
# includes it has to agree with SampleBox_UI about whether the spike exists.
# A mismatch here would be an ODR violation, not a compile error.
if(SAMPLEBOX_DRAG_SPIKE)
  target_compile_definitions(SampleBox_UI PUBLIC SAMPLEBOX_DRAG_SPIKE=1)
endif()

target_link_libraries(SampleBox_UI PUBLIC
  SampleBox_Core
  juce::juce_events
  juce::juce_graphics
  juce::juce_gui_basics
)
