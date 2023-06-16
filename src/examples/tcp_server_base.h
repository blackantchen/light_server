/*
 * File: tcp_server_base.h
 * Created Date: June 16th 2022
 * Author: Jerry
 * -----
 * Last Modified:
 * Modified By:
 * -----
 * Copyright (c) 2022 (efancc@163.com)
 */
#if !defined(__TCP_SERVER_BASE__)
#define __TCP_SERVER_BASE__

#include "../socket_manager/socket_interface.h"
#include "../socket_manager/tcp_server.h"

using namespace std;

class TcpServerBase : protected TcpServerListener {
   protected:
    TcpServer* p_tcp_server_;

   protected:
    TcpServer* pServer() { return p_tcp_server_; };

    /*TcpServerListener Functions */
    virtual int OnConnect(SOCKET socket_id) override;
    virtual int OnDisconnect(SOCKET socket_id) override;
    virtual int DoRead(SOCKET socket_id, void* buf, size_t nbytes) override;
    /*TcpServerListener Functions --- END --- */

   public:
    TcpServerBase();
    virtual ~TcpServerBase();

    virtual bool Start() = 0;
    virtual bool Stop();

    virtual int SendMsg(SOCKET socket_id, void* buf, size_t nbytes);
};

#endif  // __TCP_SERVER_BASE__
