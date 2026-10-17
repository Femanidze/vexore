#ifndef VEXORE_THREADS_H
#define VEXORE_THREADS_H

void ThreadStartAll(void);
void ThreadStopAll(void);
unsigned long __stdcall FeatureLoopThread(void* arg);
unsigned long __stdcall SignatureLoopThread(void* arg);
unsigned long __stdcall LobbyThread(void* arg);

#endif
