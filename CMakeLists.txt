cmake_minimum_required(VERSION 3.16)

project(CKFontEditor VERSION 0.8 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)
qt_standard_project_setup(
    I18N_SOURCE_LANGUAGE cn
)

add_subdirectory(ckfont)

if(MSVC)
    add_compile_options(/utf-8)
endif()

file(GLOB_RECURSE SOURCE
    dialog/*.h dialog/*.cpp dialog/*.ui
    control/*.h control/*.cpp
    component/*.h component/*.cpp
)
add_executable(CKFontEditor
    res.qrc
    pch.h
    main.cpp
    mainwindow.h mainwindow.cpp mainwindow.ui
    flowlayout.h flowlayout.cpp
    appmodel.h appmodel.cpp

    ${SOURCE}
)
target_precompile_headers(CKFontEditor PRIVATE pch.h)
target_include_directories(CKFontEditor PRIVATE .)
target_link_libraries(CKFontEditor PRIVATE Qt${QT_VERSION_MAJOR}::Widgets ckfont)
target_compile_definitions(CKFontEditor PRIVATE
    -DPRJ_VERSION="${PROJECT_VERSION}"
    -DPRJ_LICENCE="MIT"
    -DPRJ_ARCH="${CMAKE_SYSTEM_PROCESSOR}"
    -DPRJ_REPOS="https://gitee.com/chenke404/ckfont-editor|"
)

# Translations
qt_add_translations(CKFontEditor
    TS_FILES
        translations/ckfonteditor_en_us.ts   # 美式英语翻译文件
    LUPDATE_TARGET tr_update
    LRELEASE_TARGET tr_release
)

add_dependencies(CKFontEditor tr_update tr_release)
