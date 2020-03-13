

add_library(hello_plugin SHARED hello.cc)

find_package(JANA REQUIRED)
find_package(ROOT)

target_include_directories(hello_plugin PUBLIC ${JANA_INCLUDE_DIR} ${ROOT_INCLUDE_DIRS})
target_link_libraries(hello_plugin ${JANA_LIB} ${ROOT_LIBRARIES})
set_target_properties(hello_plugin PROPERTIES PREFIX "" OUTPUT_NAME "hello" SUFFIX ".so")
install(TARGETS hello_plugin DESTINATION plugins)

