#include "threadwork.h"

void Thread::Start() {
    pthread_create(&m_threadID, NULL, ThreadFunc, this);  
    cerr << "thread id: " << m_threadID << endl;
}

void Thread::Join() {  
    if (m_threadID > 0) {  
        pthread_join(m_threadID, NULL);  
    }
}

pthread_t Thread::GetThreadID() const {
    return m_threadID;
}

void* Thread::ThreadFunc(void* pth) {
    Thread* p = static_cast<Thread*>(pth);
    p->Run();
    return NULL;
}
