/*
 * File: udp_server.h
 * Created Date: September 18th 2020
 * Author: Jerry
 * -----
 * Last Modified:
 * Modified By:
 * -----
 * Copyright (c) 2020 (efancc@163.com)
 */
#ifndef __UDP_SERVER_H__
#define __UDP_SERVER_H__

#include "server_base.h"

class UdpServer : public ServerBase, private CIOHandler {
   private:
    /* data */
    UdpServerListener* p_listener_;

    const uint32_t kSocketRxBufferSize = 2048;

   private:
    /* CIOHandler functions */
    // virtual void OnCommand(TSockMCommand* pCmd) override;
    virtual void OnTimer(uint64_t llExpirations) override;

    // virtual bool OnBeforeProcessIo(PVOID pv, uint32_t events) override;
    // virtual void OnAfterProcessIo(PVOID pv, uint32_t events, bool rs)
    // override;
    virtual bool OnReadyRead(PVOID pv, uint32_t events) override;
    virtual bool OnReadyWrite(PVOID pv, uint32_t events) override;
    // virtual bool OnHungUp(PVOID pv, uint32_t events)
    // override; virtual bool OnError(PVOID pv, uint32_t events) override;
    // virtual bool OnReadyPrivilege(PVOID pv, uint32_t events)
    // override;

    virtual void OnConnect(SOCKET socket_id) override;
    virtual void OnDisconnect(SOCKET socket_id) override;

   protected:
    bool HandleReceive(SocketObject* pSocketObj, int flag) override;
    bool HandleSend(SocketObject* pSocketObj, int flag) override;

   public:
    bool Start(UdpServerListener* p_listener);
    bool Stop();
    
    int AddListenSocket(in_addr_t ipv4, in_port_t port);
    int RemoveSocket(SOCKET socket_id);
    void SetReceivePacketsLimit(SOCKET socket_id, uint32_t num_of_packets);
    // virtual int Send(SOCKET socket_id, sockaddr to_addr, void* buf, size_t
    // nbytes) override;
    int Send(SOCKET socket_id, sockaddr* to_addr, void* buf, size_t nbytes);

   public:
    UdpServer(/* args */);
    virtual ~UdpServer();
};

#endif  // __UDP_SERVER_H__
