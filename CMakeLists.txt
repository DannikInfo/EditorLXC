cmake_minimum_required(VERSION 3.21)
project(EditorLXC)

set(CMAKE_CXX_STANDARD 23)
set(wxWidgets_CONFIGURATION mswu)

find_package(wxWidgets COMPONENTS core base REQUIRED)
include(${wxWidgets_USE_FILE})
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

include(CMakeRC)
cmrc_add_resource_library(xcle-resources
        ALIAS xcle::rc
        NAMESPACE xcle
        assets/logo.png)

add_executable(EditorLXC main.cpp mainFrame.cpp mainFrame.h idEnum.h  JSONDataModel.cpp JSONDataModel.h resourceLoader.cpp resourceLoader.h configPathes.h)


target_link_libraries(EditorLXC xcle::rc)

target_link_libraries(EditorLXC ${wxWidgets_LIBRARIES})

target_link_libraries(EditorLXC libtinyLogger.so)
target_link_libraries(EditorLXC libtinyConfig.so)