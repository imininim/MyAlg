#ifndef __DATA_FLOW_H__
#define __DATA_FLOW_H__

#include <vector>
#include <memory>
#include "threadwork.h"

namespace common {
namespace data_flow {

class Data;

typedef std::shared_ptr<Data> DataPtr;
typedef LockQueue<DataPtr>* InputQueuePtr;
typedef LockQueue<DataPtr>* OutputQueuePtr;
typedef LockQueue<DataPtr> InputQueue;
typedef LockQueue<DataPtr> OutputQueue;

class Data {
public:
        virtual bool DoWork() { return true; };
        virtual void DoCollect() {};
};

class DataWorker: public Thread {
private:
    InputQueue  &m_inputQueue;
    OutputQueue &m_outputQueue;
    bool m_isComplate;
public:
    DataWorker(InputQueue &inputQueue, OutputQueue &outputQueue):
    m_inputQueue(inputQueue), m_outputQueue(outputQueue), m_isComplate(false) {}
    void SetComplate(bool isComplate);
    virtual void Run();
};

class DataCollecter: public Thread {
private:
    OutputQueue &m_outputQueue;
    bool m_isComplate;
public:
    DataCollecter(OutputQueue &outputQueue):m_outputQueue(outputQueue), m_isComplate(false) {}
    void SetComplate(bool isComplate) {
        m_isComplate = isComplate;
    }
    virtual void Run();
};

class DataFlow {
private:
    std::vector<InputQueuePtr>   m_inputQueues;
    std::vector<OutputQueuePtr>  m_outputQueues;
    std::vector<DataWorker *>    m_workList;
    std::vector<DataCollecter *> m_collectList;
public:
    int m_inputnum;
    int m_outputnum;
    int m_workernum;
    int m_collecternum;
public:
    DataFlow():m_inputnum(1), m_outputnum(1),m_workernum(1), m_collecternum(1) {}
    DataFlow(int inputnum, int outputnum, int workernum, int collecternum):m_inputnum(inputnum), m_outputnum(outputnum),m_workernum(workernum), m_collecternum(collecternum) {}
    ~DataFlow() {
        Clear();
    }

        void Put(DataPtr data) {
                static int n = 0;
                m_inputQueues[n%m_inputnum]->push(data);
                n++;
        }
    void SetCollecterComplate() {
        for (int i = 0; i < m_collecternum; ++i) {
            m_collectList[i]->SetComplate(true);
        }
    }
    void SetWorkerComplate() {
        for (int i = 0; i < m_workernum; ++i) {
            m_workList[i]->SetComplate(true);
        }
    }

    void Clear();
    bool Init();
    void Start();
    void Join();
    virtual void UserStart() {}
    virtual bool UserInit() {
        return true;
    }
};
}
}

#endif
