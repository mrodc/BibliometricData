cmake_minimum_required(VERSION 3.31)
project(phDserver)

set(CMAKE_CXX_STANDARD 20)

include_directories(
        /usr/local/include
        /usr/local/lib
        /usr/local/lib64
        headers
)

set(CMAKE_PREFIX_PATH /usr/local/lib ..)
find_package(Boost  REQUIRED)

find_package(PkgConfig)
pkg_check_modules(LIBBSONXX REQUIRED libbsoncxx)
pkg_check_modules(LIBMONGOCXX REQUIRED libmongocxx)

set(POCO_PREFIX "/usr/local") # the directory containing "include" and "lib"
set(POCO_INCLUDE_DIR "${POCO_PREFIX}/include")
set(POCO_LIB_DIR "${POCO_PREFIX}/lib")
set(POCO_LIBS
        "${POCO_LIB_DIR}/libPocoNet.so"
        "${POCO_LIB_DIR}/libPocoUtil.so"
        "${POCO_LIB_DIR}/libPocoFoundation.so"
        "${POCO_LIB_DIR}/libPocoXML.so"
        "${POCO_LIB_DIR}/libPocoJSON.so"

)
# set the include path for the app
include_directories(${POCO_INCLUDE_DIR})

include_directories("/usr/local/include/bsoncxx/v_noabi")
include_directories("/usr/local/include/mongocxx/v_noabi")


include_directories( "${POCO_PREFIX}/include")


include_directories(${LIBMONGOCXX_INCLUDE_DIRS} ${LIBBSONCXX_INCLUDE_DIRS})
link_directories(${LIBMONGOCXX_LIBRARY_DIRS} ${LIBBSONCXX_LIBRARY_DIRS})
add_executable(
        ${PROJECT_NAME}
        headers/globalHeaders.h
        src/ServerFunction.cpp
        headers/ServerFunction.h
        src/main.cpp
        headers/Session.h
        src/Listener.cpp
        headers/Listener.h
        src/UserTimer.cpp
        headers/UserTimer.h
        src/Session.cpp
        headers/Simulator.h
        src/PhdQueue.cpp
        headers/PhdQueue.h
)


target_link_libraries(${PROJECT_NAME} ${LIBMONGOCXX_LIBRARIES} ${LIBBSONCXX_LIBRARIES})

target_link_libraries(${PROJECT_NAME} "${POCO_LIBS}")

target_link_libraries(${PROJECT_NAME}
        /usr/local/lib64/libmongoc2.so
        /usr/local/lib64/libbson2.so
)
