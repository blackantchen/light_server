/*
 * File: tcp_client.h
 * Created Date: September 16th 2020
 * Author: Jerry
 * -----
 * Last Modified:
 * Modified By:
 * -----
 * Copyright (c) 2020 (efancc@163.com)
 */
#ifndef __TCP_CLIENT_H__
#define __TCP_CLIENT_H__

#include "socket_manager.h"
#include "socket_interface.h"

using namespace std;

#define TCP_CLIENT_HEARTBEAT_INTERVAL (20 * 1000)
#define TCP_CLIENT_KEEP_ALIVE_TIME   (60) // 60 seconds
class TcpClient : private CIOHandler {
   private:
    /* data */
    SocketManager socket_manager_;
    TcpClientListener* p_listener_;

    const uint32_t kSocketRxBufferSize = 1024;
    TByteBuffer* p_rx_buffer_;

    uint32_t heartbeat_interval_in_seconds_;
    uint32_t keep_alive_time_in_seconds_;
    uint64_t timer_event_count_;

   private:
    bool HandleReceive(SocketObject* pSocketObj, int flag);
    bool HandleSend(SocketObject* pSocketObj, int flag);

    inline TByteBuffer* RxBuffer() { return p_rx_buffer_; }
    void Dispose();

    bool DisconnectSilenceConnections(uint64_t period);
	
   private:
    // virtual void OnCommand(TSockMCommand* pCmd) override;
    virtual void OnTimer(uint64_t llExpirations) override;

    virtual bool OnBeforeProcessIo(PVOID pv, uint32_t events) override;
    // virtual void OnAfterProcessIo(PVOID pv, uint32_t events, bool rs) override;
    virtual bool OnReadyRead(PVOID pv, uint32_t events) override;
    virtual bool OnReadyWrite(PVOID pv, uint32_t events) override;
    // virtual bool OnHungUp(PVOID pv, uint32_t events)					override;
    // virtual bool OnError(PVOID pv, uint32_t events) override; 
    // virtual bool OnReadyPrivilege(PVOID pv, uint32_t events)			override; 

    virtual void OnConnect(SOCKET socket_id) override;
    virtual void OnDisconnect(SOCKET socket_id) override;

   public:
    TcpClient();
    virtual ~TcpClient() { Dispose(); }

    bool Start(TcpClientListener* p_listener);
    bool Stop();

    int ConnectToRemote(sockaddr* p_sockaddr);
    int AsynchConnectTo(sockaddr* p_sockaddr);
    int Disconnect(SOCKET socket_id);

    /* Common Operations */
    int Send(SOCKET socket_id, void* buf, size_t nbytes);
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
    void SetReceiveBytesLimit(SOCKET socket_id, uint32_t nbytes);
};

#endif // __TCP_CLIENT_H__
