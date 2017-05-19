#include "MyCluster.h"
#include "Route/base/define.h"
#include "BagPlan.h"
#include "SPath.h"
#include "Route/base/BagParam.h"
#include "AggCluster.h"
#include "StaticRand.h"
#include "BagPath.h"
#include "MJCommon.h"
#include "MyAlg.h"

namespace CLUSTERALG {

void Result::Show() const {
	for(int i = 0; i < clusterResult.size(); ++i) {
		cerr << i << ": ";
		for(auto it = clusterResult[i].begin(); it != clusterResult[i].end(); ++it) {
			cerr << *it << " ";
		}
		cerr << endl;
	}
}

void ClusterInfo::Show() const {
	cerr << "Cluster:" << endl;
	m_result.Show();
	for(int i = 0; i < m_edges.size(); i ++) {
		cerr << m_edges[i].idx << " - " << m_edges[i].idy << " dist = " << m_edges[i].value << endl;
	}
}

Cluster::Cluster(const BaseMatrix *baseMatrix, int clusterNum):m_baseMatrix(baseMatrix),m_clusterNum(clusterNum),m_isComplate(false) {
	for(int i = 0; i < baseMatrix->size(); ++i) {
		vector<Node> list;
		for(int j = 0; j < (*baseMatrix)[i].size(); ++j) {
			list.push_back(Node(j, (*baseMatrix)[i][j]));
		}
		sort(list.begin(), list.end());
		m_calMatrix.push_back(list);
	}
}

int Kmeans::ShowBaseMatrix() const {
	cerr << "BaseMatrix:" << endl;
	int len = (*m_baseMatrix).size();
	for(int i = 0; i < len; ++i) {
		for(int j = 0; j < len; ++j) {
			cerr << (*m_baseMatrix)[i][j] << " ";
		}
		cerr << endl;
	}
	return 0;
}

void HierarchicalClustering::ShowResult() const {
	for(int i = 0; i < m_result.size(); ++i) {
		cerr << "cluster i: " << i << endl;
		m_result[i].Show();
	}
}

int HierarchicalClustering::CalClusterDist(ClusterInfo &clusterInfo) {
	//聚类数据改变
	const Result &result = clusterInfo.m_result;
	vector<Edge> &edges = clusterInfo.m_edges;
	edges.clear();
	for(int i = 0; i < result.clusterResult.size(); ++i) {
		if(result.clusterResult[i].empty()) {
			continue;
		}
		for(int j = i+1; j < result.clusterResult.size(); ++j) {
			if(result.clusterResult[j].empty()) {
				continue;
			}
			int dist = GetClusterDist(result, i, j);
			edges.push_back(Edge(i, j, dist));
		}
	}
	sort(edges.begin(), edges.end());
	return 0;
}

int HierarchicalClustering::Run() {
	ClusterInfo clusterInfo;
	for(int i = 0; i < m_calMatrix.size(); ++i) {
		std::tr1::unordered_set<int> set;
		set.insert(i);
		clusterInfo.m_result.clusterResult.push_back(set);
	}
	while(clusterInfo.m_result.clusterResult.size() < m_clusterNum) {
		std::tr1::unordered_set<int> set;
		clusterInfo.m_result.clusterResult.push_back(set);
	}
	CalClusterDist(clusterInfo);
	//if(1) {
	//	cerr << "in HierarchicalClustering" << endl;
	//	clusterInfo.Show();
	//}
	DfsCluster(0, clusterInfo);
	return 0;
}

//层次聚类过程
int HierarchicalClustering::DfsCluster(int depth, const ClusterInfo &clusterInfo) {
	//当前聚类数目
	int nowCluster = m_baseMatrix->size()-depth;
	//三类过程
	if(nowCluster < m_clusterNum) {
		m_result.push_back(clusterInfo.m_result);
	}
	else if(nowCluster == m_clusterNum) {
		m_result.push_back(clusterInfo.m_result);
	}
	else if(nowCluster > m_clusterNum) {
		int MaxCluster = 3;
		if(depth >= 3) {
			MaxCluster = 1;
		}
		ClusterInfo tmp;
		for(int i = 0, cnt = 0; i < clusterInfo.m_edges.size() && cnt < MaxCluster; ++i) {
			//如果需要多于一个解则进行复制,否则不需要复制
			if(!CanMerge(clusterInfo, i, depth)) {
				continue;
			}
			ClusterInfo *useCluster = NULL;
			if(MaxCluster > 1) {
				tmp = clusterInfo;	//用户记录中间状态
				Merge(tmp, i);		//合并两个集合
				useCluster = &tmp;
			}
			else {
				useCluster = const_cast<ClusterInfo *>(&clusterInfo);
				Merge(*useCluster, i);		//合并两个集合
			}
			DfsCluster(depth+1, *useCluster);
			++cnt;
		}
	}
	return 0;
}

//层次聚类合并集合
int HierarchicalClustering::CanMerge(const ClusterInfo &clusterInfo, int idx, int depth) {
	//合并集合 xid, yid
	int xid = clusterInfo.m_edges[idx].idx, yid = clusterInfo.m_edges[idx].idy;
	//集合改变
	const Result &result = clusterInfo.m_result;
	std::vector<std::vector<int> > tmp;
	tmp.reserve(result.clusterResult.size());
	for(int i = 0; i < result.clusterResult.size(); ++i) {
		if(i == yid) {
			continue;
		}
		std::vector<int> v;
		for(auto it = result.clusterResult[i].begin(); it != result.clusterResult[i].end(); ++it) {
			v.push_back(*it);
		}
		if(i == xid) {
			for(auto it = result.clusterResult[yid].begin(); it != result.clusterResult[yid].end(); ++it) {
				v.push_back(*it);
			}
		}
		sort(v.begin(), v.end());
		tmp.push_back(v);
	}
	sort(tmp.begin(), tmp.end());
	if(m_hasVisitSet[depth].find(tmp) == m_hasVisitSet[depth].end()) {
		m_hasVisitSet[depth].insert(tmp);
		return 1;
	}
	return 0;
}

//层次聚类合并集合
int HierarchicalClustering::Merge(ClusterInfo &clusterInfo, int idx) {
	//合并集合 xid, yid
	int xid = clusterInfo.m_edges[idx].idx, yid = clusterInfo.m_edges[idx].idy;
	//集合改变
	Result &result = clusterInfo.m_result;
	result.clusterResult[xid].insert(result.clusterResult[yid].begin(), result.clusterResult[yid].end());
	result.clusterResult[yid].clear();
	
	//聚类之间的距离改变
	CalClusterDist(clusterInfo);
	return 0;
}

//两个聚类之间的距离
int HierarchicalClustering::GetClusterDist(const Result &result, int clusterA, int clusterB) {//计算两个类间的“距离”，取所有距离对的前三短的平均值
	auto setA = result.clusterResult[clusterA];
	auto setB = result.clusterResult[clusterB];
	std::set<int> distSet;
	for(auto itA = setA.begin(); itA != setA.end(); ++itA) {
		for(auto itB = setB.begin(); itB != setB.end(); ++itB) {
			distSet.insert(((*m_baseMatrix)[*itA][*itB]));
			if(distSet.size() > 3) {
				distSet.erase(--distSet.end());
			}
		}
	}
	int sum = 0, num = distSet.size();
	if(num == 0) {
		num = 1;
	}
	for(auto it = distSet.begin(); it != distSet.end(); ++it) {
		sum += *it;
	}
	return sum/num;
}

int Kmeans::GetCenter(const std::tr1::unordered_set<int> &cluster) {
	int minIdx = -1;
	int minDist = 9999999;
	if(cluster.size() > 0) {
		minIdx = *cluster.begin();
	}
	for (auto cit = cluster.begin(); cit != cluster.end(); ++cit) {
		int dist = 0;
		for (auto cit1 = cluster.begin(); cit1 != cluster.end(); ++cit1) {
			if (*cit1 == *cit) {
				continue;
			}
			dist += (*m_baseMatrix)[*cit][*cit1];
		}
		if (dist < minDist) {
			minDist = dist;
			minIdx = *cit;
		}
	}
	return minIdx;
}

int Kmeans::GetAllCenters(const Result &cluster, std::vector<int> &nodes) {
	for (int i = 0; i < cluster.clusterResult.size(); ++i) {
		//枚举每个集合内部的点
		int minIdx = GetCenter(cluster.clusterResult[i]);
		nodes.push_back(minIdx);
	}
	return 0;
}

}

