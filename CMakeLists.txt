cmake_minimum_required(VERSION 3.21)

project(ReadBlankQt VERSION 0.1 LANGUAGES CXX)

set(PROJECT_DISPLAY_NAME "ReadBlank Qt")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Network Multimedia)
find_package(LibXml2 REQUIRED)

qt_standard_project_setup(REQUIRES 6.5)

qt_policy(SET QTP0004 NEW)

# Use the command below for activating after modification.
# cmake -S . -B build -G Ninja
# or
# cmake -S . -B build -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
qt_add_executable(ReadBlankQt
    src/main.cpp

    src/model/ContentModel.h
    src/model/ContentModel.cpp
    src/model/FavoriteListModel.h
    src/model/FavoriteListModel.cpp

    src/controller/ContentController.h
    src/controller/ContentController.cpp
    src/controller/SpeechController.h
    src/controller/SpeechController.cpp
)

qt_add_qml_module(ReadBlankQt
    URI ReadBlankQt
    VERSION 1.0

    QML_FILES
        qml/Main.qml
)

target_link_libraries(ReadBlankQt
    PRIVATE
        Qt6::Quick
        Qt6::Network
        Qt6::Multimedia
	LibXml2::LibXml2
)

set_target_properties(ReadBlankQt PROPERTIES
    OUTPUT_NAME ReadBlankQtApp
)
