cmake_minimum_required(VERSION 3.0)

project(HLMWADExplorer)

find_package(wxWidgets COMPONENTS core base adv REQUIRED)

include( ${wxWidgets_USE_FILE} )

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

set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION ".")
install(TARGETS HLMWADExplorer RUNTIME DESTINATION ${CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION})
include(InstallRequiredSystemLibraries)

SET(CPACK_PACKAGE_VERSION_MAJOR "0")
SET(CPACK_PACKAGE_VERSION_MINOR "5")
SET(CPACK_PACKAGE_VERSION_PATCH "0")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
SET(CPACK_GENERATOR "ZIP")
include(CPack)