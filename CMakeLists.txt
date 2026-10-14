cmake_minimum_required(VERSION 3.10)
project(msrp)

set(PACKAGE_NAME "msrp")
set(PACKAGE_VERSION "0.9.5")
set(PACKAGE_URL "http://msrp.github.com")
set(PACKAGE_BUGREPORT "msrp-tool@googlegroups.com")

# Compilation flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Woverloaded-virtual -Wall")

# dist target
set(ARCHIVE_NAME ${CMAKE_PROJECT_NAME}-${PACKAGE_VERSION})
add_custom_target(dist
    COMMAND git archive --prefix=${ARCHIVE_NAME}/ HEAD
        | bzip2 -9 > ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar.bz2
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# Dependencies
include(FindPkgConfig)
pkg_check_modules(PCRE REQUIRED libpcre2-8)

# Sources
add_subdirectory(src)
add_subdirectory(doc)
add_subdirectory(test)

# Install
install(FILES README
  DESTINATION share/doc/msrp
  )
