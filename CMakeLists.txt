cmake_minimum_required(VERSION 3.0)
project(CameraManagerCore)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
add_subdirectory(${CMAKE_SOURCE_DIR}/modules/CameraRecorder)

add_definitions(-std=c++17
#        -O3
        -ggdb -Og

#        -fsanitize=thread
#        -fsanitize=address -fsanitize=leak -fsanitize=undefined

        -fmax-errors=1 -Wall -Wextra -Wpedantic -Werror # -Wodr
        -Wno-unused-variable -Wno-unused-parameter -Wno-missing-field-initializers -Wno-trigraphs
#        -fconcepts
)

set(PROJECT_NAME CameraManagerCore)
set(TARGET_PROJECT_MAIN CameraManagerCore)
set(TARGET_PROJECT_TEST CameraManagerCoreTests)
set(GSOAP_ROOT /usr/local)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/modules/)
set(RTTR_DIR /usr/local)

add_definitions(-DBOOST_LOG_DYN_LINK)
add_definitions(-DBOOST_NO_AUTO_PTR)
add_definitions(-DBOOST_COROUTINES_NO_DEPRECATION_WARNING)
add_definitions(-DBOOST_COROUTINE_NO_DEPRECATION_WARNING)
find_package(Boost REQUIRED COMPONENTS system iostreams filesystem date_time
                                       context thread log log_setup)
find_package(GSOAP REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(ZLIB REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)
find_package(OnvifWrapper REQUIRED)
find_package(RTTR CONFIG REQUIRED Core)

file(GLOB_RECURSE SOURCE_FILES "source/*.cpp")

set(TEST_SOURCE_FILES
        ${SOURCE_FILES}
        test_main.cpp
        test/DBAdapterTest.cpp
        test/DBStatementsTest.cpp
        test/SessionTests.cpp
        test/ExecutionWrapperTest.cpp
        test/ThreadPoolTest.cpp
        test/SignalTest.cpp
        test/IssIntegrationTest.cpp
        test/CameraContextTest.cpp
        test/SubscribersTest.cpp
        test/ModelContextTest.cpp
        test/EventContextTest.cpp
        test/CameraProcessorsTest.cpp
        test/HttpSchemasTest.cpp
        test/PasswordCryptor.cpp)

set(MAIN_SOURCE_FILES
        ${SOURCE_FILES}
        main.cpp)

set(LIBS
        RTTR::Core
        ${ONVIFWRAPPER_SHARED_LIBRARY}
        ${GSOAP_SSL_CXX_LIBRARIES}
        ${OPENSSL_LIBRARIES}
        ${SQLITE3_LIBRARIES}
        ${Boost_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        ${CMAKE_DL_LIBS}
        ${ZLIB_LIBRARIES}
        ${ONVIFGSOAP_SHARED_LIBRARY})

add_executable(${TARGET_PROJECT_MAIN} ${MAIN_SOURCE_FILES})
add_executable(${TARGET_PROJECT_TEST} ${TEST_SOURCE_FILES})
target_link_libraries(${TARGET_PROJECT_MAIN} ${LIBS})
target_link_libraries(${TARGET_PROJECT_TEST} ${LIBS})