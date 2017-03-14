#include <NetFlow.h>

using namespace std;

namespace YANGSHUALG {

namespace GRAPHALG {
int Hungary::Run() {
	m_match.resize(m_graph->GetRow(), -1);
	m_isVisit.resize(m_graph->GetRow(), 0);
	int cnt = 0;
	for(int i = 0; i < m_graph->GetCol(); ++i) {
		for(int j = 0; j < m_graph->GetRow(); ++j) {
			m_isVisit[j] = 0;
		}
		if(m_match[i] == -1) {
			cnt += Search(i);
		}
	}
	if(cnt == m_graph->GetCol()) {
		m_isComplete = true;
	}
	//for(int i = 0; i < m_match.size(); ++i) {
	//	std::cerr << m_match[i] << " ";
	//}
	//std::cerr << std::endl;
	return 0;
}
void Hungary::GetResult(std::vector<std::pair<int, int> > &result, int flag) const {
	result.clear();
	for(size_t i = 0; i < m_match.size(); ++i) {
		if(m_match[i] >= 0 || flag) {
			result.push_back(make_pair(i, m_match[i]));
		}
	}
}

int Hungary::Search(int s) {
	for(size_t i = 0; i < m_graph->m_calMatrix[s].size(); ++i) {
		const int &to = m_graph->m_calMatrix[s][i]->to;
		if(m_isVisit[to]) {
			continue;
		}
		m_isVisit[to] = 1;
		if(m_match[to] == -1 || Search(m_match[to])) {
			m_match[to] = s;
			return 1;
		}
	}
	return 0;
}

int Hungary::FullMatch() {
	m_isVisit.resize(m_graph->GetCol());
	//cerr << "m_isVisit.size() = " << m_isVisit.size() << " Col = " << m_graph->GetCol() << endl;
	for(size_t i = 0; i < m_isVisit.size(); ++i) {
		m_isVisit[i] = 0;
	}
	for(size_t j = 0; j < m_match.size(); ++j) {
		if(m_match[j] >= 0) {
			m_isVisit[m_match[j]] = 1;
		}
	}
	//for(int i = 0; i < m_isVisit.size(); ++i) {
	//	cerr << m_isVisit[i];
	//}
	//cerr << endl;

	for(size_t j = 0; j < m_match.size(); ++j) {
		if(m_match[j] < 0) {
			for(size_t k = 0; k < m_isVisit.size(); ++k) {
				if(m_isVisit[k] == 0) {
					m_isVisit[k] = 1;
					m_match[j] = k;
					break;
				}
			}
		}
	}
	return 0;
}

int Dinic::Run() {
	m_maxFlow = 0;
	ClearQueue();
	while (FindFlow(m_start, m_end)) {
		m_maxFlow += FullFlow(m_start, m_end, m_testFlow);
	}
	return 0;
}

int Dinic::ClearQueue() {
	m_deep.resize(m_graph->GetCol());
	while (!m_queue.empty()) {
		m_queue.pop();
	}
	return 0;
}

int Dinic::FindFlow(int start, int end) {
	for(size_t i = 0; i < m_deep.size(); ++i) {
		m_deep[i] = -1;
	}
	m_deep[start] = 0;
	m_queue.push(start);
	while (!m_queue.empty()) {
		int head = m_queue.front();
		m_queue.pop();
		for (size_t i = 0; i < m_graph->m_calMatrix[head].size(); ++i) {
			const int &to = m_graph->m_calMatrix[head][i]->to;
			const int &value = m_graph->m_calMatrix[head][i]->value;
			if (value <= 0) {
				continue;
			}
			if (m_deep[to] != -1) {
				continue;
			}
			m_deep[to] = m_deep[head] + 1;
			m_queue.push(to);
		}
	}
	return m_deep[end] != -1;
}

int Dinic::FullFlow(int now, int end, int flow) {
	if (now == end) {
		return flow;
	}
	int nowFlow = 0;
	for (size_t i = 0; i < m_graph->m_calMatrix[now].size(); ++i) {
		const int &to = m_graph->m_calMatrix[now][i]->to;
		const int &value = m_graph->m_calMatrix[now][i]->value;
		if(value == 0 || (m_deep[now] != m_deep[to] - 1)) {
			continue;
		}
		if(nowFlow == flow) {
			break;
		}
		int temp = FullFlow(to, end, std::min(flow-nowFlow, value));
		nowFlow += temp;

		m_graph->m_calMatrix[now][i]->value -= temp;
		m_graph->GetRetEdge(m_graph->m_calMatrix[now][i])->value += temp;
	}
	return nowFlow;
}

}

}
