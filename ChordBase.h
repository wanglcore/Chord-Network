#pragma once
#include<iostream>
#include<string>
#include<list>
#include<fstream>
#include<cmath>
#include<vector>
using namespace std;

struct FingerNode{//路由表中的信息
	string IP;
	int ID;
	bool operator==(FingerNode node) {
		return (node.ID == this->ID) && (node.IP == this->IP);
	}
};

struct ChordNode {//chord中的节点
	int ID;
	string IP;
	int presuccessor;
	int successor;
};

template<class Key,class Value>
struct store {//节点中存储的节点
	int K;
	Key key;
	Value value;
	store(int _K, Key _key, Value _value) :K(_K), key(_key), value(_value) {	}
	store(store*s) {
		K = s->K;
		key = s->key;
		value = s->value;
	}
};


template<class Key,class Value>
class Base {
public:
	Base(string IP,int _size);
	void Join(string IP, int t = 1);//节点加入
	bool Leave(string IP);//节点离开
	int findsuccessor(int K, int t = 1);//寻找K的后继节点
	int BKDHash(string s);//计算哈希函数
	virtual bool LookUp(Key key) = 0;
	//更新key对应的value
	virtual bool Update(Key key, Value new_value) = 0;
	//插入key和对应的value
	virtual void Insert(Key key, Value value, int t = 1) = 0;
	int getsize() { return size; };
	int getlocalid() { return localid; };
	list<FingerNode>Gettable(int ID) {	return FingerTable[ID]; };//返回指定节点的路由表
	list<FingerNode>getID() {	return IDlist; }//返回网络中的所有节点
	list<FingerNode>Findlist() {	return findlist; }//返回在一次查找后继节点的过程中访问的节点
	list<FingerNode>GetJoinlist() {		return Joinlist;	}//返回在一个新节点加入时,访问的节点
protected:
	vector<list<store<Key, Value>>>Kstore;//网络中存储的信息
	list<FingerNode>Joinlist;//添加节点时路由的节点
	list<FingerNode>IDlist;//网络中的节点
	list<FingerNode>findlist;//查找后继时路由的节点
private:
	int nodenumber = 0;//节点个数
	int localid;
	int size;//网络大小
	int routenumber;//路由表的大小
	void Init();//初始化网络中的节点
	void Joinstabilize(int ID, int t = 1);//用于更新节点之间的连接和路由表
	void Leavestabilize(int K);//在一个节点离开之后,更新网络的节点信息和路由表
	void getroutenumber();//有网络大小计算路由表的大小
	vector<ChordNode>v;//网络中的节点
	vector<list<FingerNode>>FingerTable;//节点的路由表
};

/*构造函数,创建一个_size大小的网络*/
template<class Key, class Value>
inline Base<Key, Value>::Base(string IP,int _size)
{
	size = _size;
	getroutenumber();
	localid = BKDHash(IP) % size;
	ChordNode localnode = { localid,IP,localid,localid };
	FingerNode fingernode = { IP,localid };
	IDlist.push_back(fingernode);
	v.resize(size);
	v[localid] = localnode;
	nodenumber = 1;
	Kstore.resize(size);
	FingerTable.resize(size);
	FingerTable[localid].resize(routenumber);
	for (auto i = FingerTable[localid].begin(); i != FingerTable[localid].end(); i++) {
		(*i).ID = localid;
		(*i).IP = IP;
	}
	Init();//初始化网络
}

/*将参数IP所代表的节点加入到网络中
功能:初始化新插入节点的路由表,重新分配<key,value>的存储位置*/
template<class Key, class Value>
inline void Base<Key, Value>::Join(string IP, int t)
{
	FingerNode fingernode;
	int ID = BKDHash(IP) % size;
	for (auto i : IDlist) {
		if (i.ID == ID) {
			return;
		}
	}
	IDlist.push_back({ IP,ID });
	ChordNode node = { ID,IP,0,0 };
	int IPsuccessor = findsuccessor(ID, t);
	Joinlist.push_back({ IP,ID });
	v[ID] = node;
	nodenumber++;
	v[ID].successor = IPsuccessor;
	v[IPsuccessor].presuccessor = ID;
	FingerTable[ID].resize(routenumber);
	FingerTable[ID] = FingerTable[IPsuccessor];//初始化新插入节点的路由表,时新插入的节点的路由表和他后记的路由表相等
	for (auto i = Kstore[IPsuccessor].begin(); i != Kstore[IPsuccessor].end(); i++) {
		if (IPsuccessor > ID) {
			if ((*i).K > ID && (*i).K <= IPsuccessor) {
				continue;
			}
			else {
				Kstore[ID].push_back((*i));
			}
		}
		else if (ID>IPsuccessor) {
			if ((*i).K <= ID && (*i).K > IPsuccessor) {
				Kstore[ID].push_back((*i));
			}
		}
	}
	Joinstabilize(ID, t);//对网络的中的节点之间的连接和路由表进行更新
}

