#ifndef __MSTALG__
#define __MSTALG__

#include <common/define.h>

namespace YANGSHUALG {
namespace GRAPHALG {
//最小生成树算法
class MSTAlg:public GraphAlg {
public:
	virtual int Run()=0;
};

class Kruskal:public MSTAlg {
private:
	const Graph *m_graph;
public:
	Kruskal(const Graph *graph):m_graph(graph) {}
	virtual int Run();
};

class Prim:public MSTAlg {
private:
	const Graph *m_graph;
	std::vector<int> m_dist;
public:
	virtual int Run();
};
}
}
#endif
