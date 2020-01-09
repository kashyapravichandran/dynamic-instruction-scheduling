#include<iostream>
#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<iomanip>
#include<fstream>
#include<math.h>

using namespace std;

ifstream fin;

enum 
{
	IF=0, ID, IS, EX, WB
};

struct entry
{
	int tag;
	
	int RS1;
	int name_RS1;
	int RS1_ready;
	
	int RS2;
	int name_RS2;
	int RS2_ready;
	
	int RD;
	
	int address;
	
	int IF_count;
	int ID_count;
	int IS_count;
	int EX_count;
	int WB_count;
	
	int type;
	int counter;
	int state; 
	
	entry *next;
	
};

class cache
{
	int BLOCKSIZE;
	int offset;
	int tag_from_address;
	int index_from_address;
	int L1_SIZE;
	int L1_ASSOC;
	int L2_SIZE;
	int L2_ASSOC;
	int L2_DATA_BLOCK;
	int L2_ADDR_TAGS;
	int **tag; 
	int **lru;
	int **selection_bits;
	int **valid_flag;
	int **valid_tag;
	int index;
	int index_bits;
	int block_bits;
	cache *nextlevel;
	int **dirty_flag;// to identify if a dirty block is evicted
	int c0_bits;
	int c1_bits;
	int c2_bits;
	
	public: 
// initializing the attributes of class cache
	int reads, writes, rmiss, wmiss, write_back,sector_miss,cache_block_miss;
	void insert(int bs,int l1_size, int l1_assoc, cache *p)
	{
		//cout<<"Insert";
		reads=0;
		writes=0;
		rmiss=0;
		wmiss=0;
		write_back=0;
		BLOCKSIZE=bs;
		L1_SIZE=l1_size;
		L2_ADDR_TAGS=1;
		L2_DATA_BLOCK=1;
		L1_ASSOC=l1_assoc;
		nextlevel=p; // NULL cause there is no next level
		index=L1_SIZE/(BLOCKSIZE*L1_ASSOC);
		//cout<<"Index"<<index;
		tag = new int* [index];
		for(int i=0;i<index;i++)
			tag[i]=new int[L1_ASSOC];
		
		for(int i=0;i<index;i++)
		for(int j=0;j<L1_ASSOC;j++)
			tag[i][j]=0;
		
		valid_flag = new int* [index];
		for(int i=0;i<index;i++)
			valid_flag[i]=new int[L1_ASSOC];
		
		lru = new int* [index];
		for(int i=0;i<index;i++)
			lru[i]=new int[L1_ASSOC];	
		
		dirty_flag = new int* [index];
		for(int i=0;i<index;i++)
			dirty_flag[i]=new int[L1_ASSOC];
		
		index_bits=log2(index);
		block_bits=log2(BLOCKSIZE);
		
		for(int i =0;i<index;i++)
		for(int j=0;j<L1_ASSOC;j++)
		dirty_flag[i][j]=0;
		
		for(int i =0;i<index;i++)
		for(int j=0;j<L1_ASSOC;j++)
		valid_flag[i][j]=0;	
		
		for(int i =0;i<index;i++)
		for(int j=0;j<L1_ASSOC;j++)
		lru[i][j]=L1_ASSOC;
		
		//cout<<"index "<<index;
		//cout<<"\n index bits"<<index_bits;
		//cout<<"\noffset bits"<<block_bits;
	
	}
	
		
	bool readFromAddress(int address, int &hitmiss)
	{
		//cout<<"HEre";
		reads++;
		int temphit;
		int ra_tag,ra_index;
		split(address,ra_tag,ra_index);
		for(int i=0;i<L1_ASSOC;i++)
		{
			if(tag[ra_index][i]==ra_tag&&valid_flag[ra_index][i]) // hit
			{
				//update LRU
				update_lru(ra_index,i);
				hitmiss=1;
				return true;
			}
		}
		
		rmiss++;
		
		for(int i=0;i<L1_ASSOC;i++)
		{
			if(!valid_flag[ra_index][i])
			{
				
				tag[ra_index][i]=ra_tag; // invalid miss
				//update LRU Counter
				lru[ra_index][i]=0;
				for(int j=0;j<L1_ASSOC;j++)
				{
					if(j!=i&&valid_flag[ra_index][j])
						lru[ra_index][j]++;
					
				}
				//add one to every lru?
				dirty_flag[ra_index][i]=0;
				valid_flag[ra_index][i]=1;
				
					if(nextlevel)
					{
						if(nextlevel->L2_ADDR_TAGS==1)
						{	
							//nextlevel->reads++;
							bool test=nextlevel->readFromAddress(address,temphit);
							if(!test)
							{
								hitmiss=3;
							}
							else 
								hitmiss=2;
								//nextlevel->rmiss++;
						}
					}
					else 
					{
						hitmiss=2;	// here temp;	
					}			
				return false;
				
			}
					
		}			
		int lru_hor_index=0;
		for(int i =0;i<L1_ASSOC;i++)
		{
			// Find Least recently used memeory
			if(lru[ra_index][i]==(L1_ASSOC-1)&&valid_flag[ra_index][i])
			{
				lru_hor_index=i;
				break;
			}
		}
		
		tag[ra_index][lru_hor_index]=ra_tag;
		lru[ra_index][lru_hor_index]=0;
		for(int j=0;j<L1_ASSOC;j++)
		{
			if(j!=lru_hor_index&&valid_flag[ra_index][j])
				lru[ra_index][j]++;
					
		}
		
		valid_flag[ra_index][lru_hor_index]=1;	
		if(nextlevel)
		{
			if(nextlevel->L2_ADDR_TAGS==1)
			{	
				//nextlevel->reads++;
				bool test=nextlevel->readFromAddress(address,temphit);
				if(!test)
				{
					hitmiss=3;
				}
				else 
				{
					hitmiss=2;
				}
					//nextlevel->rmiss++;
			}
		}
		else 
		{
				hitmiss=2;// 
		}

		dirty_flag[ra_index][lru_hor_index]=0;
		return false;
		
	}
	
	
	void split(int address, int &tag_address,int &index_address )
	{
		int temp2=0;
		temp2=pow(2,block_bits)-1;
		offset=address&temp2;
		int temp= address>>(block_bits);
		int temp1=1;
		temp1=pow(2,index_bits)-1;
		index_address=temp&temp1;
		tag_address=address>>(block_bits+index_bits);
		
	}
	