namespace YANGSHU {

using namespace CLUSTERALG;

static int __debug_level__ = 0;

ClusterOrganizer::ClusterOrganizer(BagPlan* bagPlan, std::vector<const SPath*> *rootList):m_bagPlan(bagPlan),m_rootList(rootList) {}

int ClusterOrganizer::GetPlanPlace(std::vector<const VarPlace *> &vList, int &clusterNum, int select, int loop) {
	int RELAX = 1800;
	int vLen = 0;
	int totDur = 0; //总剩余时间
	for(int i = 0; i < m_bagPlan->GetBlockNum(); ++i) {
		totDur += m_bagPlan->GetBlock(i)->_avail_dur;
	}
	//设置随机种子
	int randomSeed = loop;
	int seedId = loop % StaticRand::Capacity();
	double rval = StaticRand::Get(seedId) / static_cast<double>(StaticRand::Max());
	int keptDur = static_cast<int>(totDur * 3.0 * rval / 10.0); //在可选点中随机让可用时长选取70～100%的总时间，以防一条结果都不出现
	int criticalDur = totDur * 2 / 3;//当剩余时间小于该值后，启动随机选点机制

	//计算总的游玩天数和总的最大游玩时长
	int guessDayNum = 0;
	int maxBlockDur = -1;
	for(int i = 0; i < m_bagPlan->GetBlockNum(); ++i) {//计算确实有剩余时间能够填充景点的block数量
		int blockDur = m_bagPlan->GetBlock(i)->_avail_dur;
		if(blockDur > 5400) {
			++guessDayNum;
		}
		if(blockDur > maxBlockDur) {
			maxBlockDur = blockDur;
		}
	}
	//计算景点个数
	int posPoiNum = 0;
	for(int vPos = 0; vPos < m_bagPlan->PosNum(); ++vPos) {
		const VarPlace* vPlace = dynamic_cast<const VarPlace*>(m_bagPlan->GetPosPlace(vPos));
		if (NULL == vPlace) continue;
		posPoiNum ++;
	}

	//选择
	float noMissDropProbability = 0.2;
	float otherDropProbability = 0.2;
	if (select == 0) {
		noMissDropProbability = 0;
		otherDropProbability = 0.2;
	} else if (select == 1) {
		noMissDropProbability = 0.2;
		otherDropProbability = 0.2;
	} else if (select == 2) {
		noMissDropProbability = 0.5;
		otherDropProbability = 0.5;
	} else if (select == 3) {
		noMissDropProbability = 0.9;
		otherDropProbability = 0.5;
	} else {
		cerr << "select error: " << select << endl;
	}

	std::tr1::unordered_set<uint8_t> addVPosSet;
	int minPOIdur = 999999;
	int totalUsedDur = 0;

	//必选点
	for (int vPos = 0; vPos < m_bagPlan->PosNum(); ++vPos) {//添点逻辑仿写于Richer.cpp
		//某种条件下随机删点
		const VarPlace* vPlace = dynamic_cast<const VarPlace*>(m_bagPlan->GetPosPlace(vPos));
		if (NULL == vPlace) {
			continue;
		}
		if (addVPosSet.find(vPos) != addVPosSet.end()) {
			continue;
		}
		//非必选点直接跳过
		if (m_bagPlan->m_nonMissSet.find(vPlace) == m_bagPlan->m_nonMissSet.end()) {
			continue;
		}
		//有概率跳过
		{
			double rval = StaticRand::Get(loop++) / static_cast<double>(StaticRand::Max());
			loop = loop % StaticRand::Capacity();
			if (rval < noMissDropProbability) continue; 
		}
		if ((totDur < (m_bagPlan->GetMinAllocDur(vPlace) + keptDur))) {
		   	continue;
		}
		
		vList.push_back(vPlace);
		addVPosSet.insert(vPos);
		int allocDur = m_bagPlan->GetAllocDur(vPlace);
		//记录最小的游玩时间
		if(allocDur < minPOIdur) {
			minPOIdur = allocDur;
		}
		totDur -= allocDur;
		totalUsedDur += allocDur;
	}

	//可选点
	int criticalPoiNum = posPoiNum * 2 / 3;
	int totPoiNumLimit = guessDayNum * (BagParam::m_dayViewLimit - 1);
	for(int vPos = 0; vPos < m_bagPlan->PosNum(); ++vPos) {
		if(vList.size() > totPoiNumLimit) {
			break;
		}
		if(addVPosSet.find(vPos) != addVPosSet.end()) {
			continue;
		}
		//概率跳过
		{
			double rval = StaticRand::Get(loop++) / static_cast<double>(StaticRand::Max());
			loop = loop % StaticRand::Capacity();
			if (rval < otherDropProbability) continue; 
		}

		const VarPlace *vPlace = dynamic_cast<const VarPlace*>(m_bagPlan->GetPosPlace(vPos));
		if(NULL == vPlace) {
			continue;
		}
		if (m_bagPlan->m_userOptSet.find(vPlace) == m_bagPlan->m_userOptSet.end()) continue;
		if ((totDur < (m_bagPlan->GetAllocDur(vPlace) + keptDur))) {
			continue;
		}

		vList.push_back(vPlace);
		addVPosSet.insert(vPos);
		int allocDur = m_bagPlan->GetAllocDur(vPlace);
		if(allocDur < minPOIdur) {
			minPOIdur = allocDur;
		}
		totDur -= allocDur;
		totalUsedDur += allocDur;
	}

	for (int vPos = 0; vPos < m_bagPlan->PosNum(); ++vPos) {
		if (vList.size() > totPoiNumLimit) break;
		if (addVPosSet.find(vPos) != addVPosSet.end()) continue;
		//概率跳过
		{
			double rval = StaticRand::Get(loop++) / static_cast<double>(StaticRand::Max());
			loop = loop % StaticRand::Capacity();
			if (rval < otherDropProbability) continue; 
		}
		const VarPlace *vPlace = dynamic_cast<const VarPlace*>(m_bagPlan->GetPosPlace(vPos));
		if (NULL == vPlace) continue;
		if ((totDur < (m_bagPlan->GetAllocDur(vPlace) + keptDur))) continue;

		vList.push_back(vPlace);
		addVPosSet.insert(vPos);
		int allocDur = m_bagPlan->GetAllocDur(vPlace);
		//记录最小的游玩时间
		if(allocDur < minPOIdur) {
			minPOIdur = allocDur;
		}
		totDur -= allocDur;
		totalUsedDur += allocDur;
	}

	minPOIdur = ((5400>minPOIdur)?5400:minPOIdur);

	clusterNum = 0;
	for(int i = 0; i < m_bagPlan->GetBlockNum(); ++i) {
		int blockDur = m_bagPlan->GetBlock(i)->_avail_dur;
		if(blockDur > minPOIdur) {
			clusterNum ++;
		}
	}

	return 0;
}

int ClusterOrganizer::List2Matrix(const std::vector<const VarPlace *> &vList, CLUSTERALG::BaseMatrix &matrix) {
	for (int i = 0; i < vList.size(); ++i) { //填充地图信息
		const VarPlace *vPlace = vList[i];
		vector<int> list;
		for (int j = 0; j < vList.size(); ++j) {
			const VarPlace *wPlace = vList[j];
			int dist = LYConstData::CaluateSphereDist(vPlace, wPlace);
			if(PlaceGroup::IsFreq(vPlace->_ID, wPlace->_ID)) {
				dist = 0;
			}
			if(dist < 0) {
				dist = 0;
			}
			list.push_back(dist);
			//m_metricCoordDist[i][j] = LYConstData::CaluateSphereDist(vPlace, wPlace);
		}
		matrix.push_back(list);
	}
	return 0;
}

int ClusterOrganizer::Run() {
	//int select = Nomiss_all;
	//int select = Nomiss_all_Others_random;
	std::vector<NodeInfo> nodeInfo;	//每个节点的关系
	std::vector<BlockInfo> blockInfo;	//block信息
	for(int loop = 0; loop < 100; ++loop) {
		nodeInfo.clear();
		blockInfo.clear();
		//选点
		int clusterNum = 0;
		std::vector<const VarPlace *> vList;
		//选点逻辑需要同步
		GetPlanPlace(vList, clusterNum, loop%4, loop);
		//准备补入 m_bindInfo m_blockInfo
		InitInfo(vList, nodeInfo, blockInfo);
		
		//选点去重，如果这些点已经选过一次，那么第二次出现时聚类结果肯定一样
		auto hash = CalHashPointSet(nodeInfo);
		if(HasVisit(hash)) {
			cerr << "has Same Cluster" << endl;
			continue;
		}
		// -------------------------
		if(__debug_level__ > 5) {
			cerr << "clusterNum = " << clusterNum << endl;
			for(int i = 0; i < vList.size(); i ++) {
				cerr << vList[i]->_name << endl;
			}
		}
		//转成矩阵
		CLUSTERALG::BaseMatrix matrix;
		List2Matrix(vList, matrix);

		if(__debug_level__ > 8) {
			for(int i = 0; i < matrix.size(); i ++) {
				for(int j = 0; j < matrix[i].size(); j ++) {
					cerr << matrix[i][j] << " ";
				}
				cerr << endl;
			}
		}

		//层次聚类,hash去重
		CLUSTERALG::HierarchicalClustering h(&matrix, clusterNum);
		h.Run();

		//将结果合并
		auto tmp = h.GetResult();
		CLUSTERALG::ClusterResult results;
		for(int i = 0; i < tmp.size(); ++i) {
			auto &t = tmp[i];
			CLUSTERALG::Result result;
			result.clusterResult.reserve(clusterNum);
			for(int j = 0; j < t.clusterResult.size(); ++j) {
				if(t.clusterResult[j].size() != 0) {
					result.clusterResult.push_back(t.clusterResult[j]);
				}
			}
			results.push_back(result);
		}

		if(__debug_level__ > 8) {
			ShowCluster(&results, vList);
		}

		CLUSTERALG::ClusterResult lastResults;
		for(int i = 0; i < results.size(); ++i) {
			if(__debug_level__ > 5) {
				cerr << "kmeans i = " << i << endl;
			}
			MyKmeans myKmeans(&(results[i]), &matrix, clusterNum, m_bagPlan, vList, nodeInfo, blockInfo);
			myKmeans.Run();
			auto &result = myKmeans.GetResult();
			if(result.size() > 0) {
				//cerr << "push result" << endl;
				lastResults.push_back(result[0]);
			}
		}
		//结果转成线路
		//cerr << "yangshu size = " << lastResults.size() << endl;
		ClusterDumpToSPath(lastResults, nodeInfo, blockInfo);
		//if(m_rootList->size() == 0) {
		//	select = Nomiss_random;
		//}
		//else {
		//	select = Nomiss_all_Others_random;
		//}
		if (m_bagPlan->IsRootTimeOut()) {
			MJ::PrintInfo::PrintLog("[%s]Cluster::DoCluster, TIMEOUT path length now: %d, iter: %d", m_bagPlan->m_qParam.log.c_str(), m_rootList->size(),loop);
			break;
		}
	}
	//最终线路筛选
	std::sort(m_rootList->begin(), m_rootList->end(), SPathCmp());
	if(m_rootList->size() > 1000) {
		std::vector<const SPath*>::iterator tempSPathIt = m_rootList->begin() + 1000;
		for (; tempSPathIt != m_rootList->end(); tempSPathIt++) {
			delete *tempSPathIt;
		}
		m_rootList->erase(m_rootList->begin() + 1000, m_rootList->end());
	}
	for(int i = 0; i < m_rootList->size() && i < 5; ++i) {
		((*m_rootList)[i])->DumpDetial(m_bagPlan, true);
	}
	return 0;
}

int ClusterOrganizer::InitInfo(std::vector<const VarPlace *> &vList, std::vector<NodeInfo> &m_bindInfo, std::vector<BlockInfo> &m_blockInfo) {
	m_blockInfo.clear();
	m_bindInfo.clear();
	//确定最小景点的游玩时间
	int minPOIDur = 999999;
	for(int i = 0; i < vList.size(); ++i) {
		int dur = m_bagPlan->GetAllocDur(vList[i]);
		if(dur < minPOIDur) {
			minPOIDur = dur;
		}
	}
	//一天的游玩时间最少是2个小时,并且要多余最小景点的游玩时间
	minPOIDur = ((minPOIDur>5400)?minPOIDur:5400);
	
	//记录一天最多的游玩时间
	int maxBlockDur = -1;
	int RELAX = 1800;
	for(int i = 0; i < m_bagPlan->GetBlockNum(); ++i) {
		int blockDur = m_bagPlan->GetBlock(i)->_avail_dur;
		if(maxBlockDur < blockDur) {
			maxBlockDur = blockDur;
		}
		if (blockDur > minPOIDur) {
			BlockInfo blockInfo;
			blockInfo.m_idx = i;
			blockInfo.m_relax = m_bagPlan->GetBlock(i)->_avail_dur + RELAX;
			blockInfo.m_restNum = m_bagPlan->GetBlock(i)->_restNum;
			blockInfo.m_dur = m_bagPlan->GetBlock(i)->_avail_dur;
			m_blockInfo.push_back(blockInfo);
		}
	}

	for(int i = 0; i < vList.size(); ++i) {
		NodeInfo nodeInfo;
		nodeInfo.m_idx = 0;
		for(int j = 0; j < m_bagPlan->PosNum(); ++j) {
			if(vList[i] == m_bagPlan->GetPosPlace(j)) {
				nodeInfo.m_idx = j;
				break;
			}
		}
		nodeInfo.m_place = vList[i];
		nodeInfo.m_dur = m_bagPlan->GetAllocDur(vList[i]);
		//游玩时间超过最长的一天时,这一天为该景点的游玩时长
		if(nodeInfo.m_dur > maxBlockDur) {
			nodeInfo.m_dur = maxBlockDur;
		}
		for(int j = 0; j < vList.size(); ++j) {
			if(i == j) {
				nodeInfo.m_bindInfo.push_back(j);
			}
			else if(PlaceGroup::IsFreq(vList[i]->_ID, vList[j]->_ID)) {
				nodeInfo.m_bindInfo.push_back(j);
			}
		}
		m_bindInfo.push_back(nodeInfo);
	}
	
	sort(m_blockInfo.begin(), m_blockInfo.end(), BlockInfo::CmpOfRelax);
	return 0;
}

int ClusterOrganizer::ShowCluster(const CLUSTERALG::ClusterResult *result, const std::vector<const VarPlace *> &vList) const {
	for(int i = 0; i < result->size(); ++i) {
		cerr << "Cluster: " << i << endl;
		auto &tmp = (*result)[i];
		for(int j = 0; j < tmp.clusterResult.size(); ++j) {
			cerr << "i: " << j << " :";
			for(auto it = tmp.clusterResult[j].begin(); it != tmp.clusterResult[j].end(); ++it) {
				cerr << vList[*it]->_name << " ";
			}
			cerr << endl;
		}
	}
	return 0;
}

int ClusterOrganizer::CalViewOpenCloseInfo(const CLUSTERALG::Result &result, std::vector<std::vector<int> > &matrix, const std::vector<NodeInfo> &m_bindInfo, const std::vector<BlockInfo> &m_blockInfo) {
	if(__debug_level__ > 5) {
		cerr << "in OpenClose" << endl;
		for(int i = 0; i < m_bagPlan->PosNum(); ++i) {
			const VarPlace* vPlace = dynamic_cast<const VarPlace*>(m_bagPlan->GetPosPlace(i));
			if(vPlace == NULL) {
				continue;
			}
			cerr << vPlace->_name << " - " << i << endl;
		}
	}
	for(int i = 0; i < result.clusterResult.size(); ++i) {
		int dur = 0;
		for(auto it = result.clusterResult[i].begin(); it != result.clusterResult[i].end(); ++it) {
			dur += m_bindInfo[*it].m_dur;
		}

		for(int j = 0; j < m_blockInfo.size(); ++j) {
			if(m_blockInfo[j].m_relax > dur) {
				int idx = m_blockInfo[j].m_idx;
				int avial = 2;
				for(auto it = result.clusterResult[i].begin(); it != result.clusterResult[i].end(); ++it) {
					int vPos = m_bindInfo[*it].m_idx;
					if(m_bagPlan->GetVType(idx, vPos) == NODE_TYPE_NULL) {//开关门判断，摘自Richer.cpp
						avial = 1;
						break;
					}
				}
				matrix[i][j] = avial;
			}
			else {
				matrix[i][j] = 0;
			}
		}
	}
	return 0;
}

int ClusterOrganizer::ClusterDumpToSPath(const CLUSTERALG::ClusterResult &results, const std::vector<NodeInfo> &m_bindInfo, const std::vector<BlockInfo> &m_blockInfo) {
	std::vector<const SPath*> illegalPath;
	std::vector<const SPath*> legalPath;
	for(int i = 0; i < results.size(); ++i) {
		if (results[i].clusterResult.size() == 0) {
			continue;
		}
		if (m_blockInfo.size() == 0) {
			continue;
		}
		//生成距离矩阵
		std::vector<std::vector<int> > matrix;
		matrix.resize(results[i].clusterResult.size());
		for(int k = 0; k < matrix.size(); ++k) {
			matrix[k].resize(m_blockInfo.size());
		}
		CalViewOpenCloseInfo(results[i], matrix, m_bindInfo, m_blockInfo);

		if(__debug_level__ > 0) {
			cerr << "matrix i:" << i << endl;
			for(int j = 0; j < matrix.size(); ++j) {
				for(int k = 0; k < matrix[j].size(); ++k) {
					cerr << matrix[j][k];
				}
				cerr << endl;
			}
		}

		//聚类和天数对应
		vector<int> tMatch;
		int isCompleteMatch = 0;
		{
			//匈牙利算法输入需将矩阵逆转
			YANGSHUALG::Graph graph(matrix[0].size(), matrix.size());
			//cerr << "in add" << endl;
			for(int j = 0; j < matrix.size(); ++j) {
				for(int k = 0; k < matrix[j].size(); ++k) {
					if(matrix[j][k] == 1 || matrix[j][k] == 2) {
						graph.Add(k, j, 1);
					}
				}
			}
			graph.ReFlush(1);
			YANGSHUALG::GRAPHALG::Hungary hungary(graph);
			hungary.Run();
			hungary.GetResult(tMatch);
			//for(int j = 0; j < tMatch.size(); ++j) {
			//	cerr << j << " - " << tMatch[j] << endl;
			//}

			//cerr << "yangshu Full" << endl;
			hungary.FullMatch();
			hungary.GetResult(tMatch);
			isCompleteMatch = hungary.IsCompleteMatch();
			//for(int j = 0; j < tMatch.size(); ++j) {
			//	cerr << j << " - " << tMatch[j] << endl;
			//}
		}
		/*{
		cerr << "init Graph " << matrix.size() << " - " << matrix[0].size() << endl;
		YANGSHUALG::Graph graph(matrix[0].size(), matrix.size());
		for(int j = 0; j < matrix.size(); ++j) {
			for(int k = 0; k < matrix[j].size(); ++k) {
				if(matrix[j][k] == 1) {
					graph.Add(j, k, 1, 1);
					graph.Add(k, j, 0, -1);
				} else if (matrix[j][k] == 2) {
					graph.Add(j, k, 1, 2);
					graph.Add(k, j, 0, -2);
				}
			}
		}
		graph.ReFlush(1);
		//YANGSHUALG::GRAPHALG::CostFlow costFlow(graph);
		YANGSHUALG::GRAPHALG::Hungary hungary(graph);
		hungary.Run();
		vector<int> tMatch;
		//hungary.GetResult(tMatch);
		//for(int j = 0; j < tMatch.size(); ++j) {
		//	cerr << j << " - " << tMatch[j] << endl;
		//}

		//cerr << "yangshu Full" << endl;
		hungary.FullMatch();
		hungary.GetResult(tMatch);

		//for(int j = 0; j < tMatch.size(); ++j) {
		//	cerr << j << " - " << tMatch[j] << endl;
		//}
		}*/

		SPath *tmpPath = new SPath();
		tmpPath->Init(m_bagPlan);
		for(int j = 0; j < tMatch.size(); ++j) {
			int block = m_blockInfo[j].m_idx;
			int cluster = tMatch[j];
			//int block = m_blockInfo[tMatch[j]].m_idx;
			//int cluster = j;
			//cerr << "block = " << block << "  cluster = " << cluster << endl;
			//cerr << "m_relax = " << m_blockInfo[j].m_relax << endl;
			//cerr << "size = " << results[i].clusterResult.size() << endl;
			if(cluster < 0) {
				continue;
			}
			int allDur = 0;
			//cerr << cluster << " node:" << endl;
			for(auto it = results[i].clusterResult[cluster].begin(); it != results[i].clusterResult[cluster].end(); ++it) {
				//cerr << " " << *it;
				const VarPlace* vPlace = m_bindInfo[*it].m_place;
				int idx = m_bindInfo[*it].m_idx;
				int hot = vPlace ? vPlace->GetHotLevel(m_bagPlan->m_shopIntensity) : 0;
				int allocDur = m_bagPlan->GetAllocDur(vPlace);
				uint8_t vType = m_bagPlan->GetVType(block, idx);
				bit160 bid = m_bagPlan->GetPosBid(idx);
				tmpPath->PushV(block, idx, hot, allocDur, bid, vType);
				tmpPath->SetScore(tmpPath->m_hot);
				allDur += allocDur;
			}
			//cerr << endl;
			//cerr << "allDur = " << allDur << endl;
		}
		tmpPath->CalHashPointSet(m_bagPlan);
		tmpPath->CalHashDayOrder(m_bagPlan);
		//tmpPath->SetHashFrom(m_clusterInfoP->m_pointSetHash);
		if(isCompleteMatch) {
			m_rootList->push_back(tmpPath);
		}
		else {
			illegalPath.push_back(tmpPath);
		}
	}
	//如果没有一个合法的线路才引入不合法线路
	//if(m_rootList->size() == 0) {
	for (int i = 0; i < illegalPath.size(); ++i) {
		m_rootList->push_back(illegalPath[i]);
	}
	//}
	return 0;
}

int MyKmeans::KmeansAdjust() {
	//清空老的聚类
	for(int i = 0; i < m_clusterList.m_cluster.size(); ++i) {
		m_clusterList.m_cluster[i].m_node.clear();
	}

	//把每个点放到进的聚类中
	for(int i = 0; i < m_bindInfo.size(); ++i) {
		int minClusterIndex = -1;
		int minDist = 9999999;
		for(int j = 0; j < GetClusterNum(); ++j) {
			if(m_P2Cluster[i][j] < minDist) {
				minDist = m_P2Cluster[i][j];
				minClusterIndex = j;
			}
		}
		//cerr << "minClusterIndex = " << minClusterIndex << endl;
		m_clusterList.m_cluster[minClusterIndex].m_node.push_back(i);
	}

	//重新计算游玩时间
	for(int i = 0; i < m_clusterList.m_cluster.size(); ++i) {
		int &allDur = m_clusterList.m_cluster[i].m_duration;
		allDur = 0;
		for(int j = 0; j < m_clusterList.m_cluster[i].m_node.size(); ++j) {
			//int dur = m_bagPlan->GetAllocDur(m_bindInfo[i].m_place);
			int dur = m_bindInfo[m_clusterList.m_cluster[i].m_node[j]].m_dur;
			allDur += dur;
		}
	}
	
	{
		std::vector<ClusterNode*> v;
		for(int i = 0; i < m_clusterList.m_cluster.size(); ++i) {
			v.push_back(&m_clusterList.m_cluster[i]);
		}
		//cerr << "dur" << endl;
		//for (int i = 0; i < v.size(); ++i) {
		//	std::cerr << v[i]->m_duration << " ";
		//}
		//cerr << endl;
		std::sort(v.begin(), v.end(), ClusterNode::DurCmpOfPoint);
		//std::sort(v.begin(), v.end(), [](const ClusterNode *lhs, const ClusterNode *rhs) { 	
		//	if(lhs->m_duration == lhs->m_duration) { return lhs->m_idx < lhs->m_idx; }
		//	return lhs->m_duration > lhs->m_duration;
		//});
		//cerr << "block" << endl;
		//for (int i = 0; i < m_blockInfo.size(); ++i) {
		//	std::cerr << m_blockInfo[i].m_relax << " ";
		//}
		//cerr << endl;
		//cerr << "dur" << endl;
		//for (int i = 0; i < v.size(); ++i) {
			//std::cerr << v[i]->m_duration << " ";
		//}
		//cerr << endl;
		for(int i = 0; i < v.size(); ++i) {
			v[i]->m_leftTime = m_blockInfo[i].m_relax - (v[i]->m_duration);
		}
	}

	//更新result
	m_clusterList.m_result.clusterResult.clear();
	for(int i = 0; i < m_clusterList.m_cluster.size(); ++i) {
		std::tr1::unordered_set<int> tmp;
		for(int j = 0; j < m_clusterList.m_cluster[i].m_node.size(); ++j) {
			tmp.insert(m_clusterList.m_cluster[i].m_node[j]);
		}
		m_clusterList.m_result.clusterResult.push_back(tmp);
	}
	return 0;
}

//点到集合之间的距离
int MyKmeans::CalPointToClusterDist(const vector<int> &vCenter) {
	for(int i = 0; i < vCenter.size(); ++i) {
		CalPointToClusterDistOneCluster(i, vCenter[i]);
	}
	return 0;
}

int MyKmeans::CalPointToClusterDistOneCluster(int cluster, int centerIdx) {
	if(cluster < 0 || centerIdx < 0) {
		cerr << "[yangshuError]: centerIdx:" << centerIdx << endl;
		return 0;
	}
	for(int i = 0; i < m_bindInfo.size(); ++i) {
		m_P2Cluster[i][cluster] = CalPointToClusterDistOneNode(i, centerIdx);
	}
	return 0;
}

int MyKmeans::ShowP2ClusterData() const {
	for(int i = 0; i < m_bindInfo.size(); ++i) {
		for(int j = 0; j < GetClusterNum(); ++j) {
			cerr << m_P2Cluster[i][j] << " ";
		}
		cerr << endl;
	}
	return 0;
}

int MyKmeans::CalPointToClusterDistOneNode(const int idxA, const int idxB) const {
	int dist = 0;
	int num = 0;
	for(int i = 0; i < m_bindInfo[idxA].m_bindInfo.size(); ++i) {
		const auto &from = m_bindInfo[idxA].m_bindInfo[i];
		for(int j = 0; j < m_bindInfo[idxB].m_bindInfo.size(); ++j) {
			const auto &to = m_bindInfo[idxB].m_bindInfo[j];
			dist += GetDist(from, to);
			++num;
		}
	}
	//保证被除数大于0
	num = (num>0?num:1);
	return dist/num;
}

int MyKmeans::GreedSwitchAllPoint() {
	std::vector<const ClusterNode*> legalClusterList;
	std::vector<const ClusterNode*> illegalClusterList;
	for(int i = 0; i < m_clusterList.m_cluster.size(); ++i) {
		if(m_clusterList.m_cluster[i].m_leftTime >= 0 && m_clusterList.m_cluster[i].m_node.size() <= BagParam::m_dayViewLimit) {
			legalClusterList.push_back(&(m_clusterList.m_cluster[i]));
		}
		else {
			illegalClusterList.push_back(&(m_clusterList.m_cluster[i]));
		}
	}

	if(illegalClusterList.size() == 0) {
		if(__debug_level__ > 3) {
			cerr << "has GreedSwitchAllPoint" << endl;
		}
		return 0;
	}

	//按照剩余时间从小到大牌，在本列表中所有的剩余时间都为负数
	//std::sort(illegalClusterList.begin(), illegalClusterList.end(), ClusterNode::LeftTimeCmp);
	std::sort(illegalClusterList.begin(), illegalClusterList.end(), ClusterNode::LeftTimeCmpOfPoint);
	//for(int i = 0; i < illegalClusterList.size(); ++i) {
	//	cerr << illegalClusterList[i]->m_leftTime << endl;
	//}
	//exit(0);
	
	for (int i = 0; i < illegalClusterList.size(); i++) {
		const ClusterNode* illegalClusterNode = illegalClusterList[i];
		int minfromCIndex = illegalClusterNode->m_idx;
		bool canSwitch = true;
		while(canSwitch && (illegalClusterNode->m_leftTime < 0 || illegalClusterNode->m_node.size() > BagParam::m_dayViewLimit)) {
			int minDist = 99999999;
			int minPIndex = -1;
			int minCToIndex = -1;
			for(int pIndex = 0; pIndex < illegalClusterNode->m_node.size(); ++pIndex) {
				if(illegalClusterNode->m_node.size() <= 1) {
					break;
				}
				int localPos = illegalClusterNode->m_node[pIndex];
				//原代码 if(m_clusterInfoP->m_vPosRestType[localPos] == LY_PLACE_TYPE_RESTAURANT) continue; //不交换餐厅
				if(m_bindInfo[localPos].m_place->_t & LY_PLACE_TYPE_RESTAURANT) {
					continue;
				}
				//原代码 int dur = m_clusterInfoP->m_vPosLegalDurList[localPos];
				int dur = m_bindInfo[localPos].m_dur;
				for(int j = 0; j < legalClusterList.size(); ++j) {
					const ClusterNode* legalClusterNode = legalClusterList[j];
					if(legalClusterNode->m_leftTime < dur ||legalClusterNode->m_node.size()>=BagParam::m_dayViewLimit) {
						continue;
					}
					int legalClusterIndex = legalClusterNode->m_idx;
					int distP2C = m_P2Cluster[localPos][legalClusterIndex]; 
					if (distP2C < minDist) {
						minDist = distP2C;
						minPIndex = localPos;
						minCToIndex = legalClusterIndex;
					}
				}
			}
			if(minPIndex >= 0) {
				int ret = MovePointFromClusterAToClusterB(minPIndex, minfromCIndex, minCToIndex);
			}
			else {
				canSwitch = false;
			}
		}//while
	}
	return 0;
}

int MyKmeans::GreedSwitchAllRest() {
	std::vector<const ClusterNode*> legalClusterList;
	std::vector<const ClusterNode*> illegalClusterList;
	for(int i = 0; i < m_clusterList.m_cluster.size(); ++i) {
		if(m_clusterList.m_cluster[i].m_leftTime >= 0 && m_clusterList.m_cluster[i].m_node.size() <= BagParam::m_dayViewLimit) {
			legalClusterList.push_back(&(m_clusterList.m_cluster[i]));
		}
		else {
			illegalClusterList.push_back(&(m_clusterList.m_cluster[i]));
		}
	}

	if(illegalClusterList.size() == 0) {
		if(__debug_level__ > 3) {
			cerr << "has GreedSwitchAllRest" << endl;
		}
		return 0;
	}

	//按照剩余时间从小到大牌，在本列表中所有的剩余时间都为负数
	std::sort(illegalClusterList.begin(), illegalClusterList.end(), ClusterNode::RestLeftCmpOfPoint);

	for(int i = 0; i < illegalClusterList.size(); ++i) {
		const ClusterNode* illegalClusterNode = illegalClusterList[i];
		int minfromCIndex = illegalClusterNode->m_idx;
		bool canSwitch = true;
		while(canSwitch && illegalClusterNode->m_restLeft < 0) {
			int minDist = 99999999;
			int minPIndex = -1;
			int minCToIndex = -1;
			for(int pIndex = 0; pIndex < illegalClusterNode->m_node.size(); pIndex++) {
				if(illegalClusterNode->m_node.size() <= 1) {
					break;
				}
				int localPos = illegalClusterNode->m_node[pIndex];
				//原代码 if(m_clusterInfoP->m_vPosRestType[localPos] != LY_PLACE_TYPE_RESTAURANT) continue;
				if(!(m_bindInfo[localPos].m_place->_t & LY_PLACE_TYPE_RESTAURANT)) {
					continue;
				}
				for(int j = 0; j <legalClusterList.size(); ++j) {
					const ClusterNode* legalClusterNode = legalClusterList[j];
					if(legalClusterNode->m_restLeft <= 0) {
						continue; //没有可以往里塞的餐厅了
					}
					int legalClusterIndex = legalClusterNode->m_idx;
					int distP2C = m_P2Cluster[localPos][legalClusterIndex]; 
					if(distP2C < minDist) {
						minDist = distP2C;
						minPIndex = localPos;
						minCToIndex = legalClusterIndex;
					}
				}
			}
			if (minPIndex >= 0) {
				int ret = MovePointFromClusterAToClusterB(minPIndex, minfromCIndex, minCToIndex);
			} else {
				canSwitch = false;
			}
		}//while
	}
	return 0;
}

int MyKmeans::Run() {
	if(GetClusterNum() > m_initCluster->clusterResult.size()) {
		return 0;
	}
	//cerr << "YANGSHU m_bindInfo.size() = " << m_bindInfo.size() << endl;
	std::vector<int> vCenter;
	GetAllCenters(*m_initCluster, vCenter);
	if(__debug_level__ > 5) {
		ShowBaseMatrix();
	}
	vector<vector<int> > preCluster(m_initCluster->clusterResult.size());
	for(int i = 0; i < m_initCluster->clusterResult.size(); ++i) {
		std::vector<int> tmp;
		for(auto it = m_initCluster->clusterResult[i].begin(); it != m_initCluster->clusterResult[i].end(); ++it) {
			tmp.push_back(*it);
		}
		sort(tmp.begin(), tmp.end());
		preCluster.push_back(tmp);
	}
	sort(preCluster.begin(), preCluster.end());

	for (int i = 0; i < 10; ++i) {
		if(__debug_level__ > 8) {
			cerr << "Center:" << endl;
			for(int j = 0; j < vCenter.size(); ++j) {
				cerr << vCenter[j] << " ";
			}
			cerr << endl;
		}
		CalPointToClusterDist(vCenter);
		if(__debug_level__ > 9) {
			ShowP2ClusterData();
		}
		if(__debug_level__ > 5) {
			m_clusterList.ShowClusterInfo();
		}
		
		//RecordClusters
		KmeansAdjust();
		CalPointToClusterDist(vCenter);
		if(__debug_level__ > 5) {
			m_clusterList.ShowClusterInfo();
		}

		if(__debug_level__ > 5) {
			cerr << "first adjust" << endl;
		}
		//第一次调整
		GreedSwitchAllPoint();

		if(__debug_level__ > 5) {
			cerr << "second adjust" << endl;
		}
		//第二次调整
		GreedSwitchAllRest();
		
		{
			//两次聚类结果一致时结束
			vector<vector<int> > newCluster(m_clusterList.m_result.clusterResult.size());
			for(int i = 0; i < m_clusterList.m_result.clusterResult.size(); ++i) {
				std::vector<int> tmp;
				for(auto it = m_clusterList.m_result.clusterResult[i].begin(); it != m_clusterList.m_result.clusterResult[i].end(); ++it) {
					tmp.push_back(*it);
				}
				sort(tmp.begin(), tmp.end());
				newCluster.push_back(tmp);
			}
			sort(newCluster.begin(), newCluster.end());
			if(newCluster == preCluster) {
				break;
			}
			preCluster = newCluster;
		}
		vCenter.clear();
		GetAllCenters(m_clusterList.m_result, vCenter);
	}
	SetResult(m_clusterList.m_result);
	return 0;
}

int MyKmeans::ClusterAddPoint(int clusterIdx, int nodeIdx) {
	auto &cluster = m_clusterList.m_cluster[clusterIdx];
	cluster.m_node.push_back(nodeIdx);
	m_clusterList.m_result.clusterResult[clusterIdx].insert(nodeIdx);
	cluster.m_duration += m_bindInfo[nodeIdx].m_dur;
	cluster.m_leftTime -= m_bindInfo[nodeIdx].m_dur;
	return 0;
}
int MyKmeans::ClusterSubPoint(int clusterIdx, int nodeIdx) {
	auto &cluster = m_clusterList.m_cluster[clusterIdx];
	auto it = find(cluster.m_node.begin(), cluster.m_node.end(), nodeIdx);
	cluster.m_node.erase(it);
	m_clusterList.m_result.clusterResult[clusterIdx].erase(nodeIdx);
	cluster.m_duration -= m_bindInfo[nodeIdx].m_dur;
	cluster.m_leftTime += m_bindInfo[nodeIdx].m_dur;
	return 0;
}

int MyKmeans::ClusterAddRest(int clusterIdx, int nodeIdx) {
	auto &cluster = m_clusterList.m_cluster[clusterIdx];
	cluster.m_node.push_back(nodeIdx);
	m_clusterList.m_result.clusterResult[clusterIdx].insert(nodeIdx);
	cluster.m_restNum += 1;
	cluster.m_restLeft -= 1;
	return 0;
}
int MyKmeans::ClusterSubRest(int clusterIdx, int nodeIdx) {
	auto &cluster = m_clusterList.m_cluster[clusterIdx];
	auto it = find(cluster.m_node.begin(), cluster.m_node.end(), nodeIdx);
	cluster.m_node.erase(it);
	m_clusterList.m_result.clusterResult[clusterIdx].erase(nodeIdx);
	cluster.m_restNum -= 1;
	cluster.m_restLeft += 1;
	return 0;
}

int MyKmeans::MovePointFromClusterAToClusterB(int nodeIdx, int clusterAIdx, int clusterBIdx) {
	const auto &node = m_bindInfo[nodeIdx];
	//餐厅和景点分开处理
	if(__debug_level__ > 5) {
		cerr << "move before" << endl;
		m_clusterList.ShowClusterInfo();
	}
	if(node.m_place->_t & LY_PLACE_TYPE_RESTAURANT) {
		ClusterSubRest(clusterAIdx, nodeIdx);
		ClusterAddRest(clusterBIdx, nodeIdx);
	}
	else {
		//cerr << "in mv" << endl;
		//m_clusterList.ShowClusterInfo();
		//cerr << "in Sub" << endl;
		ClusterSubPoint(clusterAIdx, nodeIdx);
		//m_clusterList.ShowClusterInfo();
		//cerr << "in Add" << endl;
		ClusterAddPoint(clusterBIdx, nodeIdx);
		//m_clusterList.ShowClusterInfo();
		//exit(0);
	}
	if(__debug_level__ > 5) {
		cerr << "move ret" << endl;
		m_clusterList.ShowClusterInfo();
	}
	//更新数据
	int minIdx = GetCenter(m_clusterList.m_result.clusterResult[clusterAIdx]);
	//m_clusterList.ShowClusterInfo();
	//cerr << "idx = " << clusterAIdx << " minIdx = " << minIdx << endl;
	CalPointToClusterDistOneCluster(clusterAIdx, minIdx);
	minIdx = GetCenter(m_clusterList.m_result.clusterResult[clusterBIdx]);
	CalPointToClusterDistOneCluster(clusterBIdx, minIdx);
	return 0;
}

int ClusterList::ShowClusterInfo() const {
	cerr << "ClusterList:" << endl;
	for(int i = 0; i < m_cluster.size(); ++i) {
		cerr << "i:" << i << " use: " << m_cluster[i].m_duration <<
		" left: " << m_cluster[i].m_leftTime << " rest: " << m_cluster[i].m_restNum <<
		" restLeft: " << m_cluster[i].m_restLeft << endl;
		cerr << "info:";
		for(int j = 0; j < m_cluster[i].m_node.size(); ++j) {
			cerr << "  " << m_cluster[i].m_node[j];
		}
		cerr << endl;
		for(auto it = m_result.clusterResult[i].begin(); it != m_result.clusterResult[i].end(); ++it) {
			cerr << *it << " ";
		}
		cerr << endl;
	}
	return 0;
}

uint32_t ClusterOrganizer::CalHashPointSet(const std::vector<NodeInfo> &nodeInfo) {
	bit160 vPathBid;
	for (int i = 0; i < nodeInfo.size(); ++i) {
		uint8_t vPos = nodeInfo[i].m_idx;
		vPathBid |= m_bagPlan->GetPosBid(vPos);
	}
	return YANGSHUALG::Hash(vPathBid);
}


}

