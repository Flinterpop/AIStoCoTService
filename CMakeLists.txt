cmake_minimum_required(VERSION 3.20...4.0) # FetchContent is available in 3.11+

#set(CMAKE_C_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED True)

# https://www.programmersought.com/article/68945253024/

project(AISToCoT VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 17)


# This section deal with the character sets UNICODE _UNICODE
# Directx win32 settings
# this deal with convert string
add_definitions(-DUNICODE -D_UNICODE)


add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP>)

option (BUILD_SHARED_LIBRARY OFF)

# statically link MSVC runtime in thrift to being fully self-contained.
# this increases .exe size but guarantees working on all windows versions
# see https://cmake.org/cmake/help/latest/variable/CMAKE_MSVC_RUNTIME_LIBRARY.html
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")




# Add Mongoose source and include paths (adjust paths)
set(MONGOOSE_SRC ${CMAKE_CURRENT_SOURCE_DIR}/mongoose/mongoose.c)
set(MONGOOSE_INC ${CMAKE_CURRENT_SOURCE_DIR}/mongoose)
# Create a static library for Mongoose
add_library(mongoose_lib STATIC ${MONGOOSE_SRC})
target_include_directories(mongoose_lib PUBLIC ${MONGOOSE_INC})



set(
    AISLIB_list 
    	
	aislib/ais.cpp
	aislib/ais_bitset.cpp
	aislib/ais1_2_3.cpp
	aislib/ais4_11.cpp
	aislib/ais5.cpp
	aislib/ais6.cpp
	aislib/ais7_13.cpp
	aislib/ais8.cpp
	aislib/ais8_1_22.cpp
	aislib/ais8_1_26.cpp
	aislib/ais8_200.cpp
	aislib/ais8_366.cpp
	aislib/ais8_366_22.cpp
	aislib/ais8_367.cpp
	aislib/ais9.cpp
	aislib/ais10.cpp
	aislib/ais12.cpp
	aislib/ais14.cpp
	aislib/ais15.cpp
	aislib/ais16.cpp
	aislib/ais17.cpp
	aislib/ais18.cpp
	aislib/ais19.cpp
	aislib/ais20.cpp
	aislib/ais21.cpp
	aislib/ais22.cpp
	aislib/ais23.cpp
	aislib/ais24.cpp
	aislib/ais25.cpp
	aislib/ais26.cpp
	aislib/ais27.cpp
	aislib/decode_body.cpp
	aislib/vdm.cpp
)
# Add COT source and include paths (adjust paths)
#set(COT_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src_COT/${COT_list})
set(AISLIB_INC ${CMAKE_CURRENT_SOURCE_DIR}/aislib)
# Create a static library for COT
add_library(AISLIB STATIC ${AISLIB_list})
target_include_directories(AISLIB PUBLIC ${AISLIB_INC})



set(
    SerialLib_list 
	
	serialLib/CSerialPort/ibuffer.hpp                                                          
	serialLib/CSerialPort/ilog.hpp                                                             
	serialLib/CSerialPort/IProtocolParser.h                                                    
	serialLib/CSerialPort/ithread.hpp                                                          
	serialLib/CSerialPort/itimer.hpp                                                           
	serialLib/CSerialPort/iutils.hpp                                                           
	serialLib/SerialPort.cpp                                                       
	serialLib/CSerialPort/SerialPort.h                                                         
	serialLib/SerialPortBase.cpp                                                   
	serialLib/CSerialPort/SerialPortBase.h              
	serialLib/CSerialPort/SerialPortHotPlug.hpp     
	serialLib/CSerialPort/SerialPortHotPlug_win.hpp                                            
	serialLib/SerialPortInfo.cpp                                                   
	serialLib/CSerialPort/SerialPortInfo.h                                                     
	serialLib/SerialPortInfoBase.cpp                                               
	serialLib/CSerialPort/SerialPortInfoBase.h                                                 
	serialLib/SerialPortInfoWinBase.cpp                                            
	serialLib/CSerialPort/SerialPortInfoWinBase.h                                              
	serialLib/CSerialPort/SerialPortListener.h                                                 
	serialLib/SerialPortWinBase.cpp                                                
	serialLib/CSerialPort/SerialPortWinBase.h                                                  
	serialLib/CSerialPort/SerialPort_global.h                                                  
	serialLib/CSerialPort/SerialPort_version.h  
)

set(SERIALLIB_INC ${CMAKE_CURRENT_SOURCE_DIR}/serialLib)
# Create a static library for COT
add_library(SERIALLIB STATIC ${SerialLib_list})
target_include_directories(SERIALLIB PUBLIC ${SERIALLIB_INC})









set(
    source_list
	
	#mongoose/mongoose.h
	#mongoose/mongoose.c
	
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


    src_app/hasMain.cpp
	src_app/AIStoCOT.h
	src_app/MySerialPortListener.h
	src_app/MySerialPortListener.cpp
	
	src_base/csv.hpp
    src_base/BG_SocketBase.h
    src_base/BG_SocketBase.cpp
	
	src_COT/bg_TakMessage.cpp
    src_COT/COTSender.cpp

	src_AISPARSE/AISParser.cpp
	src_AISPARSE/AISParser.h 
	src_AISPARSE/AISVessel.h
	src_AISPARSE/AISObject.h
	src_AISPARSE/MapEntity.h
	src_AISPARSE/csv.hpp
	

)


set_property(GLOBAL PROPERTY USE_FOLDERS ON) 


#this organizes the VS project folder like the source tree folder
foreach(source IN LISTS source_list)
    get_filename_component(source_path "${source}" PATH)
    string(REPLACE "/" "\\" source_path_msvc "${source_path}")
    source_group("${source_path_msvc}" FILES "${source}")
endforeach()



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



target_include_directories(${PROJECT_NAME}  PUBLIC src_base)
target_include_directories(${PROJECT_NAME}  PUBLIC src_COT)
target_include_directories(${PROJECT_NAME}  PUBLIC src_AISPARSE)



target_link_libraries(${PROJECT_NAME} PRIVATE mongoose_lib)
target_link_libraries(${PROJECT_NAME} PRIVATE AISLIB)
target_link_libraries(${PROJECT_NAME} PRIVATE SERIALLIB)