#include <define.h>
#include <iostream>

using namespace std;

namespace YANGSHUALG {

Graph::Graph(int n, int m) {
   Resize(n, m);
}

void Graph::Resize(int n, int m) {
	m_isChange = 0;
	m_baseMatrix.clear();
	m_calMatrix.clear();

	m_baseMatrix.resize(n);
	for(size_t i = 0; i < m_baseMatrix.size(); ++i) {
		m_baseMatrix[i].resize(m);
	}
	m_calMatrix.resize(n);
}
void Graph::Add(int from, int to, int value, int size) {
	m_isChange = 1;
	m_baseMatrix[from][to].value = value;
	m_baseMatrix[from][to].size = size;
}

void Graph::ReFlush(int retEdge, int flag) {
	if(m_isChange == 0) {
		return;
	}
	Clear();
	m_calMatrix.resize(m_baseMatrix.size());
	for(size_t i = 0, cnt = 0; i < m_baseMatrix.size(); ++i) {
		for(size_t j = i+1; j < m_baseMatrix[i].size(); ++j) {
			const int &value1 = m_baseMatrix[i][j].value;
			const int &value2 = m_baseMatrix[j][i].value;

			if(flag) {
				auto edge = new Edge(i, j, cnt, m_baseMatrix[i][j].value, m_baseMatrix[i][j].size);
				m_calMatrix[i].push_back(edge);
				m_assistedMatrix.push_back(edge);
				++cnt;
				edge = new Edge(j, i, cnt, m_baseMatrix[j][i].value, m_baseMatrix[j][i].size);
				m_calMatrix[j].push_back(edge);
				m_assistedMatrix.push_back(edge);
				++cnt;
			} else {
				if(value1) {
					auto edge = new Edge(i, j, cnt, m_baseMatrix[i][j].value, m_baseMatrix[i][j].size);
					m_calMatrix[i].push_back(edge);
					m_assistedMatrix.push_back(edge);
					++ cnt;
				}
				if(value2) {
					auto edge = new Edge(j, i, cnt, m_baseMatrix[j][i].value, m_baseMatrix[j][i].size);
					m_calMatrix[j].push_back(edge);
					m_assistedMatrix.push_back(edge);
					++cnt;
				}
				if(retEdge) {
					if(value1 && !value2) {
						auto edge = new Edge(j, i, cnt, m_baseMatrix[j][i].value, m_baseMatrix[j][i].size);
						m_calMatrix[j].push_back(edge);
						m_assistedMatrix.push_back(edge);
						++cnt;
					} else if (!value1 && value2) {
						auto edge = new Edge(i, j, cnt, m_baseMatrix[i][j].value, m_baseMatrix[i][j].size);
						m_calMatrix[i].push_back(edge);
						m_assistedMatrix.push_back(edge);
						++cnt;
					}
				}
			}
		}
	}
}
int Graph::GetCol() const {
	return m_baseMatrix.size();
}
int Graph::GetRow() const {
	if(m_baseMatrix.size() < 1) {
		return 0;
	}
	return m_baseMatrix[0].size();
}
Edge *Graph::GetRetEdge(const Edge *edge) const {
	int idx = (edge->idx)^1;
	return m_assistedMatrix[idx];
}
Node& Graph::GetNode(int from, int to) {
	return m_baseMatrix[from][to];
}
const Node& Graph::GetNode(int from, int to) const {
	return m_baseMatrix[from][to];
}
Edge* Graph::GetEdge(int from, int to) {
	for (size_t i = 0; i < m_calMatrix[from].size(); ++i) {
		if (m_calMatrix[from][i]->to == to) {
			return (m_calMatrix[from][i]);
		}
	}
	return NULL;
}
const Edge* Graph::GetEdge(int from, int to) const {
	for (size_t i = 0; i < m_calMatrix[from].size(); ++i) {
		if (m_calMatrix[from][i]->to == to) {
			return (m_calMatrix[from][i]);
		}
	}
	return NULL;
}
int Graph::Show() const {
	for (size_t i = 0; i < m_baseMatrix.size(); ++i) {
		for (size_t j = 0; j < m_baseMatrix[i].size(); ++j) {
			cerr << "(" << m_baseMatrix[i][j].value << "," << m_baseMatrix[i][j].size << ") ";
		}
		cerr << std::endl;
	}
	return 0;
}
void Graph::Clear() {
	for(size_t i = 0; i < m_assistedMatrix.size(); ++i) {
		delete m_assistedMatrix[i];
	}
	m_assistedMatrix.clear();
	m_calMatrix.clear();
}
Graph::~Graph() {
	Clear();
}
}
