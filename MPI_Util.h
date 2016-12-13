//
// Created by zhaobq on 2016/12/12.
//

#ifndef MPI_CONNECT_MPI_UTIL_H
#define MPI_CONNECT_MPI_UTIL_H

//传输用的tags， 每个tags中包括了传输的数据类型
class MPI_Tags{
public:
    const static int MPI_RECV_INT         = 0;
    const static int MPI_RECV_CHAR        = 1;
    const static int MPI_SEND_INT         = 2;
    const static int MPI_SEND_CHAR        = 3;
    const static int MPI_BCAST_INT        = 4;
    const static int MPI_BCAST_CHAR       = 5;

    const static int MPI_HEART_BEAT       = 10;
    const static int MPI_REGISTEY         = 11;
    const static int MPI_DISCONNECT       = 12;

    const static int MPI_BCAST_REQ        = 20; //用于广播询问task剩余情况
    const static int MPI_BCAST_ACK       = 21; //用于回复广播询问task
};

class MPI_ERR_CODE{
public:
    const static int SUCCESS                = 0;
    const static int OPEN_PORT_ERR          = 1;
    const static int PUBLISH_SVC_ERR        = 2;
    const static int UNPUBLISH_ERR          = 3;
    const static int DISCONN_ERR            = 4;
    const static int STOP_ERR               = 5;
    const static int LOOKUP_SVC_ERR         = 6;
    const static int CONNECT_ERR            = 7;
};


#endif //MPI_CONNECT_MPI_UTIL_H
