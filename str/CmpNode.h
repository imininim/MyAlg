#ifndef __STRING_MATCH_H__
#define __STRING_MATCH_H__

#include <vector>
#include <iostream>

typedef size_t CmpIdx;

//用于比较的节点, 可用于 kmp
class CmpNode {
public:
	size_t value;	//z用于比较的值
	bool operator ==(const CmpNode &p) {
		return value == p.value;
	}
};

//有儿子数组的比较节点, Tired用
class CmpNodeWithChild: public CmpNode {
public:
	std::vector<CmpNodeWithChild*> child;
public:
	CmpNodeWithChild(size_t childSize) {
		child.resize(childSize, NULL);
	}
	//优化用,看子节点是否对值是否有特殊索引
	CmpIdx GetIdx() const {
		return 0;
	}
};

//有失败节点的比较节点, ac自动机使用
class CmpNodeWithFail: public CmpNodeWithChild {
public:
	const CmpNodeWithFail *fail;
};

//字典树
class Tried {
private:
	CmpNodeWithChild *m_root;
	const CmpIdx m_MaxChild;
public:
	Tried(size_t maxChild = 26):m_root(), m_MaxChild(maxChild) {}
	bool Insert(std::vector<size_t> &input) {
		for (size_t i = 0; i < input.size(); ++i) {
			std::cerr << input[i];
		}
		std::cerr << std::endl;
		if (m_root == NULL) {
			m_root = new CmpNodeWithChild(m_MaxChild);
		}
		CmpNodeWithChild *next = m_root;
		for (size_t i = 0; i < input.size(); ++i) {
			const size_t &value = input[i];
			std::cerr << value << " i = " << i << std::endl;
			if (next->child[value] == NULL) {
				CmpNodeWithChild *tmp = new CmpNodeWithChild(m_MaxChild);
				tmp->value = value;
				next->child[value] = tmp;
				std::cerr << "size = " << next->child.size() << std::endl;
			}
			next = next->child[value];
		}
		return true;
	}
	const CmpNodeWithChild* GetNode(std::vector<size_t> &input) const {
		if (m_root == NULL) {
			return NULL;
		}
		const CmpNodeWithChild *next = m_root;
		for (size_t i = 0; i < input.size(); ++i) {
			const size_t &value = input[i];
			if (next->child[value] == NULL) {
				return NULL;
			}
			next = next->child[value];
		}
		return next;
	}
	//返回根节点
	const CmpNodeWithChild* GetRoot() const {
		return m_root;
	}
	bool Clear() {
		Clear(m_root);
		m_root = NULL;
		return true;
	}
private:
	bool Clear(const CmpNodeWithChild *root) {
		if (root != NULL) {
			for (size_t i = 0; i < root->child.size(); ++i) {
				Clear(root->child[i]);
			}
			delete root;
		}
		return true;
	}
};


//ac自动机
class AC_Automaton {
public:

};

#endif



/*#ifndef __STRING_MATCH_H__
#define __STRING_MATCH_H__

#include <vector>

typedef size_t CmpIdx;

//用于比较的节点, 可用于 kmp
class CmpNode {
public:
	virtual bool operator ==(const CmpNode &p)=0;
};

//有儿子数组的比较节点, Tired用
class CmpNodeWithChild: public CmpNode {
public:
	std::vector<const CmpNodeWithChild*> child;
public:
	//优化用,看子节点是否对值是否有特殊索引
	virtual CmpIdx GetIdx() const {
		return 0;
	}
};

//有失败节点的比较节点, ac自动机使用
class CmpNodeWithFail: public CmpNodeWithChild {
public:
	const CmpNodeWithFail *fail;
};

//字典树
class Tried {
private:
	const CmpNodeWithChild *m_root;
public:
	Tried():m_root(NULL) {}
	bool Insert(std::vector<const CmpNode*> &input);
	bool Insert(std::vector<const CmpNode> &input);
	const CmpNode* GetNode(std::vector<const CmpNode*> &input) const;
	const CmpNode* GetNode(std::vector<const CmpNode> &input) const;
	//返回根节点
	const CmpNode* GetRoot() const {
		return m_root;
	}
};

bool Tried::Insert(std::vector<const CmpNode*> &input) {

};

bool Tried::Insert(std::vector<const CmpNode> &input) {

};

const CmpNode* Tried::GetNode(std::vector<const CmpNode*> &input) const {

}
const CmpNode* Tried::GetNode(std::vector<const CmpNode> &input) const {

}

//ac自动机
class AC_Automaton {
public:

};

#endif
*/
