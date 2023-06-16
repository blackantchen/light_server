/*
 * File: udp_server.cpp
 * Created Date: September 18th 2020
 * Author: Jerry
 * -----
 * Last Modified:
 * Modified By:
 * -----
 * Copyright (c) 2020 (efancc@163.com)
 */

#include "udp_server.h"

#include <unistd.h>

#include <iostream>

UdpServer::UdpServer() 
    : ServerBase(), 
      p_listener_(nullptr) { 
    MakeRxBuffer(kSocketRxBufferSize); 
}
UdpServer::~UdpServer() {
    ServerBase::Dispose();
}

bool UdpServer::Start(UdpServerListener* p_listener){
    ASSERT(p_listener != nullptr);

    p_listener_ = p_listener;
    socket_manager_.Start(this, heartbeat_interval_in_seconds_*1000);

    // MakeRxBuffer(kSocketRxBufferSize);
    return TRUE;
}
bool UdpServer::Stop() {
    socket_manager_.CloseAllSocket();
    return (0 == socket_manager_.Stop());
}

void UdpServer::OnConnect(SOCKET socket_id) {
    cout << "OnConnect" << endl;
    p_listener_->OnConnect(socket_id);
}
void UdpServer::OnDisconnect(SOCKET socket_id) {
    cout << "OnDisconnect" << endl;
    p_listener_->OnDisconnect(socket_id);
}
void UdpServer::OnTimer(uint64_t llExpirations) {}
bool UdpServer::OnReadyRead(PVOID pv, uint32_t events) {
    // cout << "UdpServer::OnReadyRead" << endl;
    return HandleReceive((SocketObject*)pv, events);
}
bool UdpServer::OnReadyWrite(PVOID pv, uint32_t events) {
    // cout << "UdpServer::OnReadyWrite" << endl;
    return HandleSend((SocketObject*)pv, events);
}

bool UdpServer::HandleReceive(SocketObject* pSocketObj, int flag) {
    // ASSERT(SocketObject::IsValid(pSocketObj));
    if (SocketObject::IsInvalid(pSocketObj)) return FALSE;

    SOCKET sock_id = pSocketObj->SocketId();
    TByteBuffer* buffer = this->RxBuffer();
    sockaddr from_addr;
    socklen_t addr_len = sizeof(from_addr);

    pSocketObj->active_time_ = socket_manager_.GetTime();
    int reads = flag ? -1 : MAX_CONTINUE_READS;

    for (int i = 0; i < reads || reads < 0; i++) {
        // int rc = (int)read(sock_id, buffer->Ptr(), buffer->Size());
        int rc = recvfrom(sock_id, buffer->Ptr(), buffer->Size(), 0, &from_addr, &addr_len);

        if (rc > 0) {
            if (pSocketObj->IsReceiveLimit()) {
                uint64_t time_ms = socket_manager_.GetTimeInMillisecond();
                pSocketObj->CountReceivedPacket(1, time_ms);
                if (pSocketObj->IsReceiveOverflow()) {
                    cout << ">>>>>> UDP receive OVER FLOW >>>>>>" << endl;
                    // CommonFuncs::PrintSockaddr((sockaddr*)&from_addr);
                    return FALSE;
                }
            }
            p_listener_->DoRead(sock_id, buffer->Ptr(), rc, &from_addr);
        } else if (rc == 0) {
            // AddFreeSocketObj(pSocketObj, SCF_CLOSE, SO_RECEIVE, SE_OK);
            cout << "HandleReceive: rc = 0" << endl;
            socket_manager_.CloseSocket(sock_id);
            return FALSE;
        } else {
            ASSERT(rc == SOCKET_ERROR);

            // int code = ::WSAGetLastError();

            if (errno == EWOULDBLOCK) break;

            // AddFreeSocketObj(pSocketObj, SCF_ERROR, SO_RECEIVE, code);
            cout << "rc = " << rc << endl;
            cout << "Socket read error: " << strerror(errno) << endl;
            socket_manager_.CloseSocket(sock_id);
            return FALSE;
        }
    }

    return TRUE;
}
bool UdpServer::HandleSend(SocketObject* pSocketObj, int flag) {
    // ASSERT(SocketObject::IsValid(pSocketObj));
    if (SocketObject::IsInvalid(pSocketObj)) return -1;

    while (!pSocketObj->OutPacketEmpty()) {
        MsgPacket* p_packet = pSocketObj->FrontOutPacket();
        if (p_packet != nullptr) {
            int wr_len = 0;
            while (p_packet->Size()) {
                wr_len = sendto(pSocketObj->SocketId(), p_packet->Ptr(),
                                p_packet->Size(), 0, p_packet->pSocketAddr(),
                                sizeof(sockaddr));
                if (wr_len > 0) {
                    p_packet->Reduce(wr_len);
                } else {
                    if (errno == EWOULDBLOCK || errno == EAGAIN)
                        return 0;
                    else{
                        cout << "UdpServer::HandleSend error, sock = " << pSocketObj->SocketId()
                             << ", " << strerror(errno) << endl;
                        return -1;
                    }
                }
            }
        }
        pSocketObj->PopOutPacket();
    }

    return 0;
}

int UdpServer::Send(SOCKET socket_id, sockaddr* to_addr, void* buf,
                    size_t nbytes) {
    return socket_manager_.UdpSend(socket_id, to_addr, buf, nbytes);
}

int UdpServer::AddListenSocket(in_addr_t ipv4, in_port_t port) {
    struct sockaddr_in s_addr;
    bzero(&s_addr, sizeof(s_addr));
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(port);
    s_addr.sin_addr.s_addr = htonl(ipv4);
    return socket_manager_.AddUdpSocket((sockaddr*)&s_addr);
}

int UdpServer::RemoveSocket(SOCKET socket_id) { 
    return socket_manager_.CloseSocket(socket_id); 
}

void UdpServer::SetReceivePacketsLimit(SOCKET socket_id, uint32_t num_of_packets) {
    socket_manager_.SetReceivePacketsLimit(socket_id, num_of_packets);
}
