#include <stdlib.h> 
#include <stdio.h>
#include <string.h>

typedef struct AddrSet {
    unsigned int len;// Number of instructions in this 
    unsigned long long * addr;// Array of instructions with consecutive addresses
    struct AddrSet * next;// Next consecutive addresses block
}AddrSet;

struct AddrSet * header;

unsigned long long tr[2][8005];
char buffer[40];
char source_addr[13];
char dest_addr[13];

AddrSet * init_addrset(unsigned long long beginner, unsigned long long ending) {
    AddrSet * new_block = (AddrSet*)malloc(sizeof(AddrSet));
    unsigned int length = (ending - beginner)/4 + 1;
    new_block->len = length;
    new_block->addr = (unsigned long long*)malloc(length*sizeof(unsigned long long));
    for(int i=0; i<length; i++) {
        new_block->addr[i] = beginner + i*4;
        printf("%llx\n",new_block->addr[i]);
    }
    new_block->next = NULL;
    return new_block;
}

int isHex(char x) {
    return (x>='0'&&x<='9')||(x>='a'&&x<='f');
}

unsigned long long toHex(char t[13]) {
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

void handler(int pointer, char*buffer) {
    int id = -1;
    unsigned long long source_num = 0;
    unsigned long long dest_num = 0;

    while(buffer[++id]==' ');
    if( isHex(buffer[id]) && buffer[id]!='0' ) {
        //source addr
        strncpy(source_addr, buffer+id, 12);
        source_addr[12]='\0';
        printf("%s\n", source_addr);// test code
        //update id
        id += 12;
        // initialize number
        source_num = toHex(source_addr);
    }

    while(!isHex(buffer[++id]));
    if( isHex(buffer[id]) && buffer[id]!='0' ) {
        //dest addr
        strncpy(dest_addr, buffer+id, 12);
        dest_addr[12]='\0';
        //No need to update id
        //initialize number
        dest_num = toHex(dest_addr);
    }

    //update raletion array
    tr[0][pointer] = source_num;
    tr[1][pointer] = dest_num;
}

void restore_all_instructions(FILE *fp, int pointer) {

    unsigned long long s_addr;// source address
    unsigned long long d_addr;// destination address
    s_addr = tr[0][0];// first jmp instruction 

    /*Header Initialization*/
    header = (AddrSet*)malloc(sizeof(AddrSet));
    header->len = 1;
    header->addr = (unsigned long long*)malloc(1*sizeof(unsigned long long));
    header->addr[0] = s_addr;
    header->next = NULL;
    fprintf(fp, "%llx\n", s_addr);

    /*Temp of Header*/
    AddrSet * temp = header;

    for(int i=0; i<pointer-1; i++) {
       unsigned long long d_addr = tr[1][i];
       unsigned long long s_addr = tr[0][i+1];
       temp->next = init_addrset(d_addr, s_addr);
       temp = temp->next;
       for(int j=0; j<temp->len; j++) {
           fprintf(fp, "%llx\n", temp->addr[i]);
       }
    }

    /*Ending Initialization*/
    temp->next = init_addrset(tr[1][pointer-1],tr[1][pointer-1]);
    fprintf(fp, "%llx\n", tr[1][pointer-1]);
}


void restore_instructions_to_file(FILE*fp, int pointer) {
    unsigned long long s_addr;// source address
    unsigned long long d_addr;// destination address
    s_addr = tr[0][0];// first jmp instruction 
    fprintf(fp, "%llx\n", s_addr);

    for(int i=0; i<pointer-1; i++) {
       unsigned long long d_addr = tr[1][i];
       unsigned long long s_addr = tr[0][i+1];

       unsigned int length = (s_addr - d_addr)/4 + 1;
       for(int j=0; j<length; j++) {
           unsigned long long addr = d_addr + 4*j;
           fprintf(fp, "%llx\n", addr);
       }
    }
    fprintf(fp, "%llx\n", tr[1][pointer-1]);
}


int main() {
    FILE *fp = NULL;
    fp = fopen("a2.in", "rw");
    int pointer = 0;

    printf("%ld\n",sizeof(buffer));
    while(fgets(buffer, sizeof(buffer), fp)!=NULL) {
        handler(pointer, buffer);
        pointer++;
    }
    //close file
    fclose(fp);
    //test code
    // for(int j=0;j<pointer;j++){
    // 	printf("%d : %llx %llx\n",j,tr[0][j],tr[1][j]);//0:source , 1:destination
	// }

    FILE *resfile = NULL;
    resfile = fopen("res_trace.txt","w");
    if(resfile==NULL) printf("No!\n");
    //restore all instruction addresses
    //restore_all_instructions(resfile, pointer);
    restore_instructions_to_file(resfile, pointer);
    fclose(resfile);
}