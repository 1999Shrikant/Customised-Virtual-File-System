////////////////////////////////////////////////////////////////////
//
//
//	Customized Virtual File System Application
//
//
////////////////////////////////////////////////////////////////////


////////////////////////
//
// Header Files
//
////////////////////////


#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<iostream>

////////////////////////
//
// Defining The Macros
//
////////////////////////

#define MAXINODE 50

#define READ 1
#define WRITE 2

#define MAXFILESIZE 2048

#define REGULAR 1
#define SPECIAL 2

#define START 0
#define CURRENT 1
#define END 2

/////////////////////////////////////
//
// Creating SuperBlock Structure
//
/////////////////////////////////////


typedef struct superblock
{
	int TotalInodes;
	int FreeInode;
}SUPERBLOCK,*PSUPERBLOCK;

/////////////////////////////////////
//
// Creating Inode Structure
//
/////////////////////////////////////


typedef struct inode		// 86 bytes allocated for this block
{
	char FileName[50];	// file name stored
	int InodeNumber;	// inode number
	int FileSize;		// 1024 byte
	int FileActualSize; // allocated when we write into it ie 10 bytes of data
	int FileType;		// type of file
	char *Buffer;		// On Windows It Stores Block
	int LinkCount;		// linking count
	int ReferenceCount; // reference count
	int permission;  	// read write permission(1,2,3)
	struct inode *next;	// self referential structure
}INODE,*PINODE,**PPINODE;

////////////////////////////////////
//
// Creating FileTable Structure
//
////////////////////////////////////


typedef struct filetable
{
	int readoffset;			// From where to read
	int writeoffset;		// From where to write
	int count;					// Remains 1 through the code
	int mode; 					// mode of file
	PINODE ptrinode;		// pointer, Linked point to inode
}FILETABLE,*PFILETABLE;

///////////////////////////////////
//
// Creating UFDT Structure
//
///////////////////////////////////

typedef struct ufdt         // create ufdt structure
{
	PFILETABLE ptrfiletable; // Pointer Which Points To File Table
}UFDT;

UFDT UFDTArr[50];			// Create Array Of Structure i.e Array Of Pointer
SUPERBLOCK SUPERBLOCKobj;	// global variable
PINODE head = NULL;		// global pointer

///////////////////////////////////////////////////////////////////////////////////////
//
//	Function Name	: 	man
//	Input			: 	char *
//	Output			: 	None
//	Description 	: 	It Display The Description For Each Commands
//	Author			: 	Shrikant Chobi
//	Date			:	19 June 2022
//
///////////////////////////////////////////////////////////////////////////////////////




void man(char *name)
{
	if(name == NULL) return;
	
	if(strcmp(name,"create")==0)
	{
		printf("Description: Used to create new regular file \n");
		printf("Usage: create File_name Permission\n");
	}
	else if(strcmp(name,"read")==0)
	{
		printf("Description: Used to read data from regular file \n");
		printf("Usage:read File_name No_Of_Bytes_To_Read \n");
	}
	else if(strcmp(name,"write")==0)
	{
		printf("Description: Used to write into regular file \n");
		printf("Usage:write File_name\n After this enter the data that we want to write \n");
	}
	else if(strcmp(name,"ls")==0)
	{
		printf("Description: Used to list all information of files\n");
		printf("Usage : ls \n");
	}
	else if(strcmp(name,"stat")==0)
	{
		printf("Description : Used to display information of file\n");
		printf("Usage: stat File_name\n");
	}
	else if(strcmp(name,"fstat")==0)
	{
		printf("Description:Used to display information of file\n");
		printf("Usage : stat File_Descriptor\n");
	}
	else if(strcmp(name,"truncate")==0)
	{
		printf("Description : Used to remove data from file\n");
		printf("Usage:truncate File_name\n");
	}
	else if(strcmp(name,"open")==0)
	{
		printf("Description : Used to open existing file\n");
		printf("Usage:open File_name mode\n");
	}
	else if(strcmp(name,"close")==0)
	{
		printf("Description:Used to close opened file\n");
		printf("Usage:close File_name\n");
	}
	else if(strcmp(name,"closeall")==0)
	{
		printf("Description:Used to close all opened file\n");
		printf("Usage: closeall\n");
	}
	else if(strcmp(name,"lseek")==0)
	{
		printf("Description:Used to change file offset\n");
		printf("Usage:lseek File_Name ChangeInOffset StartPoint\n");
	}
	else if(strcmp(name,"rm")==0)
	{
		printf("Description: Used to delete the file\n");
		printf("Usage:rm File_Name\n");
	}
	else
	{
		  printf("ERROR : No manual entry available.\n");
	}
}

