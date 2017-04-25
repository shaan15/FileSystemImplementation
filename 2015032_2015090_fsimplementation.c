/*Done by: Gunkirat Kaur (2015032) and Shaan Chopra (2015090) */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BLOCK_NUM 4096
#define NUM_INODE_BLOCKS 5
#define NUM_DATA 56
#define NUM_INODES 80
#define INODE_SIZE 256

typedef struct _inode{
    int size;
    int num_blocks;
    int data_pointers;
} inode;
  
typedef struct _file_on_disk{
char name[100];
int count_files;
char *file_in_disk[NUM_INODES];
int file_inodes[NUM_INODES];
    
} disk;

int count=0;
FILE *fileid[10];
disk *files[10];

char inode_bitmap[BLOCK_NUM];
char data_bitmap[BLOCK_NUM];

int inode_strtadd=12288;
int data_startaddress=8;
int inode_bitmap_blockadd=1;
int data_bitmap_blockadd=2;


void write_data(int disk_id,int block_num,void *block);
  
int createSFS(char *filename){
    int i;
    for(i=0;i<count;i++){

        if(strcmp(filename,files[i]->name)==0){
            printf("File aready exists");
            return -1;
        }
    }
    files[count]=(disk *)malloc(sizeof(disk));
    strcpy(files[count]->name,filename);
    files[count]->count_files=0;
    fileid[count]=fopen(filename,"wb+");
    for(i=0;i<NUM_INODES;i++){
        inode_bitmap[i]='0';
    }
    for(i=0;i<NUM_DATA;i++){
       data_bitmap[i]='0';
    }    
         
    write_data(count,inode_bitmap_blockadd,inode_bitmap);
    write_data(count,data_bitmap_blockadd,data_bitmap);   
    count++;
    printf("Created %s\n",filename );
    return 1;
}

void read_data(int disk_id,int block_num,void *block){
    fseek(fileid[disk_id],BLOCK_NUM*block_num,SEEK_SET);
    fread(block,1,BLOCK_NUM,fileid[disk_id]);

}

void write_data_1(int disk_id,int block_num,void *block){
    read_data(disk_id,data_bitmap_blockadd,data_bitmap);
    fseek(fileid[disk_id],BLOCK_NUM*block_num,SEEK_SET);
    fwrite(block,1,BLOCK_NUM,fileid[disk_id]);
    // printf("%d\n",block_num );
    if(data_bitmap[block_num]=='0')
        data_bitmap[block_num]='1';
    else
        printf("%s\n","Data already exist" );
    fseek(fileid[disk_id],BLOCK_NUM*data_bitmap_blockadd,SEEK_SET);
    fwrite(data_bitmap,1,BLOCK_NUM,fileid[disk_id]);

}
void write_data(int disk_id,int block_num,void *block){
    
    fseek(fileid[disk_id],BLOCK_NUM*block_num,SEEK_SET);
    fwrite(block,1,BLOCK_NUM,fileid[disk_id]);
}

int check_inode(){
    int i;
    for(i=3;i<NUM_INODES;i++){
        // printf("%c\n",inode_bitmap[i] );
        if(inode_bitmap[i]=='0'){
            inode_bitmap[i]='1';
            return i;
        }
    }

return -1;

}

int allocate_pointer(int x){
    for(int i=0;i<NUM_DATA;i++){
        if(data_bitmap[i]=='0'){
            data_bitmap[i]='1';
            return i;
        }
    }
    inode_bitmap[x]='0';
    return -1;
}

void  write_file(int disk_id,char *filename,void *block){
    read_data(disk_id,inode_bitmap_blockadd,inode_bitmap);
    read_data(disk_id,data_bitmap_blockadd,data_bitmap);
    int index=check_inode();
    char *temp=(char*)block;
    // printf("%d\n",index );
    if(index==-1){
        printf("Inodes full");
        return ;
    }
    inode *ptr=(inode *)malloc(sizeof(inode));
    ptr->size=strlen(temp);
    if((ptr->size)%BLOCK_NUM==0)
        ptr->num_blocks=(ptr->size)/BLOCK_NUM;
    else
        ptr->num_blocks=(ptr->size/BLOCK_NUM)+1;
    ptr->data_pointers=allocate_pointer(index);
    // printf("allocated\n");
    if(ptr->data_pointers==-1)
    {
        printf("Insufficient disk space\n");
        return;
    }
    int n=files[disk_id]->count_files;
    // printf("%d\n",index );
    files[disk_id]->file_inodes[n]=index;
    // printf("%s\n",filename );
    files[disk_id]->file_in_disk[n]=filename;
    // printf("%s\n", files[disk_id]->file_in_disk[n]);
    write_data(disk_id,data_startaddress+(ptr->data_pointers),block);
    int t=inode_strtadd+(index*INODE_SIZE);
    // printf("writteeeeeeee\n");
    int x=fseek(fileid[disk_id],t,SEEK_SET);
    if(x!=0) 
        return;
    fwrite(ptr,1,INODE_SIZE,fileid[disk_id]);
    write_data(disk_id,inode_bitmap_blockadd,inode_bitmap);
    write_data(disk_id,data_bitmap_blockadd,data_bitmap);
    (files[disk_id]->count_files)++;
    // printf("Written\n");
}    
    
