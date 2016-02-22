cmake_minimum_required(VERSION 2.8)

project(HLMWADExplorer)

SET(CPACK_PACKAGE_VERSION_MAJOR "0")
SET(CPACK_PACKAGE_VERSION_MINOR "9")
SET(CPACK_PACKAGE_VERSION_PATCH "0")

find_package(wxWidgets 3.0 COMPONENTS core base adv REQUIRED)

include( ${wxWidgets_USE_FILE} )

if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
	option(MSVC_USE_STATIC_RUNTIME ON)
endif()

if (MSVC_USE_STATIC_RUNTIME)
	foreach(flag_var
			CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
			CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
	   if(${flag_var} MATCHES "/MD")
		  string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
	   endif(${flag_var} MATCHES "/MD")
	endforeach(flag_var)
endif()

set(SRCS ${SRCS}
	App.cpp
	ExploreFrame.cpp
	ExploreFrame.h
	HLMWADExplorer.fbp
	HLMWADFrames.cpp
	HLMWADFrames.h
	WADArchive.cpp
	WADArchive.h
	TexturePack.h
	TexturePack.cpp
	MergeDialog.h
	MergeDialog.cpp
	TexturePackPanel.h
	TexturePackPanel.cpp
	StringTablePanel.h
	StringTablePanel.cpp
	)
	
if (APPLE)
	add_custom_command(
		OUTPUT ${${PROJECT_NAME}_BINARY_DIR}/appicon.icns
		COMMAND iconutil -c icns -o ${${PROJECT_NAME}_BINARY_DIR}/appicon.icns ${CMAKE_SOURCE_DIR}/appicon.iconset
		DEPENDS appicon.iconset
		COMMENT "Building application icon..."
		)

	set(SRCS ${SRCS}
		${${PROJECT_NAME}_BINARY_DIR}/appicon.icns
		)
	set(MACOSX_BUNDLE_ICON_FILE appicon.icns)
	set(MACOSX_BUNDLE_BUNDLE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
	SET_SOURCE_FILES_PROPERTIES(
		appicon.icns
		PROPERTIES
		MACOSX_PACKAGE_LOCATION Resources)
elseif(WIN32)
configure_file(VersionInfo.rc.in
	${${PROJECT_NAME}_BINARY_DIR}/VersionInfo.rc)
	set(SRCS ${SRCS}
		appicon.rc
		${${PROJECT_NAME}_BINARY_DIR}/VersionInfo.rc
	)
endif()

# Prepare version information
configure_file(VersionInfo.h.in
	${${PROJECT_NAME}_BINARY_DIR}/VersionInfo.h)

include_directories(${${PROJECT_NAME}_BINARY_DIR})

add_executable(HLMWADExplorer WIN32 MACOSX_BUNDLE ${SRCS})
target_link_libraries(HLMWADExplorer ${wxWidgets_LIBRARIES})

if (NOT MSVC)
	set(CMAKE_CXX_FLAGS                "-Wall -std=c++11")
endif()

set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION ".")
if (APPLE)
	install(TARGETS HLMWADExplorer BUNDLE DESTINATION ${CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION})
	set(CPACK_SYSTEM_NAME Mac)
	set_target_properties(${PROJECT_NAME} PROPERTIES
		MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/HLMWADExplorerInfo.plist.in
	)
else()
	install(TARGETS HLMWADExplorer RUNTIME DESTINATION ${CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION})
	install(FILES README.md LICENSE DESTINATION ${CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION})
endif()
if (NOT MSVC_USE_STATIC_RUNTIME)
	include(InstallRequiredSystemLibraries)
endif()

SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
SET(CPACK_GENERATOR "ZIP")
include(CPack)