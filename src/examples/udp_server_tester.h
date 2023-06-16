/*
 * File: udp_server_tester.h
 * Created Date: September 18th 2020
 * Author: Jerry
 * -----
 * Last Modified:
 * Modified By:
 * -----
 * Copyright (c) 2020 (efancc@163.com)
 */
#ifndef __UDP_SERVER_TESTER_H__
#define __UDP_SERVER_TESTER_H__

#include "../socket_manager/socket_interface.h"
#include "../socket_manager/udp_server.h"
// #include "dm_packet.h"

using namespace std;

class UdpServerTester : private UdpServerListener
{
private:
    /* data */
    UdpServer* p_udp_server_;

    SOCKET listen_sock_a_;
    SOCKET listen_sock_b_;

    // DmPacket* p_dm_packet_;

private:
    // virtual int OnConnect(SOCKET socket_id) override;
    // virtual int OnDisconnect(SOCKET socket_id) override;
    // virtual int DoRead(SOCKET socket_id, void* buf, size_t nbytes) override;
    virtual int DoRead(SOCKET socket_id, void* buf, size_t nbytes, sockaddr *from_addr) override;

public:
    UdpServerTester();
    ~UdpServerTester();

    bool Start();
    bool Stop();

    // int AddListenSocket(in_addr_t ipv4, in_port_t port);
    // int Send(SOCKET socket_id, sockaddr to_addr, void* buf, size_t nbytes);

    int SendToA(sockaddr* to_addr, void* buf, size_t nbytes);
    int SendToB(sockaddr* to_addr, void* buf, size_t nbytes);

    void Demo();
};

#endif // __UDP_SERVER_TESTER_H__
