#Copyright 2016-2018 hyperchain.net (Hyperchain)
#Distributed under the MIT software license, see the accompanying
#file COPYING or https://opensource.org/licenses/MIT.

#Permission is hereby granted, free of charge, to any person obtaining a copy of this 
#software and associated documentation files (the "Software"), to deal in the Software
#without restriction, including without limitation the rights to use, copy, modify, merge,
#publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons
#to whom the Software is furnished to do so, subject to the following conditions:

#The above copyright notice and this permission notice shall be included in all copies or
#substantial portions of the Software.

#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
#INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
#PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE

#FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
#OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
#DEALINGS IN THE SOFTWARE.
#

CMAKE_MINIMUM_REQUIRED(VERSION 3.7)

set(CMAKE_CXX_STANDARD 11)

if(UNIX)
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")  
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall") 

elseif(WIN32)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /MD")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od /MDd")
SET( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /level='requireAdministrator' /uiAccess='false'" )

endif()
	
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

PROJECT(hc)

set(BIN_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

IF( NOT CMAKE_BUILD_TYPE )
   SET( CMAKE_BUILD_TYPE Release)
ENDIF()
message(STATUS "Build Type: " "${CMAKE_BUILD_TYPE}")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if(IS_DIRECTORY ${CMAKE_SOURCE_DIR}/admin)
	add_subdirectory(admin)
endif(IS_DIRECTORY ${CMAKE_SOURCE_DIR}/admin)

#include current directory
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(SQLite3 REQUIRED)
if(SQLite3_FOUND)
    message(STATUS "Found SQLite3")
endif()

find_package(Qt5 REQUIRED COMPONENTS Core Widgets WebEngineWidgets WebChannel)
if(Qt5_FOUND)
    message(STATUS "Found Qt5")
endif()

find_package(cpprestsdk REQUIRED)
if(cpprestsdk_FOUND)
    message(STATUS "Found cpprestsdk")
endif()


find_package (Boost REQUIRED COMPONENTS system)

cmake_policy(SET CMP0071 NEW)

set(CMAKE_AUTOMOC TRUE)
set(CMAKE_AUTORCC TRUE)

set(main_SRC mainwindow.cpp singleapp.cpp main.cpp)
set(main_HEAD mainwindow.h singleapp.h)
set(main_UI mainwindow.ui)

FILE(GLOB customui_SRC "customui/*.cpp")
FILE(GLOB customui_HEAD "customui/*.h")
FILE(GLOB customui_UI "customui/*.ui") 

FILE(GLOB wnd_SRC "wnd/*.cpp")
FILE(GLOB wnd_HEAD "wnd/*.h")
FILE(GLOB wnd_UI "wnd/*.ui") 

FILE(GLOB db_SRC "db/*.cpp")
FILE(GLOB db_HEAD "db/*.h")

FILE(GLOB util_SRC "util/*.cpp")
FILE(GLOB util_HEAD "util/*.h")

FILE(GLOB channel_SRC "channel/*.cpp")
FILE(GLOB channel_HEAD "channel/*.h")

FILE(GLOB sqlite3_HEAD "thirdparty/sqlite3/*.h")

#HChainP2PManager
FILE(GLOB HChainP2PManager_SRC "HChainP2PManager/*.cpp")
FILE(GLOB HChainP2PManager_HEAD "HChainP2PManager/*.h")

FILE(GLOB crypto_SRC "HChainP2PManager/crypto/*.cpp")
FILE(GLOB crypto_HEAD "HChainP2PManager/crypto/*.h")

FILE(GLOB data_SRC "HChainP2PManager/data/*.cpp")
FILE(GLOB data_HEAD "HChainP2PManager/data/*.h")

FILE(GLOB debug_SRC "HChainP2PManager/debug/*.cpp")
FILE(GLOB debug_HEAD "HChainP2PManager/debug/*.h")

FILE(GLOB HttpUnit_SRC "HChainP2PManager/HttpUnit/*.cpp")
FILE(GLOB HttpUnit_HEAD "HChainP2PManager/HttpUnit/*.h")

FILE(GLOB interface_SRC "HChainP2PManager/interface/*.cpp")
FILE(GLOB interface_HEAD "HChainP2PManager/interface/*.h")

FILE(GLOB network_SRC "HChainP2PManager/network/*.cpp")
FILE(GLOB network_HEAD "HChainP2PManager/network/*.h")

FILE(GLOB utility_SRC "HChainP2PManager/utility/*.cpp")
FILE(GLOB utility_HEAD "HChainP2PManager/utility/*.h")

set(p2p_SRC ${crypto_SRC} ${data_SRC} ${debug_SRC} ${HttpUnit_SRC} ${interface_SRC} ${network_SRC} ${utility_SRC} ${HChainP2PManager_SRC})
set(p2p_HEAD ${crypto_HEAD} ${data_HEAD} ${debug_HEAD} ${HttpUnit_HEAD} ${interface_HEAD} ${network_HEAD} ${utility_HEAD} ${HChainP2PManager_HEAD})
#end HChainP2PManager

#All source files
set(final_HEAD
    ${main_HEAD}
    ${customui_HEAD}
	${wnd_HEAD}
	${db_HEAD}
	${p2p_HEAD}
	${util_HEAD}
	${channel_HEAD}
	${sqlite3_HEAD}
	)
 
set(final_SRC
    ${main_SRC}
    ${customui_SRC}
	${wnd_SRC}
	${db_SRC}
	${p2p_SRC}
	${util_SRC}
	${channel_SRC}
	)

set(final_UI
    ${customui_UI}
	${main_UI}
	${wnd_UI}
	)
	
set(resource_SRC ${CMAKE_BINARY_DIR}/logo.rc)

qt5_wrap_ui(WRAP_FILES ${final_UI})	
	

source_group("Ui" FILES ${UI_FILES} ${WRAP_FILES} )

if(WIN32)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
endif()

add_executable(${PROJECT_NAME} ${final_SRC} ${final_HEAD} ${WRAP_FILES} ${resource_SRC})

target_link_libraries(${PROJECT_NAME} Qt5::Widgets Qt5::Core Qt5::Gui Qt5::Network Qt5::WebChannel Qt5::WebEngineWidgets)
if(WIN32)
target_link_libraries(${PROJECT_NAME} Qt5::WinMain)
endif()

target_link_libraries(${PROJECT_NAME} cpprestsdk::cpprest Boost::system ${SQLITE3_LIBRARIES})

include(HyperChainCPack.cmake)

configure_file (
"${PROJECT_SOURCE_DIR}/logo.rc.in"
"${PROJECT_BINARY_DIR}/logo.rc")






