#ifndef __THREAD_WORK_H__
#define __THREAD_WORK_H__

#include <iostream>
#include <mutex>
#include <queue>

#include <pthread.h>  
using namespace std;

template<typename T>
class LockQueue {
public:
        LockQueue() {}
        std::queue<T> m_queue;
        std::mutex m_mtx;
        bool empty() {
                std::lock_guard<std::mutex> lck (m_mtx);
                return m_queue.empty();
        }
        void push(const T &t) {
                std::lock_guard<std::mutex> lck (m_mtx);
                m_queue.push(t);
        }
        void pop() {
                std::lock_guard<std::mutex> lck (m_mtx);
                m_queue.pop();
        }
        T& front() {
                std::lock_guard<std::mutex> lck (m_mtx);
                return m_queue.front();
        }
        size_t size() {
                std::lock_guard<std::mutex> lck (m_mtx);
                return m_queue.size();
        }
};

class Thread {
public:
    Thread() {}
    virtual ~Thread() {}
    void Start();
    void Join(); 
    pthread_t GetThreadID() const;

    static void* ThreadFunc(void* pth);
    virtual void Run() = 0;
private:  
    pthread_t m_threadID;
};

#endif