	void update_lru(int index,int lru_index)
	{
		// Memeber function to update the LRU counter
		int temp=lru[index][lru_index];
		lru[index][lru_index]=0;
		for(int i=0;i<L1_ASSOC;i++)
		{
			if(lru[index][i]<temp&&valid_flag[index][i]&&lru_index!=i)
				lru[index][i]++;
		}
	}
	
	void output1(int test=0)
	{
		
		cout<<"\na. number of accesses :"<<reads;
		cout<<"\nb. number of misses :"<<rmiss;
		cout<<"\n";
		if(test)
		{
			cout<<"set 0 : ";
			for(int j=0;j<L1_ASSOC;j++)
			{
				cout<<"  "<<std::hex<<tag[0][j]<<std::dec<<" ";
				
			}
			cout<<"\n";
		}	
		
		for(int i =test;i<index;i++)
		{
			cout<<"set "<<i<<" :";
			for(int j=0;j<L1_ASSOC;j++)
			{
				cout<<""<<std::hex<<tag[i][j]<<std::dec<<" ";
				
			}
			cout<<"\n";
		}
	

	}
	

};

class superscalar
{
	int S,N, IC, cycle;
	int schedule, dispatch;
	int name[128];
	int valid[128];
	entry *front, *rear;
	cache L1,L2;
	int L1_size;
	int L2_size;
		
	void enqueue(entry *p)
	{
		if(front==NULL)
		{
			front=p;
			rear=p;
			return;
		}
		rear->next=p;
		rear=p;	
	}
	
	void dequeue()
	{
		entry *temp;
		
		temp=front;
		if(front==rear)
		{
			front=NULL;
			rear=NULL;
			//delete(temp);
		}
		else 
			front=front->next;
			
		delete(temp);
	}
		
	public:
		
