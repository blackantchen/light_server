/*
 * main.cpp
 *
 *  Created on: Dec 1, 2017
 *      Author: jerry
 */
// #include "./utilities/app_timer.h"
#include "./examples/console_input_process_thread.h"
#include "./examples/debug_fun.h"
#include "./include/common_include.h"

#ifdef _DEBUG_EN
#include "../memory_leak_detector/debug_new.h"
#endif

#define _TCP_SERVER_T_EN
#define _UDP_SERVER_T_EN

#ifdef _TCP_SERVER_T_EN
#include "./examples/tcp_server_tester2.h"
#endif

#ifdef _UDP_SERVER_T_EN
#include "./examples/udp_server_tester.h"
#endif

int main(int argc, char* argv[]) {
    StartConsoleInputProcess();

#ifdef _TCP_SERVER_T_EN
    TcpServerTester2* p_tcp_server_tester = new TcpServerTester2();
    p_tcp_server_tester->Start();
#endif

#ifdef _UDP_SERVER_T_EN
    UdpServerTester* p_udp_server_tester = new UdpServerTester();
    p_udp_server_tester->Start();
#endif

    while (1) {
#ifdef _TCP_SERVER_T_EN
        p_tcp_server_tester->Demo();
#endif

#ifdef _UDP_SERVER_T_EN
        p_udp_server_tester->Demo();
#endif

        if (IsExitFlagSet())
            break;
    }

#ifdef _TCP_SERVER_T_EN
    p_tcp_server_tester->Stop();
    delete p_tcp_server_tester;
#endif

#ifdef _UDP_SERVER_T_EN
    p_udp_server_tester->Stop();
    delete p_udp_server_tester;
#endif

    StopConsoleInputProcess();
}
