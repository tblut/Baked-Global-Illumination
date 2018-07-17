set( IrrXML_SRCS
  CXMLReaderImpl.h
  heapsort.h
  irrArray.h
  irrString.h
  irrTypes.h
  irrXML.cpp
  irrXML.h
)

add_library(IrrXML STATIC ${IrrXML_SRCS})
target_include_directories(IrrXML PUBLIC ../../include)
target_include_directories(IrrXML PUBLIC .)
