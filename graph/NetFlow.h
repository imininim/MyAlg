#ifndef __NETFLOW__
#define __NETFLOW__

#include <common/define.h>
#include <queue>

namespace YANGSHUALG {

namespace GRAPHALG {

//匈牙利算法
class Hungary:public GraphAlg {
private:
	std::vector<int> m_match;	   //储存匹配数据
	std::vector<int> m_isVisit;	 //储存是否已经完成匹配
	const Graph *m_graph;
	bool m_isComplete;
public:
	Hungary(const Graph &graph):m_graph(&graph),m_isComplete(false) {}
	virtual int Run();
	void GetResult(std::vector<std::pair<int, int> > &result, int flag=1) const;
	void GetResult(std::vector<int> &match) const {
		match = m_match;
	}
	int FullMatch();
	bool IsCompleteMatch() const {
		return m_isComplete;
	}
private:
	int Search(int s);
};

class Dinic:public GraphAlg {
private:
	Graph *m_graph;
	int m_start;	//起点
	int m_end;		//终点
	int m_testFlow;	//每次尝试搜索的流量
	int m_maxFlow;	//结果最大流量

	std::queue<int> m_queue;
	std::vector<int> m_deep;
public:
	Dinic(Graph &graph, int start, int end, int testFlow = 100000):m_graph(&graph),m_start(start),m_end(end),m_testFlow(testFlow) {}
	virtual int Run();
public:
	int FindFlow(int start, int end);					//寻找可行路径
	int FullFlow(int start, int end, int flow);		//填充流量
	int ClearQueue();
};

}

}

#endif
