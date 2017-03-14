#include <MST.h>

namespace YANGSHUALG {
namespace GRAPHALG {
	
int Prim::Run() {
	size_t start = 0;
	m_dist.resize(m_graph->GetCol());
	for (size_t i = 0; i < m_graph->m_baseMatrix.size(); ++i) {
		if (i == start) {
			m_dist[i] = -1;
			continue;
		}
		if (m_graph->m_baseMatrix[start][i].value) {
			m_dist[i] = m_graph->m_baseMatrix[start][i].value;
		} else {
			m_dist[i] = -1;
		}
	}

	int idx = -1;
	for (size_t i = 1; i < m_graph->m_calMatrix.size(); ++i) {
		idx = -1;
		int minEdge = 100000;
		for (size_t j = 0; j < m_dist.size(); ++j) {
			if ((m_dist[j] > 0) && (minEdge > m_dist[j])) {
				idx = static_cast<int>(j);
				minEdge = m_dist[j];
			}
		}

		if (idx == -1) {
			break;
		}

		for (size_t j = 0; j < static_cast<size_t>(m_graph->GetRow()); ++j) {
			if (m_dist[j] > m_graph->m_baseMatrix[idx][j].value) {
				m_dist[j] = m_graph->m_baseMatrix[idx][j].value;
			}
		}
	}
	return 0;
}
}
}