void read_file(int disk_id,char *filename,void *block){
    for(int i=0;i<(files[disk_id]->count_files);i++){
        if(strcmp(files[disk_id]->file_in_disk[i],filename)==0){
            int t=inode_strtadd+(files[disk_id]->file_inodes[i]*INODE_SIZE);
            int x=fseek(fileid[disk_id],t,SEEK_SET);
            if(x!=0) return;
            char *temp=(char *)malloc(sizeof(char)*INODE_SIZE);
            fread(temp,1,INODE_SIZE,fileid[disk_id]);
            inode *ptr=(inode *)temp;
            read_data(disk_id,data_startaddress+(ptr->data_pointers),block);
            return;
        }
    }
    printf("Error reading file\n");
}   

void print_inodeBitmap(int fid){
    char buffer[BLOCK_NUM];
    read_data(fid,inode_bitmap_blockadd,buffer);
    int i;
    for(i=0;i<NUM_INODES;i++){
        printf("%c ", buffer[i]);
    }
    printf("\n");   
}

void print_dataBitmap(int fid){
    char buffer[BLOCK_NUM];
    read_data(fid,data_bitmap_blockadd,buffer);
    int i;
    for(i=0;i<NUM_DATA;i++){
        printf("%c ", buffer[i]);
    }
    printf("\n"); 
}

// I think this function is uncorrect. Since we are not using directories, how do we print file names of individual files on different disks?
void print_FileList(int fid){
    // inode * i = (inode *)malloc(sizeof(inode));
    //i=read_inode(fsid,root_inode);
    char data_array[BLOCK_NUM];

    // read_data(fid,data_startaddress+i->data_pointers,data_array);
    if(files[fid]->count_files==0)
       {
       printf("No files\n");
       return;
        }    
    int j;
    for(j=0; j<files[fid]->count_files; j++){
        printf("%s ",files[fid]->file_in_disk[j]);
    }
    printf("\n");
}

int main(){
    
    char filename[100];
    strcpy(filename,"file");
    int check=createSFS(filename);
    if(check!=1)
        printf("Not created\n");
    else
    {   int i;
        for(i=0;i<count;i++)
            if(strcmp(files[i]->name,filename)==0)
                break;
        //printf("%d\n",i);
    }
//printf("File created\n");
    char command[100];
    printf("%s\n","Enter Command" );
    scanf("%s",command);
    while(strcmp(command,"exit")!=0){
        if(strcmp(command,"createSFS")==0){
            printf("Enter filename: ");
            scanf("%s",filename);
            int check=createSFS(filename);
            if(check==-1)
                 printf("Not created\n");
            else
             {  int i;
                 for(i=0;i<count;i++)
                     if(strcmp(files[i]->name,filename)==0)
                        break;
                // printf("%d\n",files[i]);
            }
            // printf("\n");

        }

        else if(strcmp(command,"readData")==0){ //works
            int did,blockNum;
            printf("Enter File ID and Data Block Number to read: ");
            scanf("%d %d",&did,&blockNum);
            char buffer[BLOCK_NUM];
            read_data(did,blockNum, buffer);
            printf("%s\n", buffer);
            printf("\n");
            
        }
        else if(strcmp(command,"writeData")==0){    //works for contiguos strings
            int did,blockNum;
            printf("Enter File ID and Data Block Number to write: ");
            scanf("%d %d",&did,&blockNum);
            printf("Enter Data (not space separated) to be written: ");
            char buffer[BLOCK_NUM+1];
            scanf ("%s", buffer);
            // fgets (buffer, 100, stdin);
            write_data_1(did,blockNum,buffer);
            //printf("%s\n", buffer);
            printf("Data block written\n");
            printf("\n");
        }
        else if(strcmp(command,"readFile")==0){
            char file_name[100];
            int did;
            printf("Enter Disk ID to read: ");
            scanf("%d",&did);
            printf("Enter Filename: ");
            scanf("%s",file_name);
            char buffer[BLOCK_NUM];
            read_file(did,file_name,buffer); 
            printf("%s\n", buffer);  
            printf("\n");  
            
        }
        else if(strcmp(command,"writeFile")==0){
            char file_name[100];
            int did;
            printf("Enter Disk ID to write: ");
            scanf("%d",&did);
            printf("Enter Filename: ");
            scanf("%s",file_name);
            char buffer[BLOCK_NUM];
            printf("Enter stuff (not space separated) to be written in file: ");
            scanf("%s",buffer);
            //printf("%s",buffer);
            write_file(did,file_name,buffer); 
            printf("File Written\n");    
            printf("\n");       
        }
        else if(strcmp(command,"print_inodeBitmap")==0){
            int k;
            for(k=0;k<count;k++){
                printf("Inode bitmap for File %d: \n",k);
                print_inodeBitmap(k);
            }
            printf("\n");
        }
        else if(strcmp(command,"print_dataBitmap")==0){
            int k;
            for(k=0;k<count;k++){
                printf("Data bitmap for File %d: \n",k);
                print_dataBitmap(k);
            }
            printf("\n");
        }
        else if(strcmp(command,"print_FileList")==0){
            int k;
            for(k=0;k<count;k++){
                printf("File list for File %d: \n",k);
                print_FileList(k);
            }
            printf("\n");
        }
        else{
             printf("Invalid command entered\n");
            
        }
        printf("%s\n","Enter Command" );
        scanf("%s",command);
    }
    

}