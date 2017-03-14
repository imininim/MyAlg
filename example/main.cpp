#include <iostream>
#include <common/define.h>
#include <graph/NetFlow.h>

using namespace std;

int main() {
	int col, row, cnt;
	cin >> col >> row >> cnt;
	YANGSHUALG::Graph graph(col, row);
	while (cnt --) {
		int from, to, value;
		cin >> from >> to >> value;
		graph.Add(from, to, value);
	}
	graph.ReFlush(1);
	graph.Show();
	cerr << "end" << endl;
	auto edge = graph.GetEdge(1, 2);
	cerr << edge->from << "," << edge->to << "," << edge->value << endl;
	auto retEdge = graph.GetRetEdge(edge);
	cerr << retEdge->from << "," << retEdge->to << "," << retEdge->value << endl;
	return 0;
}

