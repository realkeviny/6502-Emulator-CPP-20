include(Catch)

if(ENABLE_TESTING)
  set(TEST_MAIN "unit_tests")
  set(TEST_SOURCES "test_6502_load.cpp" "test_6502_store.cpp")
  set(TEST_INCLUDES "./")

  add_executable(${TEST_MAIN} ${TEST_SOURCES})
  target_include_directories(${TEST_MAIN} PUBLIC ${TEST_INCLUDES})
  target_link_libraries(
    ${TEST_MAIN} PUBLIC ${PROCESSOR_NAME} ${DISASSEMBLER_NAME}
                        Catch2::Catch2WithMain)

  catch_discover_tests(${TEST_MAIN})
endif()
