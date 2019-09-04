############################## Project Version ################################

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.1)

SET(DOFPIPE_VERSION_MAJOR 0)
SET(DOFPIPE_VERSION_MINOR 1)
SET(DOFPIPE_VERSION_PATCH 0)
SET(DOFPIPE_VERSION_STRING 
    ${DOFPIPE_VERSION_MAJOR}.${DOFPIPE_VERSION_MINOR}.${DOFPIPE_VERSION_PATCH})
SET(BIN_BUILD_VERSION ${DOFPIPE_VERSION_STRING} CACHE STRING "Build Version")
OPTION(BUILD_VERSION "Set the version being built." ${BIN_BUILD_VERSION})
ADD_DEFINITIONS(-DBUILD_VERSION="${BUILD_VERSION}")
###############################################################################

############################ Project Settings #################################
IF(CMAKE_BUILD_TYPE MATCHES Debug)
    message("debug mode")
    SET(PROJECT dofpipe-dbg)
ELSE()
    message("release mode")
    SET(PROJECT dofpipe)
ENDIF(CMAKE_BUILD_TYPE MATCHES Debug)

PROJECT(dofpipe)
SET(LICENSE "GPLV3")
###############################################################################

########################### Documentation #####################################
SET(MAN_PAGE "docs/dofpipe.8")
###############################################################################

########################### Build and include settings ########################
SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG=1 -g")
STRING(CONCAT CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wall -pedantic")

LINK_DIRECTORIES(
    ${LINK_DIRECTORIES}
)

SET(PIPE_SOURCES 
    "src/dofpipe.c" 
)

ADD_EXECUTABLE(dofpipe ${PIPE_SOURCES})
TARGET_LINK_LIBRARIES(dofpipe lsm9ds1)
TARGET_LINK_LIBRARIES(dofpipe cjson)
TARGET_LINK_LIBRARIES(dofpipe m)

INSTALL(TARGETS dofpipe
    DESTINATION bin
)

INSTALL(FILES ${MAN_PAGE}
    DESTINATION share/man/man8
)

######################## RPM Package ##########################################
#SET(DEPENDENCIES "liblsm9ds1 >= 0.6.0-alpha")
SET(CPACK_RPM_PACKAGE_AUTOREQ OFF)
SET(CPACK_RPM_PACKAGE_REQUIRES ${DEPENDENCIES})
SET(CPACK_RPM_PACKAGE_RELOCATABLE ON)
SET(DESTDIR /)
SET(CPACK_RPM_PACKAGE_ARCHITECTURE "raspberrypi3")
SET(CPACK_PACKAGE_VERSION ${BUILD_VERSION})
SET(CPACK_GENERATOR "RPM;TGZ")
SET(CPACK_PACKAGE_NAME ${PROJECT})
SET(CPACK_PACKAGE_RELEASE 1)
SET(CPACK_PACKAGE_CONTACT "Christopher Jordan-Denny")
SET(CPACK_PACKAGE_VENDOR "Christopher Jordan-Denny")
SET(CPACK_PACKAGING_INSTALL_PREFIX ${DESTDIR})
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Program to output LSM9DS1 data.")
SET(CPACK_RPM_PACKAGE_LICENSE ${LICENSE})
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
STRING(CONCAT CPACK_PACKAGE_FILE_NAME 
    "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}"
    "-${CPACK_RPM_PACKAGE_ARCHITECTURE}")
INCLUDE(CPack)
###############################################################################
