add_executable(SampleBox_UnitTests ScannerTests.cpp)

# SampleBox_Core is an INTERFACE (header-only) target, so linking it alone
# resolves the snapshot types but not LibraryScanner's implementation.
target_link_libraries(SampleBox_UnitTests PRIVATE
  SampleBox_Core
  SampleBox_Indexing
)

add_test(NAME SampleBox.Unit COMMAND SampleBox_UnitTests)
