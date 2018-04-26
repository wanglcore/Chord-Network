#include"ChordBase.h"

template<class Key, class Value>
Base<Key, Value>::Base(string IP)
{
	localid = BKDHash(IP) % 2048;
	ChordNode localnode = { localid,IP,localid,localid };
	FingerNode fingernode = { localid,IP};
	/*fingernode.ID = localid;
	fingernode.IP = "127.0.0.1";*/
	IDlist.push_back(fingernode);
	v.resize(2048);
	v[localid] = localnode;
	nodenumber = 1;
	Kstore.resize(2048);
	FingerTable.resize(2048);
	FingerTable[localid].resize(11);
	for (auto i = FingerTable[localid].begin(); i != FingerTable[localid].end(); i++) {
		(*i).ID = localid;
		(*i).IP = IP;
	}
	Init();//初始化网络
}

template<class Key, class Value>
void Base<Key, Value>::Join(string IP, int t )
{
	FingerNode fingernode;
	int ID = BKDHash(IP) % 2048;
	IDlist.push_back({ IP,ID });
	ChordNode node = { ID,IP,0,0 };
	int IPsuccessor = findsuccessor(ID, t);
	Joinlist.push_back({ IP,ID });
	v[ID] = node;
	nodenumber++;
	v[ID].successor = IPsuccessor;
	v[IPsuccessor].presuccessor = ID;
	FingerTable[ID].resize(11);
	FingerTable[ID] = FingerTable[IPsuccessor];
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

template<class Key, class Value>
bool Base<Key, Value>::Leave(string IP)
{
	int ID = BKDHash(IP) % 2048;
	bool nofind = true;
	for (auto i = IDlist.begin(); i != IDlist.end(); i++) {
		if ((*i).ID == ID) {
			nofind = false;
			IDlist.remove({ IP,ID });
			int presuccessorid = v[ID].presuccessor;
			v[presuccessorid].successor = v[ID].successor;
			v[v[ID].successor].presuccessor = presuccessorid;
			for (auto i = Kstore[ID].begin(); i != Kstore[ID].end(); i++) {
				Kstore[v[ID].successor].push_back((*i));
			}
			Kstore[ID].clear();
			FingerTable[ID].clear();
			nodenumber--;
			Leavestabilize(ID);
			return true;
		}
	}
	if (nofind) {
		findlist.clear();
	}
	return false;
}

template<class Key, class Value>
int Base<Key, Value>::BKDHash(string s)
{
	std::string ss = s;
	int seed = 131;
	int hash = 0;
	for (char i : ss) {
		hash = hash * seed + i;
	}
	return (hash & 0x7FFFFFFF);
}

template<class Key, class Value>
int Base<Key, Value>::findsuccessor(int K, int t)
{
	if (t == 1)
		Joinlist.clear();
	findlist.clear();
	if (v[localid].presuccessor == localid && v[localid].successor == localid) {
		if (t == 1)
			Joinlist.push_back({ v[localid].IP ,localid });
		findlist.push_back({ v[localid].IP,localid });
		return localid;
	}
	else if (v[localid].presuccessor == v[localid].successor) {//只有两个节点
		if (v[localid].successor > localid) {
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
		else if (v[localid].successor<localid) {
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

template<class Key, class Value>
void Base<Key, Value>::Init()
{
	fstream os("./ip.txt");
	string line;
	while (getline(os, line)) {
		Join(line, 0);
	}
	os.close();
}

template<class Key, class Value>
void Base<Key, Value>::Joinstabilize(int ID, int t)
{
	if (v[v[localid].successor].presuccessor != localid) {
		v[localid].successor = v[v[localid].successor].presuccessor;
		v[v[localid].successor].presuccessor = localid;
	}
	else {
		int id = localid, n = 0;
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
			int nextindex = (tableid + (int)pow(2, index)) % 2048;
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
	int index = 0;
	for (auto i = FingerTable[ID].begin(); i != FingerTable[ID].end(); i++) {
		int nextindex = (ID + (int)pow(2, index)) % 2048;
		index++;
		int nextid = findsuccessor(nextindex, 0);
		(*i).ID = nextid;
		(*i).IP = v[nextid].IP;
	}
}

template<class Key, class Value>
void Base<Key, Value>::Leavestabilize(int K)
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
