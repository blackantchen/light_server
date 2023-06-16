/*
 * key_input_process_thread.h
 *
 *  Created on: Jun 29, 2018
 *      Author: jerry
 */

#ifndef TARGET_DEVICE_KEY_INPUT_PROCESS_THREAD_H_
#define TARGET_DEVICE_KEY_INPUT_PROCESS_THREAD_H_


extern void *KeyInputProcessThread(void *pApp);
extern void CancelKeyInputProcessThread(void);

int StartConsoleInputProcess();
void StopConsoleInputProcess();

#endif /* TARGET_DEVICE_KEY_INPUT_PROCESS_THREAD_H_ */
