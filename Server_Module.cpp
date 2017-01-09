//
// Created by zhaobq on 2016/12/16.
//
#include <boost/python.hpp>
#include "MPI_Server.cpp"
#include "MPI_Connect_Base.cpp"
#include "IRecv_handler_Wrapper.cpp"
using namespace boost::python;


BOOST_PYTHON_MODULE(Server_Module)
{
    class_<Pack_Int>("Pack_Int", init<int>())
        .def_readonly("buf", &Pack_Int::buf)
        ;

    class_<Pack_Str>("Pack_Str", init<char*>())
        .def_readonly("buf", &Pack_Str::buf)
        ;

    class_<IRecv_handler_Wrapper, boost::noncopyable>("IRecv_handler")
        //.def("handler_recv", pure_virtual(&IRecv_handler::handler_recv))
        .def("handler_recv_int", pure_virtual(handler_recv_int))
        .def("handler_recv_str", pure_virtual(handler_recv_str))
        ;
    class_<MPI_Server>("MPI_Server", init<IRecv_handler*, char*>())
        .def("run", &MPI_Server::run)
        .def("initialize", &MPI_Server::initialize)
        .def("stop", &MPI_Server::stop)
        .def("finalize", &MPI_Server::finalize)
        .def("recv_handle", &MPI_Server::recv_handle)
        .def("send_int", &MPI_Server::send_int)
        .def("send_string", &MPI_Server::send_string)
        .def("disconnect_client", &MPI_Server::disconnect_client)
        .def("bcast", &MPI_Server::bcast)
        ;

}
