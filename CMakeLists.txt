cmake_minimum_required(VERSION 3.10)

project(htmlcxx_expand CXX)

option(BUILD_TESTS "Create test target for htmlcxx" ON)

set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

file(GLOB HTML_HEADERS "src/html/*.h")
file(GLOB HTML_SOURCES "src/html/*.cc")

file(GLOB XPATH_HEADERS "src/xpath/*.h")
file(GLOB XPATH_SOURCES "src/xpath/*.cpp")

include_directories("src/html" "src/xpath")

add_compile_definitions(PROJECT_PATH="${CMAKE_CURRENT_SOURCE_DIR}")

add_library(htmlcxx_expand ${HTML_SOURCES} ${XPATH_SOURCES})

target_include_directories(htmlcxx_expand
  PUBLIC
    $<INSTALL_INTERFACE:include>    
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
if(NOT BUILD_SHARED_LIBS)
  target_compile_definitions(htmlcxx_expand PUBLIC "CPPHTML_STATIC")
endif()

install(FILES ${HTML_HEADERS} "src/html/ParserSax.tcc" ${XPATH_HEADERS} DESTINATION include/html)
install(FILES ${XPATH_HEADERS} DESTINATION include/xpath)
install(TARGETS htmlcxx_expand RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)

message(STATUS ${BUILD_TESTS})

if(BUILD_TESTS)
  include_directories("src")
  file(GLOB TEST_SOURCE "test/*.cpp")
  add_executable(test ${TEST_SOURCE})
  target_link_libraries(test htmlcxx_expand)
endif()
