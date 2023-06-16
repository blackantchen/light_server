/*
 * File: tcp_server.h
 * Created Date: September 21st 2020
 * Author: Jerry
 * -----
 * Last Modified:
 * Modified By:
 * -----
 * Copyright (c) 2020 (efancc@163.com)
 */
#ifndef __TCP_SERVER_H__
#define __TCP_SERVER_H__

#include "server_base.h"

#define MAX_PENDING_CONNECTTIONS_OF_TCP_SERVER  100
class TcpServer : public ServerBase, private CIOHandler {
   private:
    /* data */
    TcpServerListener* p_listener_;
    SOCKET listen_socket_;
    int32_t max_connections_of_listen_socket_;
    const uint32_t kSocketRxBufferSize = 2048;

   private:
    /* CIOHandler functions */
    // virtual void OnCommand(TSockMCommand* pCmd) override;
    virtual void OnTimer(uint64_t llExpirations) override;

    virtual bool OnBeforeProcessIo(PVOID pv, uint32_t events) override;
    virtual void OnAfterProcessIo(PVOID pv, uint32_t events, bool rs) override;
    virtual bool OnReadyRead(PVOID pv, uint32_t events) override;
    virtual bool OnReadyWrite(PVOID pv, uint32_t events) override;
    // virtual bool OnHungUp(PVOID pv, uint32_t events)
    // override; virtual bool OnError(PVOID pv, uint32_t events) override;
    // virtual bool OnReadyPrivilege(PVOID pv, uint32_t events)
    // override;

    virtual void OnConnect(SOCKET socket_id) override;
    virtual void OnDisconnect(SOCKET socket_id) override;

private:
    bool CreateListenSocket(in_addr_t ipv4, in_port_t port);
    void CloseListenSocket();
    bool StartAccept();
    void StopAccept();
    bool HandleAccept(uint32_t events);

   protected:
    bool HandleReceive(SocketObject* pSocketObj, int flag) override;
    bool HandleSend(SocketObject* pSocketObj, int flag) override;

   public:
    bool Start(TcpServerListener* p_listener, in_addr_t ipv4, in_port_t port);
    bool Stop();
    
    int Disconnect(SOCKET socket_id);
    int Send(SOCKET socket_id, void* buf, size_t nbytes);

   public:
    TcpServer(/* args */);
    virtual ~TcpServer();
};

#endif // __TCP_SERVER_H__
