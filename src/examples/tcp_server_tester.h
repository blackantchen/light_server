/*
 * File: tcp_server_tester.h
 * Created Date: September 21st 2020
 * Author: Jerry
 * -----
 * Last Modified:
 * Modified By:
 * -----
 * Copyright (c) 2020 (efancc@163.com)
 */
#ifndef __TCP_SERVER_TESTER_H__
#define __TCP_SERVER_TESTER_H__

#include "dm_packet.h"
#include "../socket_manager/socket_interface.h"
#include "../socket_manager/tcp_server.h"

using namespace std;

class TcpServerTester : private TcpServerListener {
   private:
    /* data */
    TcpServer* p_tcp_server_;
    DmPacket* p_dm_packet_;

   private:
    /*TcpServerListener Functions */
    virtual int OnConnect(SOCKET socket_id) override;
    virtual int OnDisconnect(SOCKET socket_id) override;
    virtual int DoRead(SOCKET socket_id, void* buf, size_t nbytes) override;
    // virtual int DoRead(SOCKET socket_id, void* buf, size_t nbytes, sockaddr *from_addr) override;

   private:
   public:
    TcpServerTester();
    ~TcpServerTester();

    bool Start();
    bool Stop();

    int SendDmMsg(SOCKET socket_id, void* buf, size_t nbytes);
};

#endif  // __TCP_SERVER_TESTER_H__
