#include "route.h"
#include "lib_record.h"
#include <stdio.h>
#include <math.h>
#include <hash_set>
#include <vector>
#include <string>
#include <cmath>
#include <climits>
#include <cstdlib>
#include <memory.h>

#define MAXVET 605 //最多结点数
#define MAXWEIGHT 65535 //路径最大权值
#define ANTCOUNT 100//一波放置的蚂蚁数目
#define ROU 0.5//信息素挥发速度
#define TIMELIMIT 9500//运行时限
using namespace std;
using namespace __gnu_cxx;
int Distances[MAXVET][MAXVET];//距离矩阵
double Tao[MAXVET][MAXVET];//信息素矩阵
int CityCount;//城市数量
vector<int> BestPath;//目前最佳路径
int BestLength;//目前最佳路径长度
hash_set<int> Demands;//需要经过的中间节点
vector<int> DemandsList;//中间节点的数组
int StartCity,EndCity;
class Ant
{
private:
	//随机选择一个城市
	int _randomSelect()
	{
		return DemandsList[rand()%DemandsList.size()];
	}
	//前往下一个城市
	void _moveToNextCity(int curCity)
	{
		//如果已经满足回巢条件&有边指向巢->直接回去
		if(_isSatisfied()&&Distances[curCity][start]<MAXWEIGHT)
		{
			isSucc=true;
			return;
		}
//		//一些剪枝，将不可能满足要求的蚂蚁死亡 可优化
//		bool flag=false;
//		for(int i=0;i<CityCount;++i)
//		{
//			if(Distances[curCity][i]<MAXWEIGHT&&visited.find(i)==visited.end())//有至少一条出路，且出口不在禁忌表中
//			{
//				flag=true;
//				break;
//			}
//		}
//		if(!flag)
//		{
//			isDead=true;
//			isSucc=false;
//			return;
//		}
		//利用信息素寻路
		double p[MAXVET];//每个城市被选中的概率
		double alpha=1.0;  
        double beta=2.0;  
        double sum=0.0; 
		//计算公式中的分母部分  
		for(int i=0;i<CityCount;++i)
		{
			if(Distances[curCity][i]<MAXWEIGHT&&visited.find(i)==visited.end())//有至少一条出路，且出口不在禁忌表中
			{
				p[i]=(pow(Tao[curCity][i], alpha)*pow(1.0/Distances[curCity][i], beta));
				sum+=p[i];  
			}
			else
			{
				p[i]=0.0;
			}
		}
		if(sum==0.0)
		{
			isDead=true;
			isSucc=false;
			return;
		}
		//计算每个城市被选中的概率  
		for(int i=0;i<CityCount;++i)
		{
			p[i]=p[i]/sum;
		}
		int rnd=rand()%10000;//精度可调整
		double selectrnd=(double)rnd/10000.0;
		double selectsum=0.0;
		for(int i=0;i<CityCount;++i)
		{
			if(Distances[curCity][i]<MAXWEIGHT&&visited.find(i)==visited.end())//有至少一条出路，且出口不在禁忌表中
			{
				if(selectrnd>selectsum&&selectrnd<=selectsum+p[i])//被选中
				{
					//move to i
					tourPath.push_back(i);
					visited.insert(i);
					tourLength+=Distances[curCity][i];
					if(i==StartCity)//被选中是起点,手动转移到终点
					{
						tourPath.push_back(EndCity);
						visited.insert(EndCity);
					}
					else if(i==EndCity)//终点 ->起点
					{
						tourPath.push_back(StartCity);
						visited.insert(StartCity);
					}
					return;
				}
				else
					selectsum+=p[i];
			}
		}
	}
	bool _isSatisfied()
	{
		if(visited.find(StartCity)==visited.end()|visited.find(EndCity)==visited.end())//还没经过起点或者终点
		{
			return false;
		}
		for(hash_set<int>::iterator i=Demands.begin();i!=Demands.end();++i)
		{
			if(visited.find(*i)==visited.end())
				return false;
		}
		return true;
	}
public: 
	int start;//起始城市
	vector<int> tourPath;//经过的路径
	bool isDead;//是否死亡
	hash_set<int> visited;//禁忌表
	int tourLength;//路径长度
	bool isSucc;//是否满足要求
	//初始化ant
	void set()
	{
		isDead=false;
		isSucc=false;
		tourPath.clear();
		visited.clear();
		tourLength=0;
		start=_randomSelect();
		tourPath.push_back(start);
		visited.insert(start);
	}
	
