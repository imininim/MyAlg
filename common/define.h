#ifndef __YANGSHU_DEFINE__
#define __YANGSHU_DEFINE__

#include <vector>

namespace YANGSHUALG {
//边
class Edge {
public:
	int from;
	int to;
	int idx;
	int value;
	int size;
public:
	Edge(int f, int t, int i, int v, int s):from(f), to(t), idx(i), value(v), size(s) {}
	Edge() {}
};

//点
class Node {
public:
	int value;
	int size;
public:
	Node():value(0),size(0) {}
};

//图
class Graph {
private:
	int m_isChange;
public:
	void Resize(int n, int m);
	void Add(int from, int to, int value = 1, int size = 1);
	Edge *GetRetEdge(const Edge *edge) const;
	Node& GetNode(int from, int to);
	const Node& GetNode(int from, int to) const;
	Edge* GetEdge(int from, int to);
	const Edge* GetEdge(int from, int to) const;
	Graph(int n, int m);
	~Graph();
public:
	std::vector<std::vector<Node> > m_baseMatrix;	//基础矩阵
	std::vector<std::vector<Edge *> > m_calMatrix;	 //计算矩阵
	std::vector<Edge *> m_assistedMatrix;			//辅助矩阵
public:
	void ReFlush(int retEdge=0, int flag=0);		//第一个参数:是否建立反向边 第二个参数:是否建立无用边
	int GetCol() const;
	int GetRow() const;
	int Show() const;
private:
	void Clear();
};

namespace GRAPHALG {

class GraphAlg {
public:
	virtual int Run()=0;
	virtual ~GraphAlg() {} 
};

}

}
#endif