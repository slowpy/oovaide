# Generated by oovCMaker
add_executable(oovCppParser CppParser.cpp IncDirMap.cpp ModelWriter.cpp 
  oovCppParser.cpp ParseBase.cpp ParserModelData.cpp)

target_link_libraries(oovCppParser oovCommon ${LLVM_LIBRARIES})

install(TARGETS oovCppParser
  EXPORT OovaideTargets
  RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT lib)