///////////////////////////////////////////////////////////////////////////////////////
//
//	Function Name	: 	DisplayHelp
//	Input			: 	None
//	Output			: 	None
//	Description 	: 	It Display All List / Operations About This Applications
//	Author			: 	Shrikant Chobi
//	Date			:	19 June 2022
//
///////////////////////////////////////////////////////////////////////////////////////


void DisplayHelp()
{
	printf("ls 		    : To list out all File\n");
	//printf("clear : To clear console\n");
	printf("open 		: To open the file\n");
	printf("close 		: To close the file\n");
	printf("closeall 	: To close all opened files\n");
	printf("read 		: To Read content of file\n");
	printf("write 		: To write content into file\n");
	printf("exit 		: To Terminate file system\n");
	printf("stat 		: To Display information of file using name\n");
	printf("fstat 		: To Display information of fileusing file descriptor\n");
	printf("truncate 	: To Remove all data from file\n");
	printf("rm 		: To Delet the file\n");
}

///////////////////////////////////////////////////////////////////////////////////////
//
//	Function Name	: 	GetFDFromName
//	Input			: 	char *
//	Output			: 	Integer
//	Description 	: 	Get File Descriptor Value
//	Author			: 	Shrikant Chobi
//	Date			:	19 June 2022
//
///////////////////////////////////////////////////////////////////////////////////////



int GetFDFormName(char *name)
{
	int i=0;
	
	while(i<50)
	{
		  if(UFDTArr[i].ptrfiletable !=NULL)
			  if(strcmp((UFDTArr[i].ptrfiletable->ptrinode->FileName),name)==0)
				  break;
			  i++;
	}
	
	if(i == 50)           return-1;
	else                       return i;
}

///////////////////////////////////////////////////////////////////////////////////////
//
//	Function Name	: 	Get_Inode
//	Input			: 	char *
//	Output			: 	PINODE
//	Description 	: 	Return Inode Value Of File
//	Author			: 	Shrikant Chobi
//	Date			:	19 June 2022
//
///////////////////////////////////////////////////////////////////////////////////////


PINODE Get_Inode(char * name)
{
	PINODE temp = head;
	int i=0;
	
	if(name == NULL)
		   return NULL;
	   
	   while(temp!=NULL)
	   {
		   if(strcmp(name,temp->FileName)==0)
			   break;
		   temp=temp->next;
	   }
	   return temp;
}

///////////////////////////////////////////////////////////////////////////////////////
//
//	Function Name	: 	CreateDILB
//	Input			: 	None
//	Output			: 	None
//	Description 	: 	It Creates The DILB When Program Starts
//	Author			: 	Shrikant Chobi
//	Date			:	19 June 2022
//
///////////////////////////////////////////////////////////////////////////////////////


void CreateDILB()
{
	int i = 1;
	PINODE newn = NULL;
	PINODE temp = head;
	
	while(i<= MAXINODE)
	{
		   newn = (PINODE)malloc(sizeof(INODE));
		   
		 newn->LinkCount=0;
		 newn->ReferenceCount=0;
		 newn->FileType=0;
		 newn->FileSize=0;
		 
		    newn->Buffer = NULL;
			newn->next =NULL;
		
		newn->InodeNumber = i;
		
		    if(temp==NULL)
			{
				head = newn;
				temp = head;
			}
			else
			{
				temp->next = newn;
				temp = temp->next;
			}
			i++;
	}
	printf("DILB created successfully\n");
}