/*IP为参数的节点离开网络
功能:清空离开节点的路由表,重新分配<key,value>的存储位置*/
template<class Key, class Value>
inline bool Base<Key, Value>::Leave(string IP)
{
	int ID = BKDHash(IP) % size;

	bool nofind = true;
	for (auto i = IDlist.begin(); i != IDlist.end(); i++) {
		if ((*i).ID == ID) {//在节点的链表中查找IP的对应ID;
			nofind = false;
			IDlist.remove({ IP,ID });
			int presuccessorid = v[ID].presuccessor;
			v[presuccessorid].successor = v[ID].successor;
			v[v[ID].successor].presuccessor = presuccessorid;
			for (auto i = Kstore[ID].begin(); i != Kstore[ID].end(); i++) {//移动<key,value>的位置
				Kstore[v[ID].successor].push_back((*i));
			}
			Kstore[ID].clear();
			FingerTable[ID].clear();
			nodenumber--;
			Leavestabilize(ID);//对网络中的其他节点的路由表进行更新
			return true;
		}
	}
	if (nofind) {
		findlist.clear();
	}
	return false;
}

/*计算哈希函数*/
template<class Key, class Value>
inline int Base<Key, Value>::BKDHash(string s)
{
	std::string ss = s;
	int seed = 131;
	int hash = 0;
	for (char i : ss) {
		hash = hash * seed + i;
	}
	return (hash & 0x7FFFFFFF);
}

/*查找给定Key的后继*/
template<class Key, class Value>
inline int Base<Key, Value>::findsuccessor(int K, int t)
{
	if (t == 1)
		Joinlist.clear();
	findlist.clear();
	if (v[localid].presuccessor == localid && v[localid].successor == localid) {//一个节点
		if (t == 1)
			Joinlist.push_back({ v[localid].IP ,localid });
		findlist.push_back({ v[localid].IP,localid });
		return localid;
	}
	else if (v[localid].presuccessor == v[localid].successor) {//只有两个节点
		if (v[localid].successor > localid) {//后继节点的ID大于当前节点的ID
			if (K > localid&&K <= v[localid].successor) {
				if (t == 1) {
					Joinlist.push_back({ v[localid].IP,localid });
					Joinlist.push_back({ v[v[localid].successor].IP,v[localid].successor });
				}
				findlist.push_back({ v[localid].IP,localid });
				findlist.push_back({ v[v[localid].successor].IP,v[localid].successor });
				return v[localid].successor;
			}
			else {
				if (t == 1)
					Joinlist.push_back({ v[localid].IP,localid });
				findlist.push_back({ v[localid].IP,localid });
				return localid;
			}
		}
		else if (v[localid].successor<localid) {//后继节点的ID大于当前节点的ID
			if (K > v[localid].successor&&K <= localid) {
				if (t == 1)
					Joinlist.push_back({ v[localid].IP,localid });
				findlist.push_back({ v[localid].IP,localid });
				return localid;
			}
			else {
				if (t == 1) {
					Joinlist.push_back({ v[localid].IP,localid });
					Joinlist.push_back({ v[v[localid].successor].IP,v[localid].successor });
				}
				findlist.push_back({ v[localid].IP,localid });
				findlist.push_back({ v[v[localid].successor].IP,v[localid].successor });
				return v[localid].successor;
			}
		}
	}
	else {
		int nextid = localid;
		int successorid = v[nextid].successor;
		findlist.push_back({ v[localid].IP,localid });
		if (t == 1)
			Joinlist.push_back({ v[localid].IP,localid });
		while (true) {
			if (successorid > nextid) {
				if (K <= successorid && K > nextid) {
					findlist.push_back({ v[successorid].IP,successorid });
					return successorid;
				}
			}
			else if (successorid<nextid) {
				if (K <= successorid) {

					findlist.push_back({ v[successorid].IP,successorid });
					return successorid;
				}
				else if (K > nextid) {

					findlist.push_back({ v[successorid].IP,successorid });
					return successorid;
				}
			}
			int temp = -1, max = -1;
			for (auto i = FingerTable[nextid].rbegin(); i != FingerTable[nextid].rend(); i++) {
				if ((*i).ID >= K) {
					if (max < (*i).ID) {
						max = (*i).ID;
					}
				}
				else {
					if (temp < (*i).ID) {
						temp = (*i).ID;
					}
				}
			}
			if (temp != -1) {
				nextid = temp;
			}
			else {
				nextid = max;
			}
			if (t == 1)
				Joinlist.push_back({ v[nextid].IP,nextid });
			findlist.push_back({ v[nextid].IP,nextid });
			successorid = v[nextid].successor;
		}
	}
}

