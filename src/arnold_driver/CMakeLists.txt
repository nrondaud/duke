

# Arnold
find_package(Arnold)
include_directories(${ARNOLD_INCLUDE_DIR})


if(ARNOLD_FOUND)

	# Sources
	file(GLOB_RECURSE DRIVER_SRCS *.cpp *.cxx *.cc *.C *.c *.h *.hpp)

	# Target name
	set(DRIVER duke_driver)

	# Protobuf
	protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROJECT_SOURCE_DIR}/src/duke/protocol/duke.proto)
	include_directories("${CMAKE_CURRENT_BINARY_DIR}")

	# Link rules
	include_directories(${PROJECT_SOURCE_DIR}/src/duke)
	add_library(${DRIVER} SHARED ${DRIVER_SRCS} ${PROTO_SRCS})
	target_link_libraries(${DRIVER} ${ARNOLD_LIBRARY} ${ZEROMQ_LIBRARIES} ${PROTOBUF_LIBRARY})
	
endif()