///////////////////////////////////////////////////////////////////////////////////////
//
//	Function Name	: 	InitialiseSuperBlock
//	Input			: 	None
//	Output			: 	None
//	Description 	: 	Initialize Inode Values 
//	Author			: 	Shrikant Chobi
//	Date			:	19 June 2022
//
///////////////////////////////////////////////////////////////////////////////////////



void InitialiseSuperBlock()
{
	int i=0;
	while(i< MAXINODE)
	{
		UFDTArr[i].ptrfiletable=NULL;
		i++;
	}
	
	SUPERBLOCKobj.TotalInodes = MAXINODE;
	SUPERBLOCKobj.FreeInode = MAXINODE;
}


///////////////////////////////////////////////////////////////////////////////////////
//
//	Function Name	: 	CreateFile
//	Input			: 	char*, Integer
//	Output			: 	None
//	Description 	: 	Create New Files 
//	Author			: 	Shrikant Chobi
//	Date			:	19 June 2022
//
///////////////////////////////////////////////////////////////////////////////////////


int CreateFile(char *name,int permission)
{
	int i=0;
	PINODE temp = head;
	
	if((name == NULL)||(permission == 0)||(permission>3))
		return -1;
	
	if(SUPERBLOCKobj.FreeInode == 0)
		return -2;
	
	(SUPERBLOCKobj.FreeInode)--;
	
	if(Get_Inode(name)!=NULL)
		return -3;
	
	while(temp!= NULL)
	{
		if(temp->FileType == 0)
			break;
		temp = temp->next;
	}
	
	while(i< MAXINODE)
	{
		if(UFDTArr[i].ptrfiletable==NULL)
			break;
		i++;
	}
	
	UFDTArr[i].ptrfiletable = (PFILETABLE)malloc(sizeof(FILETABLE));
	
	UFDTArr[i].ptrfiletable->count = 1;
	UFDTArr[i].ptrfiletable->mode = permission;
	UFDTArr[i].ptrfiletable->readoffset = 0;
	UFDTArr[i].ptrfiletable->writeoffset = 0;
	
	UFDTArr[i].ptrfiletable->ptrinode = temp;
	
	strcpy(UFDTArr[i].ptrfiletable->ptrinode->FileName,name);
	UFDTArr[i].ptrfiletable->ptrinode->FileType = REGULAR;
	UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount=1;
	UFDTArr[i].ptrfiletable->ptrinode->LinkCount=1;
	UFDTArr[i].ptrfiletable->ptrinode->FileSize=MAXFILESIZE;
	UFDTArr[i].ptrfiletable->ptrinode->FileActualSize= 0;
	UFDTArr[i].ptrfiletable->ptrinode->permission = permission;
	UFDTArr[i].ptrfiletable->ptrinode->Buffer = (char *)malloc(MAXFILESIZE);
	
	return i;
}

///////////////////////////////////////////////////////////////////////////////////////
//
//	Function Name	: 	rm_File
//	Input			: 	char*
//	Output			: 	Integer
//	Description 	: 	Remove Created Files 
//	Author			: 	Shrikant Chobi
//	Date			:	19 June 2022
//
///////////////////////////////////////////////////////////////////////////////////////


