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
#Installation package setting
#

set(APPLICATION_SHORTNAME "HyperChain")
set( APPLICATION_DOMAIN   "hyperchain.com" )

include( VERSION.cmake )

set( CPACK_PACKAGE_VERSION_MAJOR  ${MIRALL_VERSION_MAJOR} )
set( CPACK_PACKAGE_VERSION_MINOR  ${MIRALL_VERSION_MINOR} )
set( CPACK_PACKAGE_VERSION_PATCH  ${MIRALL_VERSION_PATCH} )
set( CPACK_PACKAGE_VERSION_BUILD  ${MIRALL_VERSION_BUILD} )
set( CPACK_PACKAGE_VERSION  ${MIRALL_VERSION_FULL})

set( CPACK_PACKAGE_FILE_NAME  ${APPLICATION_SHORTNAME}-${CPACK_PACKAGE_VERSION}-setup )

set( CPACK_PACKAGE_DESCRIPTION_FILE  "${CMAKE_SOURCE_DIR}/README.md" ) 	# File used as a description of a project     /path/to/project/ReadMe.txt
set( CPACK_PACKAGE_DESCRIPTION_SUMMARY  ${APPLICATION_SHORTNAME} ) 		# Description summary of a project
set( CPACK_PACKAGE_INSTALL_DIRECTORY  ${APPLICATION_SHORTNAME} )     	# Installation directory on the target system -> C:\Program Files\${APPLICATION_SHORTNAME}
set( CPACK_PACKAGE_INSTALL_REGISTRY_KEY ${APPLICATION_SHORTNAME} )  	# Registry key used when installing this project  CMake 2.5.0
set( CPACK_PACKAGE_NAME  ${APPLICATION_SHORTNAME} ) 					# Package name, defaults to the project name
set( CPACK_PACKAGE_HOMEPAGE_URL  "http://${APPLICATION_DOMAIN}" )   	# Package URL for update
set( CPACK_PACKAGE_VENDOR  ${APPLICATION_SHORTNAME} )   				# Package vendor name

set( CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/logo.ico")

set(CORECOMPONET "HyperChainCore")


install(CODE "set(CMAKE_INSTALL_CONFIG_NAME \"${CMAKE_BUILD_TYPE}\")
  message(STATUS \"Install configuration: \${CMAKE_INSTALL_CONFIG_NAME}\")" COMPONENT ${CORECOMPONET})

# Directory for the installed files.  - needed to provide anything to avoid an error# CPACK_INSTALL_COMMANDS  Extra commands to install components.
set( CPACK_TOPLEVEL_TAG "unused" ) 
set(CPACK_IFW_VERBOSE "ON")

# License file for the project, used by the STGZ, NSIS, and PackageMaker generators.
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")

#Wizard style to be used (“Modern”, “Mac”, “Aero” or “Classic”).
set(CPACK_IFW_PACKAGE_WIZARD_STYLE "Modern")
set(CPACK_IFW_PACKAGE_WATERMARK "${CMAKE_SOURCE_DIR}/watermark.png")

include(CPack)
include(CPackIFW)

cpack_add_component(${CORECOMPONET}
    DISPLAY_NAME "HyperChain"
    DESCRIPTION "HyperChain Application"
	)
cpack_ifw_configure_component(${CORECOMPONET}
    SCRIPT "${CMAKE_SOURCE_DIR}/installscript.qs"
	)

cpack_ifw_configure_component(${CORECOMPONET}
	LICENSES "The HyperChain Company GPL 1.0" "${CMAKE_SOURCE_DIR}/LICENSE"
    )

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin COMPONENT ${CORECOMPONET})
install(DIRECTORY conf/ DESTINATION bin 
		FILE_PERMISSIONS OWNER_READ GROUP_READ WORLD_READ OWNER_WRITE GROUP_WRITE WORLD_WRITE 
		COMPONENT ${CORECOMPONET} )

install(DIRECTORY image ui DESTINATION bin 
		FILE_PERMISSIONS OWNER_READ GROUP_READ WORLD_READ
		DIRECTORY_PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE 
		COMPONENT ${CORECOMPONET} )

if(WIN32)
	install(DIRECTORY dependencies/release/win32/ DESTINATION bin COMPONENT ${CORECOMPONET})
elseif(APPLE)
	install(DIRECTORY dependencies/release/apple/ DESTINATION bin COMPONENT ${CORECOMPONET})
elseif(UNIX)
	install(DIRECTORY dependencies/release/linux/ DESTINATION bin 
		FILE_PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE 
		DIRECTORY_PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE 
		COMPONENT ${CORECOMPONET}
		PATTERN "resources/*" PERMISSIONS OWNER_READ GROUP_READ WORLD_READ 
		PATTERN "translations/*" PERMISSIONS OWNER_READ GROUP_READ WORLD_READ 
		PATTERN "qt.conf" PERMISSIONS OWNER_READ GROUP_READ WORLD_READ )
	install(FILES logo.ico DESTINATION bin PERMISSIONS OWNER_READ GROUP_READ WORLD_READ COMPONENT ${CORECOMPONET})
endif()

	

