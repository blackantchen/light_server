/*
 * File: tcp_server_tester2.cpp
 * Created Date: June 16th 2022
 * Author: Jerry
 * -----
 * Last Modified:
 * Modified By:
 * -----
 * Copyright (c) 2022 (efancc@163.com)
 */
#include <unistd.h>

#include <iostream>

#include "../include/common_include.h"
#include "../utilities/common_funcs.h"
#include "net_interface_cfg.h"

#include "tcp_server_tester2.h"

#ifdef _DEBUG_EN
#include "../memory_leak_detector/debug_new.h"
#endif

TcpServerTester2::TcpServerTester2() : TcpServerBase() {
}

TcpServerTester2::~TcpServerTester2() {
}

bool TcpServerTester2::Start() {
    pServer()->SetHeartbeatInterval(20);
    pServer()->SetKeepAliveTime(60);

    if (pServer()->Start(this, INADDR_ANY, TCP_SERVER_TESTER_LISTEN_PORT) == false) {
        return false;
    }
    return true;
}

int TcpServerTester2::OnConnect(SOCKET socket_id) {
    cout << "TcpServerTester2::OnConnect" << endl;
    // AddInPacket(socket_id);
    return 0;
}
int TcpServerTester2::OnDisconnect(SOCKET socket_id) {
    cout << "TcpServerTester2::OnDisconnect" << endl;
    return 0;
}
int TcpServerTester2::DoRead(SOCKET socket_id, void* buf, size_t nbytes) {
    cout << "TcpServerTester2::DoRead, len: " << nbytes << ", " << (char*)buf << endl;

    this->SendMsg(socket_id, buf, nbytes);
    return 0;
}

void TcpServerTester2::Demo() {
    //
    // 1. Any time when a client connect to this sever, send a welcome message ("hi, guys, welcome to earth") to it
    // 2. if received a message from client, pass back the message
    //
    return;
}