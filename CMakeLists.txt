cmake_minimum_required(VERSION 3.11)
project(ProgSpace)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(${PROJECT_NAME} arucotest.cpp)

add_definitions("-std=c++17")
#Set C++17
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_EXTENSIONS OFF)

#Set flags
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE 
        /W4 /WX)
else()
    #target_compile_options(${PROJECT_NAME} PRIVATE 
    #    -Wall -Wextra -pedantic -Werror)
    target_compile_options(${PROJECT_NAME} PRIVATE 
        -Wall)
endif()

target_link_libraries(${PROJECT_NAME} ${CONAN_LIBS})

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/scripts/ $<TARGET_FILE_DIR:${PROJECT_NAME}>
)