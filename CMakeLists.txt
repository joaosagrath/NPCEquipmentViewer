cmake_minimum_required(VERSION 3.21)

project(NPCEquipmentViewer VERSION 2.0.7 LANGUAGES CXX)

if(DEFINED ENV{SKYRIM_FOLDER} AND IS_DIRECTORY "$ENV{SKYRIM_FOLDER}/Data")
    set(OUTPUT_FOLDER "$ENV{SKYRIM_FOLDER}/Data")
endif()

if(DEFINED ENV{SKYRIM_MODS_FOLDER} AND IS_DIRECTORY "$ENV{SKYRIM_MODS_FOLDER}")
    set(OUTPUT_FOLDER "$ENV{SKYRIM_MODS_FOLDER}/Equipment Viewer and KID Writer")
endif()

find_package(directxtk CONFIG REQUIRED)
find_package(CommonLibSSE CONFIG REQUIRED)

set(SOURCES
    src/PCH.h
    src/Settings.h
    src/Settings.cpp
    src/MessageBox.h
    src/MessageBox.cpp
    src/EquipmentSelectionMenu.h
    src/EquipmentSelectionMenu.cpp
    src/KidWriter.h
    src/KidWriter.cpp
    src/EquipmentMenu.h
    src/EquipmentMenu.cpp
    src/EventHandler.h
    src/EventHandler.cpp
    src/main.cpp
)

add_commonlibsse_plugin(${PROJECT_NAME} SOURCES ${SOURCES})

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
target_compile_definitions(${PROJECT_NAME} PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_precompile_headers(${PROJECT_NAME} PRIVATE src/PCH.h)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SOURCES})

set(PACKAGE_FOLDER "${CMAKE_BINARY_DIR}/package")
set(PACKAGE_PLUGIN_FOLDER "${PACKAGE_FOLDER}/SKSE/Plugins")

add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${PACKAGE_PLUGIN_FOLDER}"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:${PROJECT_NAME}>" "${PACKAGE_PLUGIN_FOLDER}/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/dist/SKSE/Plugins/NPCEquipmentViewer.ini" "${PACKAGE_PLUGIN_FOLDER}/NPCEquipmentViewer.ini"
    VERBATIM
)

if(DEFINED OUTPUT_FOLDER)
    set(DLL_FOLDER "${OUTPUT_FOLDER}/SKSE/Plugins")

    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${DLL_FOLDER}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:${PROJECT_NAME}>" "${DLL_FOLDER}/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/dist/SKSE/Plugins/NPCEquipmentViewer.ini" "${DLL_FOLDER}/NPCEquipmentViewer.ini"
        VERBATIM
    )
endif()
