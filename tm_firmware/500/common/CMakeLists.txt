add_library(500rebootpatches INTERFACE)
add_library(tm500_common::rebootpatches ALIAS 500rebootpatches)

target_include_directories(500rebootpatches INTERFACE include)
target_sources(500rebootpatches INTERFACE rebootPatches.cpp)