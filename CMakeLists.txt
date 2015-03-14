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
