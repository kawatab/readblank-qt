cmake_minimum_required(VERSION 3.21)

# Use the command below for activating after modification
# ```
# cmake -S . -B build -G Ninja
# ```

project(ReadBlankQt VERSION 0.1 LANGUAGES CXX)

# For standard C++
set(PROJECT_DISPLAY_NAME "ReadBlank Qt")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# For LibXml2
find_package(LibXml2 REQUIRED)

# For QML/C++
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 REQUIRED COMPONENTS
  Quick
  Network
  Multimedia
  LinguistTools
)

qt_standard_project_setup(REQUIRES 6.5)

qt_policy(SET QTP0004 NEW)

# C++ source codes
qt_add_executable(ReadBlankQt
    src/main.cpp

    src/model/ContentModel.h
    src/model/ContentModel.cpp
    src/model/FavoriteListModel.h
    src/model/FavoriteListModel.cpp

    src/controller/ContentController.h
    src/controller/ContentController.cpp
    src/controller/LanguageController.h
    src/controller/LanguageController.cpp
    src/controller/SpeechController.h
    src/controller/SpeechController.h
    src/controller/SpeechController.cpp
)

# QML source codes
qt_add_qml_module(ReadBlankQt
    URI ReadBlankQt
    VERSION 1.0

    QML_FILES
        qml/Main.qml
)

# ts files for Qt Linguist
# Update ts files
# ```
# cmake --build build --target update_translations
# ```
# Edit ts files
# ```
# linguist6 translations/readblank-qt_hu.ts
# ```
qt_add_translations(ReadBlankQt
    TS_FILES
        translations/readblank-qt_hu.ts
        translations/readblank-qt_ja.ts
        translations/readblank-qt_fr.ts
    RESOURCE_PREFIX "/i18n"
)

# Link Libraries
target_link_libraries(ReadBlankQt
    PRIVATE
        Qt6::Quick
        Qt6::Network
        Qt6::Multimedia
	LibXml2::LibXml2
)

# Target properties
set_target_properties(ReadBlankQt PROPERTIES
    OUTPUT_NAME ReadBlankQtApp
)