	superscalar(int s, int n, int BS, int L1S, int L1A, int L2S, int L2A)
	{
		S=s;
		N=n;
		for(int i=0;i<127;i++)
		valid[i]=1;
		schedule=0;
		dispatch=0;
		IC=0;
		cycle=-1;
		front=NULL;
		rear=NULL;
		L1_size=L1S;
		L2_size=L2S;
		//L2=NULL;
		if(BS)
		{
			//cout<<"Here";
			if(L2S)
			{
				L2.insert(BS,L2S,L2A,NULL);
				L1.insert(BS,L1S,L1A,NULL);
			}
			else
				L1.insert(BS,L1S,L1A,NULL);
		//	cout<<"Works?";
			//L1.insert(BS,L1S,L1A,L2);
		// 	cout<<"This?";
		}
	}

	
	void Fake_Retire()
	{
		//cout<<"Here";
		if(front)
			while(front->state==WB)
			{
				//cout<<"?";
				cout<<front->tag<<" fu{"<<front->type<<"} src{"<<front->RS1<<","<<front->RS2<<"} dst{"<<front->RD<<"} IF{"<<front->IF_count<<",1} ID{"<<front->ID_count<<","<<front->IS_count-front->ID_count<<"} IS{"<<front->IS_count<<","<<front->EX_count-front->IS_count<<"} EX{"<<front->EX_count<<","<<front->WB_count-front->EX_count<<"} WB{"<<front->WB_count<<",1}\n";
				
				dequeue();
				if(front==NULL)
					break;
		
			}
	//cout<<"Finished";
	}
	
	void Execute()
	{
		entry *temp=front;
		
		//srearing data to the renaming map table and the reservation station
		while(temp)
		{
			if(temp->state==EX&&temp->counter==0)
			{
				temp->state=WB;
				temp->WB_count=cycle;
				if(temp->RD!=-1&&name[temp->RD]==temp->tag)
				{
					valid[temp->RD]=1;
				}
				
				entry *temp1=front;
				while(temp1)
				{
					if(temp1->state==IS)
					{
						if(temp1->RS1!=-1&&!temp1->RS1_ready&&temp1->name_RS1==temp->tag)
						{
							temp1->RS1_ready=1;
						}
						if(temp1->RS2!=-1&&!temp1->RS2_ready&&temp1->name_RS2==temp->tag)
						{
							temp1->RS2_ready=1;
						}
					}
					temp1=temp1->next;
				}
			}
			temp=temp->next;
		}
		
		temp=front;
		
		
		//decreasing the counter value
		while(temp)
		{
			if(temp->state==EX&&temp->counter)
			{
				temp->counter--;
			}
			temp=temp->next;
		}
	}
	
	
	void Issue()
	{
		int check;
		entry *temp=front;
		int execute=0;
		while(temp)
		{
			
			if(temp->state==IS&&temp->RS1_ready&&temp->RS2_ready&&execute<N)
			{
				//cout<<" ? ";
				execute++;
				schedule--;
				temp->state=EX;
				temp->EX_count=cycle;
				if(temp->type==0)
				{
					temp->counter=0;
				}
				else if(temp->type==1)
				{
					temp->counter=1;
				}
				else if(temp->type==2)
				{
					if(L2_size)
					{
						//cout<<"Here";
						bool test=L1.readFromAddress(temp->address,check);
						if(test)
						{
							temp->counter=4;
							
						}
						else 
						{
							test=L2.readFromAddress(temp->address,check);
							if(test)
								temp->counter=9;
							else 
								temp->counter=19;
						}
							
					}
					else if(L1_size)
					{
						bool test=L1.readFromAddress(temp->address,check);
						if(test)
							temp->counter=4;
						else 
							temp->counter=19;
					}
					else 
					{
						temp->counter=4;
					}
				}
			}
			temp=temp->next;
		}
	}
	
