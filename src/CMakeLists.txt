# MemoApp src

option(MEMOAPP_USE_MYSQL "Compile & link legacy MySQL report backend (ReportDB.cpp + libmysql)" OFF)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SrcRoot)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/nodes SrcNodes)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/data SrcData)

set(SQLITE_DIR ${CMAKE_SOURCE_DIR}/third_party/sqlite)
set(MYSQL_DIR ${CMAKE_SOURCE_DIR}/third_party/mysql)

# 默认（MEMOAPP_USE_MYSQL=OFF）：不编译 ReportDB.cpp，不链 MySQL —— 零 DLL 依赖的单 exe
# 需要切回 MySQL 版周报时：cmake -DMEMOAPP_USE_MYSQL=ON ..
if(NOT MEMOAPP_USE_MYSQL)
    list(REMOVE_ITEM SrcData ${CMAKE_CURRENT_SOURCE_DIR}/data/ReportDB.cpp)
endif()

add_executable(MemoApp WIN32 ${SrcRoot} ${SrcNodes} ${SrcData} ${SQLITE_DIR}/sqlite3.c MemoApp.rc)

target_include_directories(MemoApp PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/third_party/DuiLib
    ${SQLITE_DIR})

if(MEMOAPP_USE_MYSQL)
    target_include_directories(MemoApp PRIVATE ${MYSQL_DIR}/include)
endif()

# sqlite3.c 为第三方 C 源，屏蔽其编译警告
set_source_files_properties(${SQLITE_DIR}/sqlite3.c PROPERTIES COMPILE_OPTIONS "/w0")

target_compile_options(MemoApp PRIVATE /utf-8)
target_compile_definitions(MemoApp PRIVATE UILIB_STATIC)

target_link_libraries(MemoApp
    DuiLib
    user32.lib gdi32.lib comctl32.lib
    version.lib advapi32.lib)

if(MEMOAPP_USE_MYSQL)
    # MySQL C API：静态 mysqlclient.lib（运行时仍需 bin/ 下的 libssl-3-x64.dll 等 OpenSSL DLL）
    target_link_libraries(MemoApp
        ${MYSQL_DIR}/lib/mysqlclient.lib
        ws2_32.lib secur32.lib crypt32.lib shlwapi.lib)
endif()

set_target_properties(MemoApp PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")

set(APP_OUT_DIR ${CMAKE_SOURCE_DIR}/out)
file(MAKE_DIRECTORY ${APP_OUT_DIR})
add_custom_command(TARGET MemoApp POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/MemoApp.exe ${APP_OUT_DIR}/MemoApp.exe
  COMMENT "Copy MemoApp.exe to out/")

if(MEMOAPP_USE_MYSQL)
  add_custom_command(TARGET MemoApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      ${MYSQL_DIR}/lib/libmysql.dll ${APP_OUT_DIR}/libmysql.dll)
endif()
