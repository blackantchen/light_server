/*
 * File: udp_server_tester.cpp
 * Created Date: September 18th 2020
 * Author: Jerry
 * -----
 * Last Modified:
 * Modified By:
 * -----
 * Copyright (c) 2020 (efancc@163.com)
 */

#include "udp_server_tester.h"

#include <unistd.h>

#include <iostream>

#include "../include/common_include.h"
#include "../utilities/common_funcs.h"
#include "net_interface_cfg.h"

UdpServerTester::UdpServerTester() {
    p_udp_server_ = new UdpServer();

    // p_dm_packet_ = new DmPacket();
}

UdpServerTester::~UdpServerTester() {
    if (p_udp_server_ != nullptr) {
        delete p_udp_server_;
        p_udp_server_ = nullptr;
    }

    // if (p_dm_packet_ != nullptr) {
    //     delete p_dm_packet_;
    //     p_dm_packet_ = nullptr;
    // }
}

bool UdpServerTester::Start() {
    p_udp_server_->SetHeartbeatInterval(20);
    p_udp_server_->SetKeepAliveTime(60);

    if (p_udp_server_->Start(this) == false)
        return false;

    listen_sock_a_ = p_udp_server_->AddListenSocket(INADDR_ANY, UDP_SERVER_TEST_PORT_A);
    listen_sock_b_ = p_udp_server_->AddListenSocket(INADDR_ANY, UDP_SERVER_TEST_PORT_B);
    return true;
}
bool UdpServerTester::Stop() {
    return p_udp_server_->Stop();
}

int UdpServerTester::DoRead(SOCKET socket_id, void* buf, size_t nbytes, sockaddr* from_addr) {
    if (socket_id == listen_sock_a_) {
        cout << "Listen Port-A received a msg: " << nbytes << "bytes, " << (char*)buf << endl;
        SendToA(from_addr, buf, nbytes);
    } else if (socket_id == listen_sock_b_) {
        cout << "Listen Port-B received a msg: " << nbytes << "bytes, " << (char*)buf << endl;
        SendToB(from_addr, buf, nbytes);
    }
    return 0;
}

int UdpServerTester::SendToA(sockaddr* to_addr, void* buf, size_t nbytes) {
    return p_udp_server_->Send(listen_sock_a_, to_addr, buf, nbytes);
}
int UdpServerTester::SendToB(sockaddr* to_addr, void* buf, size_t nbytes) {
    return p_udp_server_->Send(listen_sock_b_, to_addr, buf, nbytes);
}

void UdpServerTester::Demo() {
    //
    // if received a message from client, pass back the message
    //

    return;
}