/*初始化网络*/
template<class Key, class Value>
inline void Base<Key, Value>::Init()
{
	fstream os("./ip.txt");
	string line;
	while (getline(os, line)) {
		Join(line, 0);
	}
	os.close();
}


/*在新增加一个节点之后,对网络中的其他路由表进行更新*/
template<class Key, class Value>
inline void Base<Key, Value>::Joinstabilize(int ID, int t)
{
	if (v[v[localid].successor].presuccessor != localid) {//两个节点时的情况
		v[localid].successor = v[v[localid].successor].presuccessor;
		v[v[localid].successor].presuccessor = localid;
	}
	else {
		int id = localid, n = 0;//将新加入的节点完全加入到网络中
		while (n++ < nodenumber) {
			if (v[v[id].successor].presuccessor != id) {
				v[id].successor = v[v[id].successor].presuccessor;
				v[v[id].successor].presuccessor = id;
				break;
			}
			else {
				id = v[id].successor;
			}
		}
	}
	//更新路由表;
	int tableid = localid, m = 0;
	while (m++ < nodenumber) {
		if (tableid == ID)
		{
			tableid = v[tableid].successor;
			continue;
		}
		int index = 0;
		for (auto i = FingerTable[tableid].begin(); i != FingerTable[tableid].end(); i++) {
			int nextindex = (tableid + (int)pow(2, index)) % size;
			index++;
			if (v[(*i).ID].presuccessor == ID) {
				if ((*i).ID > ID) {
					if (nextindex <= (*i).ID&&nextindex <= ID) {
						(*i).ID = ID;
						(*i).IP = v[ID].IP;
					}
					else if (nextindex>(*i).ID) {
						(*i).ID = ID;
						(*i).IP = v[ID].IP;
					}
				}
				else if ((*i).ID < ID) {
					if (nextindex >(*i).ID&&nextindex <= ID) {
						(*i).ID = ID;
						(*i).IP = v[ID].IP;
					}
				}
			}
		}
		tableid = v[tableid].successor;
	}
	int index = 0;//更新新加入节点的路由表
	for (auto i = FingerTable[ID].begin(); i != FingerTable[ID].end(); i++) {
		int nextindex = (ID + (int)pow(2, index)) % size;
		index++;
		int nextid = findsuccessor(nextindex, 0);
		(*i).ID = nextid;
		(*i).IP = v[nextid].IP;
	}
}


/*当一个节点离开时,更新网络中其他节点的路由表*/
template<class Key, class Value>
inline void Base<Key, Value>::Leavestabilize(int K)
{
	if (v[localid].presuccessor == localid && v[localid].successor == localid) {
		return;
	}
	int id = localid, m = 0;
	while (m++ < nodenumber) {
		for (auto i = FingerTable[id].begin(); i != FingerTable[id].end(); i++) {
			if ((*i).ID == K) {
				(*i).ID = v[K].successor;
				(*i).IP = v[v[K].successor].IP;
			}
		}
		id = v[id].successor;
	}
}

/*计算路由表的大小*/
template<class Key, class Value>
inline void Base<Key, Value>::getroutenumber()
{
	int temp = size;
	int number = 0;
	while (temp != 1) {
		temp /= 2;
		number++;
	}
	routenumber = number;
}

