cmake_minimum_required(VERSION 3.10)

file(STRINGS .env ConfigContent)
foreach(NameAndValue ${ConfigContent})
  string(REGEX REPLACE "^[ ]+" "" NameAndValue ${NameAndValue})
  string(REGEX MATCH "^[^=]+" Name ${NameAndValue})
  string(REPLACE "${Name}=" "" Value ${NameAndValue})
  set(${Name} ${Value})
endforeach()

configure_file(
  "main/app_config.h.in"
  "main/app_config.h"
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(controller-fware)