int rm_File(char * name) //  rm_File("Demo.txt");
{
	int fd = 0;
	
	fd = GetFDFormName(name);
	if(fd==-1)
		return -1;
	
	(UFDTArr[fd].ptrfiletable->ptrinode->LinkCount)--;
	
	if(UFDTArr[fd].ptrfiletable->ptrinode->LinkCount== 0)
	{
		    UFDTArr[fd].ptrfiletable->ptrinode->FileType = 0;
		//free(UFDTArr[fd].ptrfiletable->ptrinode->Buffer);
		     free(UFDTArr[fd].ptrfiletable);
	}
	
	UFDTArr[fd].ptrfiletable = NULL;
	(SUPERBLOCKobj.FreeInode)++;
	
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////
//
//	Function Name	: 	ReadFile
//	Input			: 	Integer, char*, Integer
//	Output			: 	Integer
//	Description 	: 	Read Data From File 
//	Author			: 	Shrikant Chobi
//	Date			:	19 June 2022
//
///////////////////////////////////////////////////////////////////////////////////////


int ReadFile(int fd, char *arr,int isize)
{
	int read_size = 0;
	
	if(UFDTArr[fd].ptrfiletable == NULL)            return -1;
	
	if(UFDTArr[fd].ptrfiletable->mode !=READ &&UFDTArr[fd].ptrfiletable->mode != READ+WRITE )                 return -2;
	
	if(UFDTArr[fd].ptrfiletable->ptrinode->permission != READ && UFDTArr[fd].ptrfiletable->ptrinode->permission != READ+WRITE)     return -2;
	
	if(UFDTArr[fd].ptrfiletable->readoffset == UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)    return -3;
	
	if(UFDTArr[fd].ptrfiletable->ptrinode->FileType != REGULAR)      return -4;
	
	read_size =(UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) - (UFDTArr[fd].ptrfiletable->readoffset);
	if(read_size < isize)
	{
		strncpy(arr,(UFDTArr[fd].ptrfiletable->ptrinode->Buffer) + (UFDTArr[fd].ptrfiletable->readoffset),read_size);
		
		UFDTArr[fd].ptrfiletable->readoffset = UFDTArr[fd].ptrfiletable->readoffset + read_size;
	}
	else
	{
		strncpy(arr,(UFDTArr[fd].ptrfiletable->ptrinode->Buffer) + (UFDTArr[fd].ptrfiletable->readoffset),isize);
		
		(UFDTArr[fd].ptrfiletable->readoffset) = (UFDTArr[fd].ptrfiletable->readoffset) +isize;
	}
	
	return isize;
}

///////////////////////////////////////////////////////////////////////////////////////
//
//	Function Name	: 	WriteFile
//	Input			: 	Integer, char*, Integer
//	Output			: 	Integer
//	Description 	: 	Write Data Into the File 
//	Author			: 	Shrikant Chobi
//	Date			:	19 June 2022
//
///////////////////////////////////////////////////////////////////////////////////////


int WriteFile(int fd,char *arr, int isize)
{
	if(((UFDTArr[fd].ptrfiletable->mode) != WRITE)&&((UFDTArr[fd].ptrfiletable->mode)!=READ+WRITE))   return -1;
	
	if(((UFDTArr[fd].ptrfiletable->ptrinode->permission) != WRITE)&&((UFDTArr[fd].ptrfiletable->ptrinode->permission)!=READ+WRITE))   return -1;
	
	if((UFDTArr[fd].ptrfiletable->writeoffset)==MAXFILESIZE)   return -2;
	
	if((UFDTArr[fd].ptrfiletable->ptrinode->FileType)!= REGULAR)   return -3;
	
	strncpy((UFDTArr[fd].ptrfiletable->ptrinode->Buffer)+ (UFDTArr[fd].ptrfiletable->writeoffset),arr,isize);
	
	(UFDTArr[fd].ptrfiletable->writeoffset) = (UFDTArr[fd].ptrfiletable->writeoffset)+ isize;
	
	(UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + isize;
	
	return isize;
}

///////////////////////////////////////////////////////////////////////////////////////
//
//	Function Name	: 	OpenFile
//	Input			: 	char*, Integer
//	Output			: 	Integer
//	Description 	: 	Open An Existing File
//	Author			: 	Shrikant Chobi
//	Date			:	19 June 2022
//
///////////////////////////////////////////////////////////////////////////////////////


int OpeanFile(char *name,int mode)
{
	int i=0;
	PINODE temp = NULL;
	
	if(name == NULL || mode <= 0)
		return -1;
	
	temp = Get_Inode(name);
	if(temp == NULL)
		return -2;
	
	if(temp->permission < mode)
		return -3;
	
	while(i<50)
	{
		if(UFDTArr[i].ptrfiletable == NULL)
			break;
		i++;
	}
	
	UFDTArr[i].ptrfiletable = (PFILETABLE)malloc(sizeof(FILETABLE));
	if(UFDTArr[i].ptrfiletable == NULL)     return -1;
	UFDTArr[i].ptrfiletable->count = 1;
	UFDTArr[i].ptrfiletable->mode = mode;
	if(mode == READ + WRITE)
	{
		UFDTArr[i].ptrfiletable->readoffset = 0;
		UFDTArr[i].ptrfiletable->writeoffset = 0;
	}
	else if(mode == READ)
	{
		UFDTArr[i].ptrfiletable->readoffset = 0;
	}
	else if(mode == WRITE)
	{
		UFDTArr[i].ptrfiletable->writeoffset = 0;
	}
	UFDTArr[i].ptrfiletable->ptrinode = temp;
	(UFDTArr[i].ptrfiletable->ptrinode ->ReferenceCount)++;
	
	return i;
}

///////////////////////////////////////////////////////////////////////////////////////
//
//	Function Name	: 	CloseFileByName
//	Input			: 	Integer
//	Output			: 	None
//	Description 	: 	Close Existing File By Its File Descriptor
//	Author			: 	Shrikant Chobi
//	Date			:	19 June 2022
//
///////////////////////////////////////////////////////////////////////////////////////


void CloseFileByName(int fd)
{
	UFDTArr[fd].ptrfiletable->readoffset=0;
	UFDTArr[fd].ptrfiletable->writeoffset=0;
	(UFDTArr[fd].ptrfiletable->ptrinode->ReferenceCount)--;
}

///////////////////////////////////////////////////////////////////////////////////////
//
//	Function Name	: 	CloseFileByName
//	Input			: 	char *
//	Output			: 	None
//	Description 	: 	Close Existing File By Its Name
//	Author			: 	Shrikant Chobi
//	Date			:	19 June 2022
//
///////////////////////////////////////////////////////////////////////////////////////


int CloseFileByName(char *name)
{
	int i=0;
	i = GetFDFormName(name);
	if(i == -1)
		return -1;
	
	UFDTArr[i].ptrfiletable->readoffset = 0;
	UFDTArr[i].ptrfiletable->writeoffset = 0;
	(UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount)--;
	
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////
//
//	Function Name	: 	CloseAllFile
//	Input			: 	None
//	Output			: 	None
//	Description 	: 	Close All Existing Files
//	Author			: 	Shrikant Chobi
//	Date			:	19 June 2022
//
///////////////////////////////////////////////////////////////////////////////////////


void CloseAllFile()
{
	int i = 0;
	while(i<50)
	{
		  if(UFDTArr[i].ptrfiletable != NULL)
		  {
			  UFDTArr[i].ptrfiletable->readoffset = 0;
			  UFDTArr[i].ptrfiletable->writeoffset=0;
			  (UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount)--;
			  break;
		  }
		  i++;
	}
}

///////////////////////////////////////////////////////////////////////////////////////
//
//	Function Name	: 	LseekFile
//	Input			: 	Integer, Integer, Integer
//	Output			: 	Integer
//	Description 	: 	Write Data Into The File From Perticular Position
//	Author			: 	Shrikant Chobi
//	Date			:	19 June 2022
//
///////////////////////////////////////////////////////////////////////////////////////


int LseekFile(int fd,int size, int from)
{
	if((fd<0) || (from>2))    return -1;
	if(UFDTArr[fd].ptrfiletable == NULL)  return -1;
	
	if((UFDTArr[fd].ptrfiletable->mode == READ) || (UFDTArr[fd].ptrfiletable->mode==READ+WRITE))
	{
		if(from == CURRENT)
		{
			if(((UFDTArr[fd].ptrfiletable->readoffset)+size) > UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) return -1;
			if(((UFDTArr[fd].ptrfiletable->readoffset)+size)<0)    return -1;
			(UFDTArr[fd].ptrfiletable->readoffset)  =  (UFDTArr[fd].ptrfiletable->readoffset) + size;
		}
		else if(from == START)
		{
			if(size > (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))   return -1;
			if(size<0)  return-1;
			(UFDTArr[fd].ptrfiletable->readoffset) = size;
		}
		else if (from == END)
		{
			if((UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)+ size > MAXFILESIZE)  return -1;
			if(((UFDTArr[fd].ptrfiletable->readoffset)+size)<0)  return -1;
			(UFDTArr[fd].ptrfiletable->readoffset)  =  (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + size;
		}
	}
	else if(UFDTArr[fd].ptrfiletable->mode == WRITE)
	{
		if(from == CURRENT)
		{
			if(((UFDTArr[fd].ptrfiletable->writeoffset) + size) >MAXFILESIZE)     return -1;
			if(((UFDTArr[fd].ptrfiletable->writeoffset)+size)<0)  return -1;
			if(((UFDTArr[fd].ptrfiletable->writeoffset)+size)>(UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
				(UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) = (UFDTArr[fd].ptrfiletable->writeoffset) + size;
            (UFDTArr[fd].ptrfiletable->writeoffset)  =  (UFDTArr[fd].ptrfiletable->writeoffset) +  size;			
		}
		else if(from== START)
		{
			if(size > MAXFILESIZE)  return -1;
			if(size < 0)  return -1;
			if(size > (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
				(UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)  =  size;
			(UFDTArr[fd].ptrfiletable->writeoffset) = size;
		}
		else if(from == END)
		{
			if((UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)+ size > MAXFILESIZE)  return-1;
			if(((UFDTArr[fd].ptrfiletable->writeoffset)+size)<0)  return -1;
			(UFDTArr[fd].ptrfiletable->writeoffset)  =  (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + size;
		}
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////
//
//	Function Name	: 	ls_file
//	Input			: 	None
//	Output			: 	None
//	Description 	: 	List Out All Existing Files Name
//	Author			: 	Shrikant Chobi
//	Date			:	19 June 2022
//
///////////////////////////////////////////////////////////////////////////////////////


void ls_file()
{
	int i = 0;
	PINODE temp = head;
	
	if(SUPERBLOCKobj.FreeInode == MAXINODE)
	{
		printf("ERROR : There are no files\n");
		return;
	}
	
	printf("\nFile Name\tInode number\tFile size\tLink count\n");
	printf("--------------------------------------------------------------------------------------\n");
	while(temp!=NULL)
	{
		if(temp->FileType != 0)
		{
			printf("%s\t\t%d\t\t%d\t\t%d\n",temp->FileName,temp->InodeNumber,temp->FileActualSize,temp->LinkCount);
		}
		temp = temp->next;
	}
	printf("---------------------------------------------------------------------------------------\n");
}

///////////////////////////////////////////////////////////////////////////////////////
//
//	Function Name	: 	fstat_file
//	Input			: 	Integer
//	Output			: 	Integer
//	Description 	: 	Display Statistical Information Of The File By Using File Descriptor
//	Author			: 	Shrikant Chobi
//	Date			:	19 June 2022
//
///////////////////////////////////////////////////////////////////////////////////////



int fstat_file(int fd)
{
	PINODE temp = head;
	int i=0;
	
	if(fd < 0)  return -1;
	
	if(UFDTArr[fd].ptrfiletable==NULL)   return -2;
	
	temp = UFDTArr[fd].ptrfiletable->ptrinode;
	
	printf("\n----------------Statistical Information about file----------------------\n");
	printf("File name : %s\n",temp->FileName);
	printf("InodeNumber: %d\n",temp->InodeNumber);
	printf("File size : %d\n",temp->FileSize);
	printf("Actual File size : %d\n",temp->FileActualSize);
	printf("Link count : %d\n",temp->LinkCount);
	printf("Reference count : %d\n",temp->ReferenceCount);
	
	if(temp->permission == 1)
		printf("File Permission : Read only\n");
	else if(temp->permission == 2)
		printf("File permission : Write \n");
	else if(temp->permission == 3)
		printf("File permission : Read & Write\n");
	printf("---------------------------------------------------------------------------------------\n\n");
	
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////
//
//	Function Name	: 	stat_file
//	Input			: 	Char*
//	Output			: 	Integer
//	Description 	: 	Display Statistical Information Of The File By Using File Name
//	Author			: 	Shrikant Chobi
//	Date			:	19 June 2022
//
///////////////////////////////////////////////////////////////////////////////////////


int stat_file(char *name)
{
	PINODE temp = head;
	int i= 0;
	
	if(name==NULL)  return -1;
	
	while(temp!=NULL)
	{
		if(strcmp(name,temp->FileName) == 0)
			break;
		temp= temp->next;
	}
	
	if(temp == NULL)   return -2;
	
	printf("\n--------------Statistical Information about file-------------\n");
	printf("File name : %s\n",temp->FileName);
	printf("InodeNumber: %d\n",temp->InodeNumber);
	printf("File size : %d\n",temp->FileSize);
	printf("Actual File size : %d\n",temp->FileActualSize);
	printf("Link count : %d\n",temp->LinkCount);
	printf("Reference count : %d\n",temp->ReferenceCount);
	
	if(temp->permission == 1)
		printf("File Permission : Read only\n");
	else if(temp->permission == 2)
		printf("File permission : Write \n");
	else if(temp->permission == 3)
		printf("File permission : Read & Write\n");
	printf("------------------------------------------------------------------------\n\n");
	
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////
//
//	Function Name	: 	truncate_File
//	Input			: 	Char*
//	Output			: 	Integer
//	Description 	: 	Delete All Data From The File
//	Author			: 	Shrikant Chobi
//	Date			:	19 June 2022
//
///////////////////////////////////////////////////////////////////////////////////////


int truncate_File(char *name)
{
	int fd = GetFDFormName(name);
	if(fd == -1)
		return -1;
	
	memset(UFDTArr[fd].ptrfiletable->ptrinode->Buffer,0,1024);
	UFDTArr[fd].ptrfiletable->readoffset = 0;
	UFDTArr[fd].ptrfiletable->writeoffset = 0;
	UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize = 0;
	
	return 0;
}

int main()
{
	char *ptr = NULL;
	int ret =0,fd =0,count = 0;
	char command[4][80],str[80],arr[1024];
	
	InitialiseSuperBlock();
	CreateDILB();
	
	while(1)
	{
		  fflush(stdin);
		  strcpy(str,"");
		
		  printf("\nCVFS : > ");
		
		fgets(str,80,stdin); // scanf("%[^'\n']s",str);
		
		count = sscanf(str,"%s %s %s %s",command[0],command[1],command[2],command[3]);
		
		     if(count == 1)
			 {
				 if(strcmp(command[0],"ls")==0)
				 {
					 ls_file();
				 }
				 else if(strcmp(command[0],"closeall")==0)
				 {
					 CloseAllFile();
					 printf("All files closed successfully \n");
					 continue;
				 }
				 else if(strcmp(command[0],"clear")==0)
				 {
					 system("cls");
					 continue;
				 }
				 else if(strcmp(command[0],"help")==0)
				 {
					 DisplayHelp();
					 continue;
				 }
				 else if(strcmp(command[0],"exit")==0)
				 {
					 printf("Terminating the Customised Virtual File System\n");
					 break;
				 }
				 else
				 {
					 printf("\nERROR : Command not found !!!\n");
					 continue;
				 }
			 }
			 else if(count == 2)
			 {
				 if(strcmp(command[0],"stat")==0)
				 {
					 ret = stat_file(command[1]);
					 if(ret == -1)
						 printf("ERROR : Incorrect parameters\n");
					 if(ret == -2)
						 printf("ERROR : There is no such file\n");
					 continue;
				 }
				 else if(strcmp(command[0],"fstat")==0)
				 {
					 ret = fstat_file(atoi(command[1]));
					 if(ret == -1)
						 printf("ERROR : Incorrect parameters\n");
					 if(ret== -2)
						 printf("ERROR : There is no such file\n");
					 continue;
				 }
				 else if(strcmp(command[0],"close")==0)
				 {
					 ret = CloseFileByName(command[1]);
					 if(ret == -1)
						 printf("ERROR : There is no such file \n");
					 continue;
				 }
				 else if(strcmp(command[0],"rm")==0)
				 {
					 ret = rm_File(command[1]);
					 if(ret == -1)
						 printf("ERROR : There is no such file\n");
					 continue;
				 }
				 else if(strcmp(command[0],"man")==0)
				 {
					 man(command[1]);
				 }
				 else if(strcmp(command[0],"write") == 0)
				 {
					 fd = GetFDFormName(command[1]);
					 if(fd == -1)
					 {
						 printf("ERROR : Incorrect parameter\n");
						 continue;
					 }
					 printf("Enter the data:\n");
					 scanf("%[^\n]",arr);
					 
					 ret = strlen(arr);
					 if (ret == 0)
					 {
						 printf("ERROR : Incorrect parameter\n");
						 continue;
					 }
					 ret = WriteFile(fd,arr,ret);
					 if(ret == -1)
						 printf("ERROR : Permission denied\n");
					 if(ret == -2)
						 printf("ERROR : There is no sufficient memory to write\n");
					 if(ret == -3)
					 	 printf("ERROR : It is not regular file\n");
					 if(ret>0)
					 	printf("!Data is written succesfully\n");
					 	continue;
					 	 
				 }
				 else if(strcmp(command[0],"truncate")==0)
			         {
				         ret = truncate_File(command[1]);
				         if(ret == -1)
					         printf("ERROR:Incorrect parameter\n");
			         }
				 else
				 {
					 printf("\nERROR : Command not found !!!\n");
					        continue;
				 }
			 }
			 else if(count == 3)
			 {
				 if(strcmp(command[0],"create")==0)
				 {
					 ret = CreateFile(command[1],atoi(command[2]));
					 if(ret >=0)
						 printf("File is successfully created with file descriptor : %d\n",ret);
					 if(ret == -1)
						 printf("ERROR : Incorrect parameters\n");
					 if(ret == -2)
						 printf("ERROR : There is no inodes\n");
					 if(ret==-3)
						 printf("ERROR : File already exists\n");
					 if(ret==-4)
						 printf("ERROR : Memory allocation failure\n");
					 continue;
				 }
				 else if(strcmp(command[0],"open")==0)
				 {
					 ret = OpeanFile(command[1],atoi(command[2]));
					 if(ret>=0)
						 printf("File is successfully opened with file descriptor: %d\n",ret);
					 if(ret==-1)
						 printf("ERROR : Incorrect parameter\n");
					 if(ret == -2)
						 printf("ERROR  : File is not present\n");
					 if(ret==-3)
						 printf("ERROR : Permission denied\n");
					 continue;
				 }
				 else if(strcmp(command[0],"read")==0)
				 {
					fd = GetFDFormName(command[1]);
                                        if(fd==-1)
					{
					   printf("ERROR : Incorrect parameter\n");
                                           continue;					   
					}
                                         ptr = (char *)malloc(sizeof(atoi(command[2]))+1);
                                         if(ptr == NULL)
					 {
						 printf("ERROR : Memory allocation failure\n");
						 continue;
					 } 
					 ret = ReadFile(fd,ptr,atoi(command[2]));
					 if(ret == -1)
						 printf("ERROR : File not existing\n");
					 if(ret == -2)
						 printf("ERROR : Permission denied\n");
					 if(ret==-3)
						 printf("ERROR : Reached at end of file\n");
					 if(ret==-4)
						 printf("ERROR : It is not regular file\n");
					 if(ret == 0)
						 printf("ERROR : File empty\n");
					 if(ret>0)
					 {
						 write(2,ptr,ret);
					 }
					 continue;
				 }
				 else
				 {
					 printf("\nERROR : Command not found !!!\n");
					 continue;
				 }
			 }
			 else if(count ==4)
			 {
				 if(strcmp(command[0],"lseek")==0)
				 {
					 fd = GetFDFormName(command[1]);
					 if(fd == -1)
					 {
						 printf("ERROR : Incorrect parameter\n");
						 continue;
					 }
					 ret = LseekFile(fd,atoi(command[2]),atoi(command[3]));
					 if(ret == -1)
					 {
						 printf("ERROR : Unable to perform lseek \n");
					 }
				 }
				 else
				 {
					 printf("\nERROR : Command not found !!!\n");
					 continue;
				 }
			 }
			 else
			 {
				 printf("\nERROR : Command not found !!!\n");
				 continue;
			 }
	}
	return 0;
}
