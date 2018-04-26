#pragma once
#include<iostream>
#include<string>
#include<list>
#include<fstream>
#include<cmath>
#include<vector>
using namespace std;

struct FingerNode{//·�ɱ��е���Ϣ
	string IP;
	int ID;
	bool operator==(FingerNode node) {
		return (node.ID == this->ID) && (node.IP == this->IP);
	}
};

struct ChordNode {//chord�еĽڵ�
	int ID;
	string IP;
	int presuccessor;
	int successor;
};

template<class Key,class Value>
struct store {//�ڵ��д洢�Ľڵ�
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
	void Join(string IP, int t = 1);//�ڵ����
	bool Leave(string IP);//�ڵ��뿪
	int findsuccessor(int K, int t = 1);//Ѱ��K�ĺ�̽ڵ�
	int BKDHash(string s);//�����ϣ����
	virtual bool LookUp(Key key) = 0;
	//����key��Ӧ��value
	virtual bool Update(Key key, Value new_value) = 0;
	//����key�Ͷ�Ӧ��value
	virtual void Insert(Key key, Value value, int t = 1) = 0;
	int getsize() { return size; };
	int getlocalid() { return localid; };
	list<FingerNode>Gettable(int ID) {	return FingerTable[ID]; };//����ָ���ڵ��·�ɱ�
	list<FingerNode>getID() {	return IDlist; }//���������е����нڵ�
	list<FingerNode>Findlist() {	return findlist; }//������һ�β��Һ�̽ڵ�Ĺ����з��ʵĽڵ�
	list<FingerNode>GetJoinlist() {		return Joinlist;	}//������һ���½ڵ����ʱ,���ʵĽڵ�
protected:
	vector<list<store<Key, Value>>>Kstore;//�����д洢����Ϣ
	list<FingerNode>Joinlist;//��ӽڵ�ʱ·�ɵĽڵ�
	list<FingerNode>IDlist;//�����еĽڵ�
	list<FingerNode>findlist;//���Һ��ʱ·�ɵĽڵ�
private:
	int nodenumber = 0;//�ڵ����
	int localid;
	int size;//�����С
	int routenumber;//·�ɱ�Ĵ�С
	void Init();//��ʼ�������еĽڵ�
	void Joinstabilize(int ID, int t = 1);//���ڸ��½ڵ�֮������Ӻ�·�ɱ�
	void Leavestabilize(int K);//��һ���ڵ��뿪֮��,��������Ľڵ���Ϣ��·�ɱ�
	void getroutenumber();//�������С����·�ɱ�Ĵ�С
	vector<ChordNode>v;//�����еĽڵ�
	vector<list<FingerNode>>FingerTable;//�ڵ��·�ɱ�
};

/*���캯��,����һ��_size��С������*/
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
	Init();//��ʼ������
}

/*������IP������Ľڵ���뵽������
����:��ʼ���²���ڵ��·�ɱ�,���·���<key,value>�Ĵ洢λ��*/
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
	FingerTable[ID] = FingerTable[IPsuccessor];//��ʼ���²���ڵ��·�ɱ�,ʱ�²���Ľڵ��·�ɱ������ǵ�·�ɱ����
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
	Joinstabilize(ID, t);//��������еĽڵ�֮������Ӻ�·�ɱ���и���
}

/*IPΪ�����Ľڵ��뿪����
����:����뿪�ڵ��·�ɱ�,���·���<key,value>�Ĵ洢λ��*/
template<class Key, class Value>
inline bool Base<Key, Value>::Leave(string IP)
{
	int ID = BKDHash(IP) % size;

	bool nofind = true;
	for (auto i = IDlist.begin(); i != IDlist.end(); i++) {
		if ((*i).ID == ID) {//�ڽڵ�������в���IP�Ķ�ӦID;
			nofind = false;
			IDlist.remove({ IP,ID });
			int presuccessorid = v[ID].presuccessor;
			v[presuccessorid].successor = v[ID].successor;
			v[v[ID].successor].presuccessor = presuccessorid;
			for (auto i = Kstore[ID].begin(); i != Kstore[ID].end(); i++) {//�ƶ�<key,value>��λ��
				Kstore[v[ID].successor].push_back((*i));
			}
			Kstore[ID].clear();
			FingerTable[ID].clear();
			nodenumber--;
			Leavestabilize(ID);//�������е������ڵ��·�ɱ���и���
			return true;
		}
	}
	if (nofind) {
		findlist.clear();
	}
	return false;
}

/*�����ϣ����*/
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

/*���Ҹ���Key�ĺ��*/
template<class Key, class Value>
inline int Base<Key, Value>::findsuccessor(int K, int t)
{
	if (t == 1)
		Joinlist.clear();
	findlist.clear();
	if (v[localid].presuccessor == localid && v[localid].successor == localid) {//һ���ڵ�
		if (t == 1)
			Joinlist.push_back({ v[localid].IP ,localid });
		findlist.push_back({ v[localid].IP,localid });
		return localid;
	}
	else if (v[localid].presuccessor == v[localid].successor) {//ֻ�������ڵ�
		if (v[localid].successor > localid) {//��̽ڵ��ID���ڵ�ǰ�ڵ��ID
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
		else if (v[localid].successor<localid) {//��̽ڵ��ID���ڵ�ǰ�ڵ��ID
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

/*��ʼ������*/
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


/*��������һ���ڵ�֮��,�������е�����·�ɱ���и���*/
template<class Key, class Value>
inline void Base<Key, Value>::Joinstabilize(int ID, int t)
{
	if (v[v[localid].successor].presuccessor != localid) {//�����ڵ�ʱ�����
		v[localid].successor = v[v[localid].successor].presuccessor;
		v[v[localid].successor].presuccessor = localid;
	}
	else {
		int id = localid, n = 0;//���¼���Ľڵ���ȫ���뵽������
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
	//����·�ɱ�;
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
	int index = 0;//�����¼���ڵ��·�ɱ�
	for (auto i = FingerTable[ID].begin(); i != FingerTable[ID].end(); i++) {
		int nextindex = (ID + (int)pow(2, index)) % size;
		index++;
		int nextid = findsuccessor(nextindex, 0);
		(*i).ID = nextid;
		(*i).IP = v[nextid].IP;
	}
}


/*��һ���ڵ��뿪ʱ,���������������ڵ��·�ɱ�*/
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

/*����·�ɱ�Ĵ�С*/
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

