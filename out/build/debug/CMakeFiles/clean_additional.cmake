# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\appStoryFlow_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\appStoryFlow_autogen.dir\\ParseCache.txt"
  "appStoryFlow_autogen"
  )
endif()
