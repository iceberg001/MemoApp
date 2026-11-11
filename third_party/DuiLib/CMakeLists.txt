# DuiLib third_party build (MemoApp)
# 在原版基础上增加 DUILIB_STATIC 选项：静态编译时排除 DllMain(UILib.cpp)，
# 并定义 UILIB_STATIC，使 UILIB_API 为空宏，可直接链接进 exe。

option(DUILIB_STATIC "Build DuiLib as static library" ON)

# DuiLib 源码为 GBK 编码且使用裸 byte 类型，保持 C++11 与系统默认代码页编译
set(CMAKE_CXX_STANDARD 11)

add_definitions(-DCMAKE)
add_definitions(-DUNICODE -D_UNICODE)
add_compile_options(/W3 /WX-)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} RootSrc)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/Control ControlSrc)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/Core CoreSrc)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/Core/Plugin PluginSrc)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/Layout LayoutSrc)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/Utils UtilsSrc)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Control)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Core)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Core/Plugin)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Layout)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Utils)

if(DUILIB_STATIC)
    add_definitions(-DUILIB_STATIC)
    list(REMOVE_ITEM RootSrc "${CMAKE_CURRENT_SOURCE_DIR}/UILib.cpp")
    add_library(DuiLib STATIC ${ControlSrc} ${CoreSrc} ${PluginSrc} ${LayoutSrc} ${UtilsSrc} ${RootSrc})
else()
    add_definitions(-DUILIB_EXPORTS)
    add_library(DuiLib SHARED ${ControlSrc} ${CoreSrc} ${PluginSrc} ${LayoutSrc} ${UtilsSrc} ${RootSrc})
endif()

target_link_libraries(DuiLib comctl32)
