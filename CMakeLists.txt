cmake_minimum_required(VERSION 3.20...4.0) # FetchContent is available in 3.11+

#set(CMAKE_C_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED True)

# https://www.programmersought.com/article/68945253024/

project(AISToCoT VERSION 0.0.1)

# This section deal with the character sets UNICODE _UNICODE
# Directx win32 settings
# this deal with convert string
add_definitions(-DUNICODE -D_UNICODE)



set(
    source_list
	
	mongoose/mongoose.h
	mongoose/mongoose.c
	
	hasmain/wmain.cpp

	mongooseservice/MongooseServiceLoop.cpp
	mongooseservice/WebUI.cpp
	mongooseservice/SampleService.cpp
	mongooseservice/SampleService.h

	windowsservice/ServiceBase.h
	windowsservice/ServiceBase.cpp
	windowsservice/ServiceInstaller.h
	windowsservice/ServiceInstaller.cpp
	windowsservice/ThreadPool.h

)

#this organizes the VS project folder like the source tree folder
foreach(source IN LISTS source_list)
    get_filename_component(source_path "${source}" PATH)
    string(REPLACE "/" "\\" source_path_msvc "${source_path}")
    source_group("${source_path_msvc}" FILES "${source}")
endforeach()


#================================================
# EXECUTE APP
#================================================
add_executable(${PROJECT_NAME} 
  ${CPP_HEADER_FILES}
  ${CPP_SOURCE_FILES}
  ${source_list}
)

set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/RunFolder")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME} )
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)


#target_compile_definitions(${PROJECT_NAME} PRIVATE "UNICODE" "_UNICODE")
# https://stackoverflow.com/questions/33137590/link-against-dx10-using-cmake

target_include_directories(${PROJECT_NAME}  PUBLIC mongoose)
target_include_directories(${PROJECT_NAME}  PUBLIC windowsservice)
target_include_directories(${PROJECT_NAME}  PUBLIC hasmain)
target_include_directories(${PROJECT_NAME}  PUBLIC mongooseservice)
