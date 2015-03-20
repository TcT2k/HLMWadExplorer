cmake_minimum_required(VERSION 2.8)

project(HLMWADExplorer)

find_package(wxWidgets COMPONENTS core base adv REQUIRED)

include( ${wxWidgets_USE_FILE} )

if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
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
	TexturePackPanel.h
	TexturePackPanel.cpp
	)

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
endif()
include(InstallRequiredSystemLibraries)

SET(CPACK_PACKAGE_VERSION_MAJOR "0")
SET(CPACK_PACKAGE_VERSION_MINOR "6")
SET(CPACK_PACKAGE_VERSION_PATCH "1")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
SET(CPACK_GENERATOR "ZIP")
include(CPack)