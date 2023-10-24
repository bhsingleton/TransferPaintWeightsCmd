cmake_minimum_required(VERSION 3.21)
project(TransferPaintWeightsCmd)
include($ENV{DEVKIT_LOCATION}/cmake/pluginEntry.cmake)

set(
	SOURCE_FILES
	"pluginMain.cpp"
	"TransferPaintWeightsCmd.h"
	"TransferPaintWeightsCmd.cpp"
)

set(
	LIBRARIES
	OpenMaya
	Foundation
)

build_plugin()