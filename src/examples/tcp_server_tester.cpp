/*
 * File: tcp_server_tester.cpp
 * Created Date: September 21st 2020
 * Author: Jerry
 * -----
 * Last Modified:
 * Modified By:
 * -----
 * Copyright (c) 2020 (efancc@163.com)
 */
#include <unistd.h>
#include <iostream>

#include "../include/common_include.h"
#include "../utilities/common_funcs.h"

#include "net_interface_cfg.h"
#include "tcp_server_tester.h"

#ifdef _DEBUG_EN
#include "../memory_leak_detector/debug_new.h"
#endif

TcpServerTester::TcpServerTester() {
    p_tcp_server_ = new TcpServer();
    p_dm_packet_ = new DmPacket();
}

TcpServerTester::~TcpServerTester() {
    if (p_tcp_server_ != nullptr) {
        delete p_tcp_server_;
        p_tcp_server_ = nullptr;
    }
    
    if (p_dm_packet_ != nullptr) {
        delete p_dm_packet_;
        p_dm_packet_ = nullptr;
    }
}

bool TcpServerTester::Start(){
    p_tcp_server_->SetHeartbeatInterval(20);
    p_tcp_server_->SetKeepAliveTime(60);

    if (p_tcp_server_->Start(this, INADDR_ANY, TCP_SERVER_TESTER_LISTEN_PORT) == false) {
        return false;
    }
    return true;
}
bool TcpServerTester::Stop(){
    return p_tcp_server_->Stop();
}
int TcpServerTester::OnConnect(SOCKET socket_id){
    cout << "TcpServerTester::OnConnect" << endl;
    // AddInPacket(socket_id);
    return 0;
}
int TcpServerTester::OnDisconnect(SOCKET socket_id){
    cout << "TcpServerTester::OnDisconnect" << endl;
    // EarseInPacket(socket_id);
    // pApp()->ClientManager()->ClientList()->RemoveBySocketID(socket_id);
    return 0;
}
int TcpServerTester::DoRead(SOCKET socket_id, void* buf, size_t nbytes){
    // return PacketRead(socket_id, buf, nbytes);
    cout << "TcpServerTester::DoRead, len: " << nbytes << ", " << (char*)buf << endl;

    return 0;
}

int TcpServerTester::SendDmMsg(SOCKET socket_id, void* buf, size_t nbytes) {
    // uint32_t pkgLen = nbytes + 16;
    // uint32_t pkgCRC = 0;
    // TMemoryWriter* wt = new TMemoryWriter(pkgLen);

    // // cout << "Dm8kDesigner::SendTcpMsg(socket_id):" << socket_id << endl;

    // int offset = 0;
    // wt->WriteBytes(offset, (BYTE*)DmPacketHeader(), 4);

    // // 1.Write package length
    // wt->WriteInt32(offset + 4, pkgLen);

    // // 2. copy cmd data
    // wt->WriteBytes(offset + 8, (BYTE*)buf, nbytes);

    // // 4.Write Cyc
    // pkgCRC = CommonFuncs::CalculateStm32Crc((uint8_t*)wt->GetBuffer(), wt->BufferSize());

    // wt->WriteInt32(offset + 8 + nbytes, pkgCRC);

    // // 5. Write Tail
    // int wr_len = wt->WriteBytes(offset + 12 + nbytes, (BYTE*)DmPacketTail(), 4);

    // // TcpClientsManager()->Send(socket_id, (void*)wt->GetBuffer(), wt->BufferSize());
    // p_tcp_server_->Send(socket_id, (void*)wt->GetBuffer(), wt->BufferSize());

    // if (wt != nullptr) {
    //     delete wt;
    //     wt = nullptr;
    // }

    // return wr_len;
    return p_tcp_server_->Send(socket_id, buf, nbytes);
}