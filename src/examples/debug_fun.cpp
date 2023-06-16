/*
 * debug_fun.cpp
 *
 *  Created on: Mar 23, 2018
 *      Author: jerry
 */

#include "common_include.h"

#ifdef _DEBUG_EN
#include "../memory_leak_detector/debug_new.h"
#endif

#define OK 0
#define NO_INPUT 1
#define TOO_LONG 2

static uint8_t ExitFlag = 0;

// extern int RtcTest(void);

static int getLine(char *prmpt, char *buff, size_t sz) {
    int ch, extra;
    // Get line with buffer overrun protection.
    if (prmpt != NULL) {
        printf("%s", prmpt);
        fflush(stdout);
    }
    if (fgets(buff, sz, stdin) == NULL) return NO_INPUT;

    // If it was too long, there'll be no newline. In that case, we flush
    // to end of line so that excess doesn't affect the next call.
    if (buff[strlen(buff) - 1] != '\n') {
        extra = 0;
        while (((ch = getchar()) != '\n') && (ch != EOF)) extra = 1;
        return (extra == 1) ? TOO_LONG : OK;
    }

    // Otherwise remove newline and give string back to caller.
    buff[strlen(buff) - 1] = '\0';
    return OK;
}

void CancelThreads() {
    ExitFlag = 1;
}

bool terminal_debug() {
    char keyBuff[30];
    int rc;

    rc = getLine(NULL, keyBuff, sizeof(keyBuff));
    if (rc == OK) {
        if (strcmp(keyBuff, ":q") == 0) {
            printf("Over\n");
            CancelThreads();
            return false;
        } else if (strcmp(keyBuff, ":e") == 0) {
        } else {
        }
    }

    return true;
}

bool IsExitFlagSet(void) {
    if (ExitFlag != 0)
        return true;
    else
        return false;
}
