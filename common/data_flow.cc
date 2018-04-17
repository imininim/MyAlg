#include "data_flow.h"
#include <iostream>
#include <unistd.h>
using namespace std;

namespace common {
namespace data_flow {
void DataWorker::SetComplate(bool isComplate) {
    m_isComplate = isComplate;
}

void DataWorker::Run() {
    while (1) {
        if (m_inputQueue.empty()) {
            if (m_isComplate) {
                break;
            }
            cerr << "sleep" << endl;
            sleep(1);
            continue;
        }
        auto data = m_inputQueue.front();
        do {
            if (data->DoWork()) {
                m_outputQueue.push(data);
            }
        } while(0);
        m_inputQueue.pop();
    }
    cerr << "dataAnaly end" << endl;
}

void DataCollecter::Run() {
        cerr << "in datacollect" << endl;
    int cnt = 0;
    while (1) {
        if (m_outputQueue.empty()) {
            if (m_isComplate) {
                break;
            }
            sleep(1);
            continue;
        }
        auto data = m_outputQueue.front();
        data->DoCollect();
        m_outputQueue.pop();
        if (cnt % 100000 == 0) {
            cerr << "collect " << cnt << endl;
        }   
                cnt ++;
    }   
    cerr << "insert end" << endl;
    return;
}

void DataFlow::Clear() {
    for (int i = 0; i < m_inputnum; ++i) {
        delete m_inputQueues[i];
    }
    for (int i = 0; i < m_outputnum; ++i) {
        delete m_outputQueues[i];
    }
    for (int i = 0; i < m_workernum; ++i) {
        delete m_workList[i];
    }
    for (int i = 0; i < m_collecternum; ++i) {
        delete m_collectList[i];
    }
    m_inputQueues.clear();
    m_outputQueues.clear();
    m_workList.clear();
    m_collectList.clear();
}

bool DataFlow::Init() {
    for (int i = 0; i < m_inputnum; ++i) {
        m_inputQueues.push_back(new InputQueue());
    }
    for (int i = 0; i < m_outputnum; ++i) {
        m_outputQueues.push_back(new OutputQueue());
    }
    for (int i = 0; i < m_workernum; ++i) {
        m_workList.push_back(new DataWorker(*(m_inputQueues[i%m_inputnum]), *(m_outputQueues[i%m_outputnum])));
    }

    for (int i = 0; i < m_collecternum; ++i) {
        m_collectList.push_back(new DataCollecter(*(m_outputQueues[i%m_outputnum])));
    }
    UserInit();
        return true;
}
void DataFlow::Start() {
    for (int i = 0; i < m_workernum; ++i) {
        m_workList[i]->Start();
    }
    for (int i = 0; i < m_collecternum; ++i) {
        m_collectList[i]->Start();
    }
    UserStart();
}
void DataFlow::Join() {
    SetWorkerComplate();
    for (int i = 0; i < m_workernum; ++i) {
        m_workList[i]->Join();
    }
    SetCollecterComplate();
    for (int i = 0; i < m_collecternum; ++i) {
        m_collectList[i]->Join();
    }
}

}
}
