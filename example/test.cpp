#include <iostream>
#include <str/CmpNode.h>

using namespace std;

/*class Test: public CmpNode {
public:
	int m_member;
	virtual bool operator ==(const CmpNode &p) {
		return m_member == p.m_member;
	}
	Test(const int &member):m_member(member) {}
};

int main() {
	Test A(1), B(1), C(3);
	cerr << "A == B:" << A == B << endl;
	cerr << "A == C:" << A == C << endl;
	return 0;
}*/

int main() {
	Tried tried(10);

	for (int i = 0; i < 100; ++i) {
		vector<size_t> tmp;
		for (int j = 0; j < 10; ++j) {
			tmp.push_back((i*j)%10);
		}
		tried.Insert(tmp);
	}
	char x[] = "0369258147";
	char y[] = "0482604826";
	char z[] = "0505050505";
	vector<size_t> tmp;
	for (int i = 0; i < 10; i ++) {
		tmp.push_back(x[i]-'0');
	}
	auto ptr = tried.GetNode(tmp);
	if (ptr == NULL) {
		std::cerr << "error" << std::endl;
	} else {
		std::cerr << "ok" << std::endl;
	}

	return 0;
}

