/*
 * File: tcp_server_tester2.h
 * Created Date: June 16th 2022
 * Author: Jerry
 * -----
 * Last Modified:
 * Modified By:
 * -----
 * Copyright (c) 2022 (efancc@163.com)
 */
#if !defined(__TCP_SERVER_TESTER2__)
#define __TCP_SERVER_TESTER2__

#include "tcp_server_base.h"

using namespace std;

class TcpServerTester2 : public TcpServerBase {
   private:
    /* data */
    // DmPacket* p_dm_packet_;

   private:
    /*TcpServerListener Functions */
    virtual int OnConnect(SOCKET socket_id) override;
    virtual int OnDisconnect(SOCKET socket_id) override;
    virtual int DoRead(SOCKET socket_id, void* buf, size_t nbytes) override;

   private:

   public:
    TcpServerTester2();
    ~TcpServerTester2();

    virtual bool Start() override;
    // virtual bool Stop();

    void Demo();
};

#endif // __TCP_SERVER_TESTER2__
