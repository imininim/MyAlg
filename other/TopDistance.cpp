#include <TopDistance.h>

using namespace std;

//检查是否保持行程包内的城市顺序; 检查各行程包是否相交
bool TopDistance::IsRoutesIllegal(const int ans[], int idx,const LYData *data)
{
    const vector<vector<int>>& referRoutes=data->GetReferRoutes();

    if(referRoutes.size()==0) return false;

    vector<vector<pair<int,int>>> cityOrder;
    cityOrder.resize(referRoutes.size());
    for(int i=0; i<=idx; i++)
    {
        for(int j=0; j<referRoutes.size(); j++)
        {
            bool isFound=false;
            for(int k=0;k<referRoutes[j].size();k++)
            {
                if(referRoutes[j][k]==ans[i])
                {
                    cityOrder[j].push_back(pair<int,int>(ans[i],i));
                    isFound=true;
                    break;
                }
            }
            if(isFound) break;
        }
    }

    //判定是否保序
    for(int i=0;i<cityOrder.size();i++)
    {
        for(int j=0;j<cityOrder[i].size();j++)
        {
            //由cityOrder的生成方式可知,referRoutes可以保证不被越界
            if(cityOrder[i][j].first!=referRoutes[i][j]) return true;
        }
    }

    //判定多个包是否相交;
    if(referRoutes.size()<=1) return false;
    map<int,int>routeOrder;
    for(int i=0; i<cityOrder.size();i++)
    {
        if(cityOrder[i].size()>0)
        {
            routeOrder[cityOrder[i].begin()->second]=cityOrder[i].back().second;
        }
    }
    int rightRange=-1;
    for(auto it=routeOrder.begin();it!=routeOrder.end();it++)
    {
        if(not (it->first>rightRange)) return true;
        rightRange=it->second;
    }

    return false;
}

int TopDistance::CalTopKDist(int cityStatus[], SearchData &preMapDis, std::multimap<int, std::string> &result, int cityNum, int topK,const LYData * data) {
	int ans[32] = {0};  //记录dfs中城市顺序
	int index = 1;		//记录当前第几个城市
	int nowDist = 0;	//记录当前路径得分
	unsigned int status = 1;	//记录哪些城市已经遍历过,开始时出发城市已经遍历过
	std::tr1::unordered_map<int,int> dp;	//优化列表 key(status 已走过的城市) value(剩余城市可以组合出的最小得分)
	DFSearch(cityStatus, index, ans, nowDist, result, status, dp, false, preMapDis, cityNum, topK, data);
	return RUN_OK;
}

void TopDistance::makeValue(const int ans[], const int &len, std::string &value) {
	char tmp[50];
	memset(tmp, '0', sizeof(tmp));
	for(int i = 0; i < len; i ++) {
		if(ans[i] < 10) {
			tmp[(i<<1)|1] += (ans[i]);
		}
		else {
			tmp[i<<1] += ans[i]/10;
			tmp[(i<<1)|1] += (ans[i]%10);
		}
	}
	tmp[len<<1] = 0;
	value = tmp;
}
int TopDistance::DFSearch(const int status[],int idx, int ans[], int nowDist, std::multimap<int,string> &allPath, int nowStatus,
	std::tr1::unordered_map<int,int> &dp, bool hasCache, SearchData &preMapDis, const int &cityNum, const int &topK,const LYData * data) {
	if(idx == cityNum) {
		string value = "";
		makeValue(ans, cityNum, value);
		allPath.insert(pair<int, string>(nowDist, value));
		//当保存的路径数目大于需要的数目时 删除得分最低的一条路径
		if(allPath.size() > topK) {
			std::map<int,string>::iterator it = allPath.end();
			it --;
			allPath.erase(it);
		}
		return 0;//最后一个城市不需要到任何城市,故返回零
	}
	//cerr << "idx = " << idx << endl;
	int min_dis = PRE_PATH_INF_SCORE;
	int min_ret = PRE_PATH_INF_SCORE;
	int sub_min_dis = PRE_PATH_INF_SCORE;
	bool sub_has_cache = (dp.find(~nowStatus)!=dp.end());
	if(sub_has_cache) {
		//如果下层的节点已经遍历完成 并且下层节点不联通则直接返回
		sub_min_dis = dp[~nowStatus];
		if(sub_min_dis >= PRE_PATH_INF_SCORE) {
			return min_dis;
		}
	}
	const vector<KVNode> &tmp = preMapDis[ans[idx-1]];
	//bool flag = 0;
	for(int i = 0; i < tmp.size(); i ++) {
		//cerr << "i = " << i << endl;
		const int &dis = tmp[i].Score;
		const int &id = tmp[i].ID;
		//判断这个节点是否已经走过
		if(nowStatus & (1 << id)) {
			continue;
		}

		//当最后一个城市放到中间时
		if(status[id] == LASTNODE && idx < cityNum-1) {
			continue;
		}
		//如果没有路径
		if(dis == PRE_PATH_INF_SCORE) {
			continue;
		}

		ans[idx] = id;
        if(IsRoutesIllegal(ans,idx,data))
        {
            continue;
        }

		//如果路径超了
		if(allPath.size() >= topK) {
			if(sub_has_cache && hasCache && sub_min_dis + nowDist + dis > allPath.rbegin()->first) { //dp[~now_status]已经完成遍历 &&
				continue;
			}
		}

		//cerr << "tail index = " << index << endl;
		int ret = DFSearch(status, idx+1, ans, nowDist+dis, allPath, nowStatus|(1 << id), dp, sub_has_cache, preMapDis, cityNum, topK, data);

		if(min_ret > ret) {
			min_ret = ret;
		}

		if(min_dis > dis + ret) {
			min_dis = dis + ret;
		}
		//flag = 1;
	}

	if(!sub_has_cache) {
		dp[~nowStatus] = min_ret;
	}
	return min_dis;
}
