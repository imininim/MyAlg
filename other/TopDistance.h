#ifndef __TOPDISTANCE_H__
#define __TOPDISTANCE_H__

#include <string>
#include <vector>
#include <tr1/unordered_map>
#include <map>
#include <LYConstData.h>
#include <string.h>
#include <LYData.h>

struct KVNode {
	int ID;			//下标
	int Score;		//得分
	bool operator <(const KVNode &rhs) const {
		return Score < rhs.Score;
	}
};

//求top距离的算法
class TopDistance {
public:
enum { LASTNODE=100, PRE_PATH_INF_SCORE=10000000 };
typedef std::tr1::unordered_map<int, std::vector<KVNode> >  SearchData;
/*深搜算法，求出top k的哈密顿路径
 * status 表示每个点的状态 比如某些点只能在第一位,某些点只能在最后一位
 * index 当前所在城市 初始 0
 * ans 当前已走过城市的顺序 初始 [1, 0, 0, ...]
 * nowDist 当前距离 初始 0
 * allPath 存储的最终结果 空
 * nowStatus 当前已走过城市的状态 初始 1 只有第一个城市去过
 * dp 走过城市的状态集合 初始空
 * hasCache 是否已经有dp 初始0
 * preMapDis 各个节点直接的距离
 * cityNum 城市数目
 * topK 需要保留的线路数目
 * */

static int CalTopKDist(int cityStatus[], SearchData &preMapDis, std::multimap<int, std::string> &result, int cityNum, int topK, const LYData * data);

private:
static int DFSearch(const int status[],int idx, int ans[], int nowDist, std::multimap<int, std::string> &allPath, int nowStatus,
	std::tr1::unordered_map<int,int> &dp, bool hasCache, SearchData &preMapDis, const int &cityNum, const int &topK, const LYData * data);

static void makeValue(const int ans[], const int &len, std::string &value);
static bool IsRoutesIllegal(const int ans[],int idx, const LYData *data);

};

#endif

