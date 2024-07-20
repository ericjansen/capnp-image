
cmake_minimum_required(VERSION 3.5)
project("capnp-image" VERSION 0.2 LANGUAGES CXX)

find_package(CapnProto CONFIG REQUIRED)
find_package(OpenCV REQUIRED)

capnp_generate_cpp(imageSources imageHeaders image.capnp)
add_executable(image-client client.cpp ${imageSources})
add_executable(image-server server.cpp ${imageSources})
target_link_libraries(image-client PRIVATE CapnProto::capnp-rpc)
target_link_libraries(image-client PRIVATE ${OpenCV_LIBS})
target_link_libraries(image-server PRIVATE CapnProto::capnp-rpc)
target_link_libraries(image-server PRIVATE ${OpenCV_LIBS})
target_include_directories(image-client PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(image-server PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