	void Dispatch()
	{
		entry *temp=front;
		while(temp)
		{
			if(temp->state==ID&&schedule<S)
			{
				temp->state=IS;
				temp->IS_count=cycle;
				schedule++;
				dispatch--;
				
				int index=temp->RS1;
				if(index!=-1)
				{
					if(!valid[index])
					{
						temp->name_RS1=name[index];
						temp->RS1_ready=0;	
					}
					else 
						temp->RS1_ready=1;
				}
				else 
					temp->RS1_ready=1;
					
				index=temp->RS2;
				if(index!=-1)
				{
					if(!valid[index])
					{
						temp->name_RS2=name[index];
						temp->RS2_ready=0;
					}
					else
						temp->RS2_ready=1;
				}
				else 
					temp->RS2_ready=1;
				
				index=temp->RD;
				if(index!=-1)
				{
					name[index]=temp->tag;
					valid[index]=0;	
				}	
			}
			else if(temp->state==IF)
			{
				//cout<<"IFFF ";
				temp->state=ID;
				temp->ID_count=cycle;
					
			}
			
			temp=temp->next;		
		}
	}
	
	void Fetch()
	{
		int RS1;
		int RD;
		int RS2;
		int PC;
		int address;
		int type;
		
		entry *temp;
		
		for(int i=0;i<N;i++)
		{
			if(!fin.eof()&&dispatch<2*N)
			{
				//cout<<"IC : " <<IC;
				if(fin>>hex>>PC>>dec>>type>>RD>>RS1>>RS2>>hex>>address)
				{
						
					dispatch++;
					temp= new entry;
					temp->tag=IC;
					temp->IF_count=cycle;
					temp->state=IF;
					
					temp->type=type;
					
					IC++;
					temp->RD=RD;
					temp->RS1=RS1;
					temp->RS2=RS2;
					temp->address=address;
					temp->next=NULL;
					
					temp->name_RS1=temp->name_RS2=0;
					temp->RS1_ready=temp->RS2_ready=0;
					temp->counter=0;
					
					//cout<<temp->tag<<temp->state;
					//PC=-1;
					enqueue(temp);
				}
			}
		}
		
	}
	
	int Advance_Cycle()
	{
		cycle++;
		
		return front!=NULL;
	}
	void output()
	{
		cycle--;
		if(L1_size)
			{
				cout<<"L1 CACHE CONTENTS";
				L1.output1(1);
				cout<<"\n";
			}
		if(L2_size)
			{
				cout<<"L2 CACHE CONTENTS";
				L2.output1();
				cout<<"\n"; 
			}
			
		
		cout<<"CONFIGURATION";
		cout<<"\n superscalar bandwidth (N) = "<<N;
		cout<<"\n dispatch queue size (2*N) = "<<2*N;
		cout<<"\n schedule queue size (S) = "<<S;
		cout<<"\nRESULTS";
		cout<<"\n number of instructions = "<<IC;
		cout<<"\n number of cycles = "<<cycle;
		cout<<std::setprecision(2)<<std::fixed;
		cout<<"\n IPC = "<<float(IC)/cycle;
	}
	
};

int main(int argc, char **argv)
{
	if(!argc)
		cout<<"Invalid Call";
	else
	{
		int blocksize, l1_size, l1_assoc, l2_size, l2_assoc;
		int S,N; 
		S=atoi(argv[1]);
		N=atoi(argv[2]);
		blocksize=atoi(argv[3]);
		l1_size=atoi(argv[4]);
		l1_assoc=atoi(argv[5]);
		l2_size=atoi(argv[6]);
		l2_assoc=atoi(argv[7]);
		
		fin.open(argv[8]);
		superscalar processor(S,N,blocksize,l1_size,l1_assoc,l2_size,l2_assoc);
		
		//cout<<fin.is_open();
		while(processor.Advance_Cycle()||!fin.eof())
		{
		
				processor.Fake_Retire();
				processor.Execute();
				processor.Issue();
				processor.Dispatch();
				processor.Fetch();
			
			//	entry *temp=front;
			//	while(temp)
			//	{
			//		cout<<"\n\n IC : "<<temp->tag;
			//		cout<<"\n\n state : "<<temp->state;
			//		cout<<"\n\n RD :    "<<temp->RD;
			//		cout<<"\n Ready : "<<temp->RS1_ready<<temp->RS2_ready;
			//		temp=temp->next;
				
			//	}
			//	cin>>blocksize;
			//	cout<<"\n--------------------------------------------------cycle : "<<cycle;
		}
		processor.output();
		
	}
}