	//一只蚂蚁的移动过程
	void run()
	{
		for(int i=0;i<MAXVET;++i)
		{
			if(!isDead&&!isSucc)
				_moveToNextCity(tourPath[tourPath.size()-1]);
			else if(isDead)
			{
				isSucc=false;
				return;
			}
			else if(isSucc)
			{
				return;
			}
		}
	}
	
};
Ant Ants[ANTCOUNT];//构造ANTCOUNT只蚂蚁
//更新信息素矩阵
void UpdateTao()
{
	int i,j;
	//信息素挥发
	for(i=0;i<CityCount;++i)
		for(j=0;j<CityCount;++j)
			Tao[i][j]=Tao[i][j]*(1-ROU);
	//更新信息素
	for(i=0;i<ANTCOUNT;++i)
	{
		if(Ants[i].isSucc)
		{
			for(j=0;j<Ants[i].tourPath.size()-1;++j)
			{
				int s=Ants[i].tourPath[j];
				int t=Ants[i].tourPath[j+1];
				if(Distances[s][t]!=0)
					Tao[s][t]+=1.0/Distances[s][t];
			}
		}
		
	}
}
//打印读取的数据
void printLog()
{
	printf("start: %d\n",StartCity);
	printf("end: %d\n",EndCity);
	printf("demands:\n");
	for(hash_set<int>::iterator j=Demands.begin();j!=Demands.end();++j)
	{
		printf("%d ",*j);
	}
	printf("\n Distances:");
	for(int i=0;i<CityCount;++i)
    {
        for(int j=0;j<CityCount;++j)
            printf("%d ",Distances[i][j]);
        printf("\n");
    }
}

void OutPut()
{
	printf("BestLength %d\nBestPath\n",BestLength);
	for(vector<int>::iterator i=BestPath.begin();i!=BestPath.end();++i)
		printf("%d ",*i);
	printf("\n");
}
//判断是否超时
bool timeLimit(double time){

    if((double)clock()/CLOCKS_PER_SEC >= time)
        return true;
    return false;
}
//你要完成的功能总入口
void search_route(char *topo[5000], int edge_num, char *demand,clock_t startTime)
{
	//init
	int i,j;
	
	CityCount=0;
	BestLength=MAXWEIGHT;
	for(i=0;i<MAXVET;++i)
	{
        for(j=0;j<MAXVET;++j)
        {
            Distances[i][j]=MAXWEIGHT;
			Tao[i][j]=0.0;
		}
	}
	//read demand
	char sdemanPos[MAXVET];
    sscanf(demand,"%d,%d,%s", &StartCity, &EndCity, sdemanPos);
    string s(sdemanPos);
    s += '|';
    int start = 0;
    for( i=0; i<s.size(); ++i){
        if(s[i] == '|'){
            Demands.insert(atoi(s.substr(start, i-start).c_str()));
            start = i+1;
        }
    }
	//read topo
	for(i=0; i<edge_num; ++i)
    {
        int node[4];
        sscanf(topo[i], "%d,%d,%d,%d", &node[0],&node[1],&node[2],&node[3]);
        CityCount=max(CityCount,max(node[1],node[2])+1);//顶点最大数
        Distances[node[1]][node[2]]=min(Distances[node[1]][node[2]],node[3]);//更新距离矩阵
    }
	printLog();
	//cut
	//pre handle
	Distances[EndCity][StartCity]=0;
	Distances[StartCity][EndCity]=0;
	//running
	clock_t finshTime=clock();
	int gen=0;
	while(((finshTime-startTime)/CLOCKS_PER_SEC)<TIMELIMIT)
	{
		++gen;
        for(i=0;i<ANTCOUNT;++i)
        {	
			//随机放置这n只蚂蚁
            Ants[i].set();
			//线性的对每只蚂蚁的移动模拟
			Ants[i].run();
			//判断是否出现新路径
			if(Ants[i].isSucc&&Ants[i].tourLength<BestLength)//发现更短路径
			{
				printf("第%d代发现新解%d\n",gen,BestLength);
				BestLength=Ants[i].tourLength;
				//更新路径，注意起始点和终点
				BestPath.clear();
				for(j=0;j<Ants[i].tourPath.size();++j)
				{
					if(Ants[i].tourPath[j]==StartCity)
						return;
				}
				for(int k=j;k<Ants[i].tourPath.size();++k)
				{
					BestPath.push_back(Ants[i].tourPath[k]);
				}
				for(int k=0;k<j;++k)
				{
					BestPath.push_back(Ants[i].tourPath[k]);
				}
			}
        }
		//每波结束更新信息素
		UpdateTao();
	}
	//output
	OutPut();
}
