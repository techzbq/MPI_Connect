cmake_minimum_required(VERSION 3.6)
project(MPI_Connect)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

link_directories(../mpi-lib/lib)
include_directories(../mpi-lib/include)

set(SOURCE_FILES main.cpp MPI_Connect_Base.cpp MPI_Connect_Base.h IRecv_handler.h MPI_Util.h MPI_Server.cpp MPI_Server.h MPI_Client.cpp MPI_Client.h Server_Module.cpp Client_Module.cpp IRecv_handler_Wrapper.cpp IRecv_Module.cpp IRecv_buffer.h)
add_executable(MPI_Connect ${SOURCE_FILES})