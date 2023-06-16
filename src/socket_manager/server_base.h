/*
 * File: server_base.h
 * Created Date: September 18th 2020
 * Author: Jerry
 * -----
 * Last Modified:
 * Modified By:
 * -----
 * Copyright (c) 2020 (efancc@163.com)
 */
#ifndef __SERVER_BASE_H__
#define __SERVER_BASE_H__

#include "socket_manager.h"
#include "socket_interface.h"

using namespace std;

class ServerBase
{
protected:
    /* data */
    SocketManager socket_manager_;

    // const uint32_t kSocketRxBufferSize = 2048;
    TByteBuffer* p_rx_buffer_;

    uint32_t heartbeat_interval_in_seconds_;
    uint32_t keep_alive_time_in_seconds_;
    uint64_t timer_event_count_;

   protected:
    inline TByteBuffer* RxBuffer() { return p_rx_buffer_; }
    void MakeRxBuffer(uint32_t nbytes) {
        p_rx_buffer_ = new TByteBuffer(nbytes);
    }
    void Dispose() {
        if (p_rx_buffer_ != nullptr) {
            delete p_rx_buffer_;
            p_rx_buffer_ = nullptr;
        }
    }

    virtual bool HandleReceive(SocketObject* pSocketObj, int flag) = 0;
    virtual bool HandleSend(SocketObject* pSocketObj, int flag) = 0;

   public:
    // bool Stop();

    // int Disconnect(SOCKET socket_id);

    /* Common Operations */
    // virtual int Send(SOCKET socket_id, void* buf, size_t nbytes) { return 0; };
    // virtual int Send(SOCKET socket_id, sockaddr to_addr, void* buf, size_t nbytes);

    bool SetHeartbeatInterval(uint32_t num_of_seconds) {
        if(num_of_seconds > 3600) return false;

        heartbeat_interval_in_seconds_ = num_of_seconds;
        return true;
    }
    bool SetKeepAliveTime(uint32_t num_of_seconds){
        if(num_of_seconds > 3600) return false;

        keep_alive_time_in_seconds_ = num_of_seconds;
        return true;
    }
public:
 ServerBase()
     : p_rx_buffer_(nullptr),
       heartbeat_interval_in_seconds_(0),
       keep_alive_time_in_seconds_(0),
       timer_event_count_(0) {}
 virtual ~ServerBase() {}
};


#endif // __SERVER_BASE_H__
