/*
 * key_input_process_thread.cpp
 *
 *  Created on: Jun 29, 2018
 *      Author: jerry
 */
#include "common_include.h"
#include "debug_fun.h"

static char thread_run = 1;
static pthread_t process_thread_id_;

void CancelConsoleInputProcessThread(void) {
    thread_run = 0;
}
static void cleanup_handler(void* arg) {
    std::cout << "Key Input Process Thread: " << __func__ << "()" << endl;
}

void* ConsoleInputProcessThread(void* pApp) {
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

    pthread_cleanup_push(cleanup_handler, pApp);

    printf("按':q'键退出\n");
    while (thread_run) {
        if (terminal_debug() == false)
            break;
        usleep(100 * 1000);
    }

    pthread_cleanup_pop(0);

    pthread_exit((void*)66);
}

int StartConsoleInputProcess() {
    return pthread_create(&process_thread_id_, NULL, ConsoleInputProcessThread, NULL);
}
void StopConsoleInputProcess() {
    CancelConsoleInputProcessThread();
    pthread_join(process_thread_id_, NULL);
}