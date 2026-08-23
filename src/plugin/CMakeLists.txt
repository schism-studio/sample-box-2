# NOTE: PLUGIN_MANUFACTURER_CODE / PLUGIN_CODE below are placeholders.
# Replace with Schism Studio's registered Steinberg codes before a real
# release build if/when one is obtained.
juce_add_plugin(SampleBox_VST3
  COMPANY_NAME "Schism Studio"
  PRODUCT_NAME "Sample Box"
  BUNDLE_ID "com.schismstudio.samplebox"
  PLUGIN_MANUFACTURER_CODE Schs
  PLUGIN_CODE Sbox
  FORMATS VST3
  VST3_CATEGORIES Tools
  IS_SYNTH FALSE
  NEEDS_MIDI_INPUT FALSE
  NEEDS_MIDI_OUTPUT FALSE
  IS_MIDI_EFFECT FALSE
  EDITOR_WANTS_KEYBOARD_FOCUS FALSE
)

target_sources(SampleBox_VST3 PRIVATE
  PluginEditor.cpp
  PluginProcessor.cpp
)

target_include_directories(SampleBox_VST3 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(SampleBox_VST3 PRIVATE
  SampleBox_Core
  SampleBox_Indexing
  SampleBox_Audio
  SampleBox_UI
  SampleBox_Export
  juce::juce_audio_processors
  juce::juce_audio_utils
  juce::juce_data_structures
)

target_compile_definitions(SampleBox_VST3 PUBLIC
  JUCE_WEB_BROWSER=0
  JUCE_USE_CURL=0
  JUCE_VST3_CAN_REPLACE_VST2=0
)
