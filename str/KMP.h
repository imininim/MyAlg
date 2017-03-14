#ifndef __MYKMP_H__
#define __MYKMP_H__

#include <iostream>
#include <string>
#include <vector>

namespace YANGSHUALG {
//KMP匹配
#include <iostream>
#include <string>
#include <vector>

using namespace std;

//KMP匹配
template<typename T>
class KMPMatch {
private:
	std::vector<int> m_next;
	const std::vector<T> &m_strS;	//模式串
	const std::vector<T> &m_strT;	//主字符串
	int GetNext() {
		m_next[0] = -1;
		for (int i = 1; i < m_strS.size(); ++i) {
			int j = m_next[i-1];
			while (j >= 0 && m_strS[i] != m_strS[j+1]) {
				j = m_next[j];
			}
			if (m_strS[i] == m_strS[j+1]) {
				m_next[i] = j+1;
			} else {
				m_next[i] = -1;
			}
		}
		return 0;
	}
	int Match() const {
		int idxS = 0, idxT = 0;
		while (idxS < m_strS.size() && idxT < m_strT.size()) {
			if (m_strS[idxS] == m_strT[idxT]) {
				++idxS;
				++idxT;
			}
			else {
				if (idxS == 0) {
					++idxT;
				}
				else {
					idxS = m_next[idxS-1]+1;
				}
			}
		}
		if (idxS == m_strS.size()) {
			return 1;
		}
		return 0;
	}
public:
	KMPMatch(const std::vector<T> &strS, const std::vector<T> &strT):m_strS(strS),m_strT(strT) {}
	int Run() {
		m_next.resize(m_strS.size());
		GetNext();
		return Match();
	}
	void Show() const {
		for (int i = 0; i < m_next.size(); ++i) {
			cerr << m_next[i] << " ";
		}
		cerr << endl;
	}
};

}
#endif