#include"ChordBase.h"

class Chord : public Base<string,string>{
public:
	Chord();
	//返回节点中存储的<key,value>值
	list<store<string,string>> Getstore(int ID) {	return Kstore[ID];	}
	//计算hash函数
	int BKDHashs(string key);
	//寻找key对应的value
	virtual bool LookUp(string key) override;
	//更新key对应的value
	virtual bool Update(string key, string new_value) override;
	//插入key和对应的value
	virtual void Insert(string key, string value, int t = 1) override;
};

/*构造函数*/
inline Chord::Chord() :Base("127.0.0.1",2048) {
	fstream os;
	string line;
	os.open("./kvalue.txt");
	while (getline(os, line)) {
		int t = line.find_first_of(' ');
		string key = line.substr(0, t);
		string value = line.substr(t + 1);
		Insert(key, value, 0);
	}
}

inline int Chord::BKDHashs(string key)
{
	std::string ss = key;
	int seed = 131;
	int hash = 0;
	for (char i : ss) {
		hash = hash * seed + i;
	}
	return (hash & 0x7FFFFFFF);
}
/*查找key为key的<key,value>*/
inline bool Chord::LookUp(string key)
{
	int sizee = getsize();
	int K = BKDHashs(key) % sizee;
	int successori = findsuccessor(K);
	for (auto i = Kstore[successori].begin(); i != Kstore[successori].end(); i++) {
		if ((*i).K == K) {
			return true;
		}
	}
	findlist.clear();
	return false;
}

/*更新key为key的<key,value>为<key,new_value>*/
inline bool Chord::Update(string key, string new_value)
{
	int sizee = getsize();
	int K = BKDHashs(key) % sizee;
	int keysuccessorid = findsuccessor(K);
	for (auto &i : Kstore[keysuccessorid]) {
		if (i.K == K) {
			i.value = new_value;
			return true;
		}
	}
	findlist.clear();
	return false;
}

/*将<key,value>插入到网络中*/
inline void Chord::Insert(string key, string value, int t)
{
	int sizee = getsize();
	int K = BKDHashs(key) % sizee;
	store<string, string> s = { K,key,value };
	int keysuccessorid = findsuccessor(K, 0);
	Kstore[keysuccessorid].push_back(s);
}

