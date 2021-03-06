//
// Created by zhaobq on 2016/12/13.
//

#include "MPI_Client.h"
#include <cstring>
#include <iomanip>

#define DEBUG

MPI_Client::MPI_Client(IRecv_buffer* mh, char* svc_name, char* uuid):MPI_Connect_Base(mh){
    svc_name_ = svc_name;
    uuid_ = uuid;
    recv_flag_mutex = PTHREAD_MUTEX_INITIALIZER;
}

MPI_Client::~MPI_Client() {
    //TODO delete someting
    if(!recv_flag && !send_flag)
        stop();
}

int MPI_Client::initialize() {
    cout << "--------------------Client init start--------------------" << endl;
    cout << "[Client]: client initail..." << endl;
    int merr= 0;
    int msglen = 0;
    char errmsg[MPI_MAX_ERROR_STRING];

    int provide;
    MPI_Init_thread(0,0, MPI_THREAD_MULTIPLE, &provide);
    cout << "[Client_"<< myrank <<"]: support thread level= " << provide << endl;
    MPI_Comm_set_errhandler(MPI_COMM_WORLD, MPI_ERRORS_RETURN);

    //recv_thread(this);
    //pthread_create(&send_t, NULL, MPI_Connect_Base::send_thread, this);
    //while(send_flag);
    //cout << "[Client]: send thread start...." << endl;


    cout << "[Client_"<< myrank <<"]: finding service name <" << svc_name_ << "> ..." <<endl;
    merr = MPI_Lookup_name(svc_name_, MPI_INFO_NULL, portname);
    if(merr){
        MPI_Error_string(merr, errmsg, &msglen);
        cout << "[Client-"<<myrank<<"-error]: Lookup service name error, msg: "<< errmsg << endl;
        return MPI_ERR_CODE::LOOKUP_SVC_ERR;
        //TODO Add error handle
    }

    cout << "[Client_"<< myrank <<"]: service found on port:<" << portname << ">" << endl;

    //while(recv_flag || send_flag);
    merr = MPI_Comm_connect(portname, MPI_INFO_NULL,0, MPI_COMM_SELF, &sc_comm_);
    if(merr){
        MPI_Error_string(merr, errmsg, &msglen);
        cout << "[Client-"<<myrank<<"-error]: Connect to Server error, msg: " << errmsg << endl;
        return MPI_ERR_CODE::CONNECT_ERR;
        //TODO Add error handle
    }
    cout << "[Client_"<< myrank <<"]: client connect to server, comm = " << sc_comm_ << endl;
    MPI_Barrier(sc_comm_);

    pthread_create(&recv_t, NULL, MPI_Connect_Base::recv_thread, this);
    while(true){
        pthread_mutex_lock(&recv_flag_mutex);
        if(!recv_flag) {
            pthread_mutex_unlock(&recv_flag_mutex);
            break;
        }
        pthread_mutex_unlock(&recv_flag_mutex);
    }
    cout << "[Client_"<< myrank <<"]: recv thread start...." << endl;

    //int rank;
    //MPI_Comm_rank(sc_comm_,&rank);
    cout << "--------------------Client "<< myrank <<" finish--------------------" << endl;
    //send_action(&wid_, 1, MPI_INT, 0, MPI_REGISTEY, sc_comm_);

    return MPI_ERR_CODE::SUCCESS;
}

int MPI_Client::stop() {
    cout << "--------------------stop Client "<< myrank<<"--------------------" << endl;
    //cout << "[Client_"<< myrank <<"]: stop Client..." << endl;
    int merr= 0;
    int msglen = 0;
    char errmsg[MPI_MAX_ERROR_STRING];
    //recv_flag = true;
    set_recv_stop();
    //send_flag = true;
    //TODO add disconnect send
    char* tmp = (char *) uuid_.data();
    //send(&tmp, 1, 0, MPI_INT, MPI_Tags::MPI_DISCONNECT, sc_comm_);
    if(send_string(tmp, strlen(tmp), 0, MPI_DISCONNECT) == MPI_ERR_CODE::SUCCESS)
        cout <<"[Client_"<< myrank <<"]: send complete..." << endl;
    //pthread_cancel(send_t);
    //MPI_Barrier(sc_comm_);
    merr = MPI_Comm_disconnect(&sc_comm_);
    if(merr){
        MPI_Error_string(merr, errmsg, &msglen);
        cout << "[Client-"<<myrank<<"-Error]: disconnect error :" << errmsg << endl;
        return MPI_ERR_CODE::DISCONN_ERR;
    }
    cout << "[Client_"<< myrank <<"]: disconnected..." << endl;
    finalize();
    cout << "--------------------Client "<< myrank <<" stop finish--------------------" << endl;
    return MPI_ERR_CODE::SUCCESS;
}

int MPI_Client::finalize() {
    int ret;
//    ret = pthread_join(send_t, NULL);
//    cout <<"[Client]: send thread stop, exit code=" << ret << endl;
    ret = pthread_join(recv_t, NULL);
    cout <<"[Client_"<< myrank <<"]: recv thread stop, exit code=" << ret << endl;
//    pthread_mutex_destroy(&send_mtx);
//    pthread_mutex_destroy(&sendmsg_mtx);
//    pthread_cond_destroy(&send_thread_cond);
    MPI_Finalize();
    return 0;
}

