#include <stdlib.h>  // strtoull
#include <stdio.h>
#include <string.h>

unsigned long long tr[2][8005];
char tmp[12];
char c;


int ishex(char c){
	return (c=='0'||c=='1'||c=='2'||c=='3'||c=='4'||c=='5'||c=='6'||c=='7'||c=='8'||c=='9'||c=='a'||c=='b'||c=='c'||c=='d'||c=='e'||c=='f');
}

unsigned long long tohex(char t[12]){
	unsigned long long tnum=0;
	for(int i=0;i<12;i++){
		tnum=tnum<<4;
		if(t[i]<='9'&&t[i]>='0')
		tnum+=(t[i]-'0');
		else if(t[i]<='f'&&t[i]>='a')
		tnum+=(t[i]-'a'+10);
	}
	return tnum;
}

int inputnum(int flag,int num){
	tmp[0]=c;
		if(tmp[0]=='0'){
			tmp[1]=getchar();
			if(!ishex(tmp[1])){
				tr[flag][num]=0;
				return 1;
			}
			else{
				for(int j=2;j<12;j++){
					c=getchar();
					if(!ishex(c)){
						//printf("length error!\n");
						tr[flag][num]=-1;
						return 0;
					}
					else tmp[j]=c;
					}
				tr[flag][num]=tohex(tmp);
				}
		}
		else{
			for(int j=1;j<12;j++){
				c=getchar();
				if(!ishex(c)){
					//printf("length error!\n");
					tr[flag][num]=-1;
					return 0;
				}
				else tmp[j]=c;
			}
			tr[flag][num]=tohex(tmp);
		} 
		return 1;
}

int filterhd(int point){ 
	while(1){
		while(c=getchar(),c==' ');
		if(!inputnum(0,point)){
			while(c!='\n') c=getchar();
			continue;
		}
		while(c=getchar(),(c==' '||c=='='||c=='>'||c=='\n'));
		if(!inputnum(1,point)){
			while(c!='\n') c=getchar();
			point++;
			continue;
		}
		//inputnum(1,point);
		while(c=getchar(),(c==' '||c=='='||c=='>'||c=='\n'));
		point++;
		return point;
	}
}


int main() {
	freopen("a1.in","rw",stdin);
	
	int point=0;
	point = filterhd(point);
	while(c!=EOF){
		inputnum(0,point);
		while(c=getchar(),(c==' '||c=='='||c=='>'||c=='\n'));
		inputnum(1,point);
		while(c=getchar(),(c==' '||c=='='||c=='>'||c=='\n'));
		point++;
	}
	for(int j=0;j<point;j++){
		printf("%d : %llx %llx\n",j,tr[0][j],tr[1][j]);//0:source , 1:destination
	}
}