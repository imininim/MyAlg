#include <iostream>
#include <str/KMP.h>
using namespace std;
using namespace YANGSHUALG;
int main() {
	string strS, strT;
	cin >> strS >> strT;
	std::vector<char> s, t;
	for (int i = 0; i < strS.size(); ++i) {
		s.push_back(strS[i]);
	}
	for (int i = 0; i < strT.size(); ++i) {
		t.push_back(strT[i]);
	}
	KMPMatch<char> kmp(s, t);
	cerr << kmp.Run() << endl;
	kmp.Show();
}