bool MPI_Client::new_msg_come(ARGS *args) {
    if(sc_comm_ == 0x0)
        return false;
    int merr = 0;
    int msglen = 0;
    char errmsg[MPI_MAX_ERROR_STRING];

    MPI_Status *stat = new MPI_Status;
    int flag = 0;
    merr = MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, sc_comm_, &flag, stat);
    if (merr) {
        MPI_Error_string(merr, errmsg, &msglen);
        cout << "[Client-Error]: Iprobe message error :" << errmsg << endl;
        delete(stat);
        return false;
    }
    if (flag) {
#ifdef DEBUG
        cout << "[Client_"<< myrank <<"]: dectect a new msg <source=" << stat->MPI_SOURCE << ";tag=" << stat->MPI_TAG << ">" <<endl;
#endif
        //args = new ARGS();
        args->arg_stat = *stat;
        args->datatype = MPI_CHAR;
        args->source_rank = stat->MPI_SOURCE;
        args->newcomm = sc_comm_;
        flag = 0;
        delete (stat);
        return true;
    } else {
        delete(stat);
        return false;
    }
}

//void MPI_Client::send(void *buf, int msgsize, int dest, MPI_Datatype datatype, int tag, MPI_Comm comm) {
//    cout << "[Client]: send message...<" << (*(int*)buf)<< ","<< msgsize << "," << dest << ">"<< endl;
//    MPI_Connect_Base::send(buf, msgsize, dest, datatype, tag, comm);
//    cout << "[Client]: send finish, send thread sleep..." << endl;
//}

int MPI_Client::send_int(int buf, int msgsize, int dest, int tag) {
#ifdef DEBUG
    cout << "[Client_"<< myrank <<"]: send message...<" << buf << ",msgsize="<< msgsize <<",dest="<<dest <<",tag=" <<tag  << ">"<< endl;
#endif
    int merr = 0;
    int msglen = msgsize;
    char errmsg[MPI_MAX_ERROR_STRING];

    merr = MPI_Send(&buf, msgsize, MPI_INT, 0,  tag, sc_comm_);
    if(merr){
        MPI_Error_string(merr, errmsg, &msglen);
        cout << "[Client-Error]: send fail...error: " << errmsg << endl;
        return MPI_ERR_CODE::SEND_FAIL;
    }

    cout << "[Client_"<< myrank <<"]: start barrier..." << endl;

    merr = MPI_Barrier(sc_comm_);
    if(merr){
        MPI_Error_string(merr, errmsg, &msglen);
        cout << "[Client-Error]: barrier fail...error: " << errmsg << endl;
        return MPI_ERR_CODE::BARRIER_FAIL;
    }

    cout << "[Client_"<< myrank <<"]: end barrier..." << endl;

    return MPI_ERR_CODE::SUCCESS;
}

int MPI_Client::send_string(char* buf, int msgsize, int dest, int tag){
#ifdef DEBUG
    cout << "[Client_"<< myrank <<"]: send message...<" << buf << ",msgsize="<< msgsize <<",dest="<<dest <<",tag=" <<tag  << ">"<< endl;
#endif
    int merr = 0;
    int msglen = msgsize;
    char errmsg[MPI_MAX_ERROR_STRING];
    merr = MPI_Send(buf, msgsize, MPI_CHAR, 0,  tag, sc_comm_);
    if(merr){
        MPI_Error_string(merr, errmsg, &msglen);
        cout << "[Client-Error]: send fail...error: " << errmsg << endl;
        return MPI_ERR_CODE::SEND_FAIL;
    }
#ifdef DEBUG
    cout << "[Client_"<< myrank <<"]: start barrier..." << endl;
#endif
    merr = MPI_Barrier(sc_comm_);
    if(merr){
        MPI_Error_string(merr, errmsg, &msglen);
        cout << "[Client-Error]: barrier fail...error: " << errmsg << endl;
        return MPI_ERR_CODE::BARRIER_FAIL;
    }
#ifdef DEBUG
    cout << "[Client_"<< myrank <<"]: end barrier..." << endl;
#endif
    return MPI_ERR_CODE::SUCCESS;
}

void MPI_Client::run() {
    // TODO main thread for client
    initialize();
}

void MPI_Client::recv_handle(int tag, void *buf, int length, MPI_Datatype type,MPI_Comm comm) {
    // TODO add conditions
    int merr, msglen;
    char errmsg[MPI_MAX_ERROR_STRING];
    switch (tag){
        case MPI_DISCONNECT:{
            //MPI_Barrier(comm);
            if(comm != sc_comm_) {
#ifdef DEBUG
                cout << "[Client-"<<myrank<<"-Error]: disconnect error: MPI_Comm is not matching" << endl;
#endif
                //TODO error handle
            }
            merr = MPI_Comm_disconnect(&sc_comm_);
            if(merr){
                MPI_Error_string(merr, errmsg, &msglen);
                cout << "[Client-"<<myrank<<"-Error]: disconnect error: " << errmsg << endl;
                //TODO Add error handle
            }
        }
            break;
        default:
            //Irecv_handler->handler_recv(tag, buf);
            cout << "[Client-Error]: Unrecognized type" << endl;
            break;
    }
/*    if(type == MPI_INT) {
        Pack_Int pack = Pack_Int((*(int *) buf));
        Irecv_handler->handler_recv_int(tag, pack);
    }
    else if(type == MPI_CHAR) {
        Pack_Str pack = Pack_Str((char *) buf);
        Irecv_handler->handler_recv_str(tag, pack);
    }
    else {
#ifdef DEBUG
        cout << "[Client-Error]: Recv datatype error" << endl;
#endif
        //TODO add error handler
    }
*/
}
