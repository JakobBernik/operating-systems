#include <wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>

typedef struct process_{
	int pid;
	int ppid;
	int threads;
	int fd;
	char state;
	char *name;
}process;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ sys
void sys(const char* path){
	FILE *version,*swaps,*modules;
	int pot = strlen(path);
	char *verzijaP =(char *) malloc(pot+9);
	char *swapiP =(char *) malloc(pot+7);
	char *moduliP =(char *) malloc(pot+9); 	
	strcpy(verzijaP,path);
	strcat(verzijaP,"/version");
	strcpy(swapiP,path);	
	strcat(swapiP,"/swaps");
	strcpy(moduliP,path);
	strcat(moduliP,"/modules");
	version = fopen(verzijaP,"r");
	swaps = fopen(swapiP,"r");
	modules = fopen(moduliP,"r");
	int fileNum = 1;
	int koncaj = 0;
	int fin = 0;
	int numOfModules = 0;
	char c;
	char buff[1024];
	if(version==NULL || swaps == NULL || modules == NULL) koncaj = 1;
	while(koncaj == 0){	
		if(fileNum == 1){ // version
			fscanf(version,"%s",buff);
			if(strcmp(buff,"version") == 0){
				fscanf(version,"%s",buff);
				if(fin == 0){
					printf("Linux: %s\n",buff);
					fin = 1;
				}
				else{
					printf("gcc: %s\n",buff);
					fileNum = 2;
				}
			}
		}else if(fileNum == 2){ // swaps
			fscanf(swaps,"%s",buff);
			if(strcmp(buff,"Priority") == 0){
				fscanf(swaps,"%s",buff);
				printf("Swap: %s\n",buff);
				fileNum = 3;
			}	
		}else if(fileNum == 3){ // modules
			c = getc(modules);		
			if(c == '\n') numOfModules+=1;
			else if(c == EOF){
				printf("Modules: %d\n",numOfModules);
				fileNum = 0;
			}
		}else koncaj = 1;		
	}
	free(verzijaP);
	free(swapiP);
	free(moduliP);
	fclose(version);
	fclose(swaps);
	fclose(modules);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ pids
int getNumOfFiles(const char *path){
	DIR *map = opendir(path);
	int numOfFiles = 0;
	while(readdir(map) != NULL){
		numOfFiles += 1;	
	}
	closedir(map);
return numOfFiles;
}

int comparer(const void *p,const void *d){
	int *pp =(int *)p;
	int *dd =(int *)d;	
return (*pp - *dd);
}


int *pids(const char *path, int fileNum){
	DIR *mapa = opendir(path);
	struct dirent *d;
	int numOfFiles = fileNum;
	int i = 0;	
	int *fileList = malloc(numOfFiles*sizeof(int));
	while( (d = readdir(mapa)) != NULL){
		fileList[i] = atoi(d->d_name);	
		i+=1;
	}
	qsort(fileList,numOfFiles,sizeof(int),comparer);
	closedir(mapa);
return fileList;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ name

int compareProcess(const void *p1,const void *p2){
	process *process1 = (process *) p1;
	process *process2 = (process *) p2;
	int first = strcasecmp(process1->name,process2->name);
	if(first == 0) return (process1->pid - process2->pid);
	else return first;
}

char *getName(const char *path,int pid){
	char buff[20];
	char *name;
	sprintf(buff,"%d",pid);
	char newPath[strlen(path)+strlen(buff)+1];
	strcpy(newPath,path);
	strcat(newPath,"/");
	strcat(newPath,buff);
	strcat(newPath,"/status");
	FILE *status =fopen(newPath,"r");
	if(status != NULL){
		int koncaj = 0;
		while(koncaj==0){
			fscanf(status,"%s",buff);
			if(strcmp(buff,"Name:") == 0){
				fscanf(status,"%s",buff);
				name =(char *) malloc((strlen(buff)+1)*sizeof(char));
				strcpy(name,buff);
				koncaj = 1;					
			}	
		}
	}
	fclose(status);
return name;
}


process *names(const char *path,int fileNum,int *pids,int sort){
	int numOfFiles = fileNum;
	int *fileList = pids;
	int i = 0;
	process *procesi =(process *) malloc(numOfFiles*sizeof(process));
	while(i < numOfFiles){
		process nov;
		nov.pid = fileList[i];
		nov.name = getName(path,nov.pid);
		memcpy(&procesi[i],&nov,sizeof(process));
		i+=1;
	}
	if(sort == 1){
		qsort(procesi,numOfFiles,sizeof(process),compareProcess);
	}
return procesi;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ps + psext

int getThreads(const char *path,int pid){
	char buff[20];
	int threads;
	sprintf(buff,"%d",pid);
	char newPath[strlen(path)+strlen(buff)+1];
	strcpy(newPath,path);
	strcat(newPath,"/");
	strcat(newPath,buff);
	strcat(newPath,"/stat");
	FILE *stat =fopen(newPath,"r");
	if(stat != NULL){
		int koncaj = 0;
		int i = 0;
		while(koncaj==0){
			fscanf(stat,"%s",buff);
			i += 1;
			if(i == 20){
				threads = atoi(buff);
				koncaj = 1;					
			}		
		}
	}
	fclose(stat);
return threads;
}

int getFd(const char *path,int pid){
	char buff[20];
	int fds = 0;
	sprintf(buff,"%d",pid);
	char newPath[strlen(path)+strlen(buff)+1];
	strcpy(newPath,path);
	strcat(newPath,"/");
	strcat(newPath,buff);
	strcat(newPath,"/fd");
	DIR *fd = opendir(newPath);
	if(fd != NULL){
		while(readdir(fd) != NULL){
		fds += 1;	
		}
	}
	fds -= 2;
	if(fds < 0){
		fds = 0;
	}
	closedir(fd);
return fds;
}

void getstuff(const char *path,process *proces,int ext){
	char buff[20];
	int ppid;
	char state;
	sprintf(buff,"%d",proces->pid);
	char newPath[strlen(path)+strlen(buff)+1];
	strcpy(newPath,path);
	strcat(newPath,"/");
	strcat(newPath,buff);
	strcat(newPath,"/status");
	FILE *status =fopen(newPath,"r");
	if(status != NULL){
		int koncaj = 0;
		while(koncaj == 0){
			fscanf(status,"%s",buff);
			if(strcmp(buff,"State:") == 0){
				fscanf(status,"%s",buff);
				proces->state = buff[0];
			}
			else if(strcmp(buff,"PPid:") == 0){
				fscanf(status,"%s",buff);
				proces->ppid = atoi(buff);
				koncaj = 1;					
			}	
		}
	}
	fclose(status);
	if(ext == 1){
	proces -> threads = getThreads(path,proces->pid);
	proces -> fd = getFd(path,proces->pid);
	}
}

void *psE(const char *path,process *procesi,int fileNum,int ext){
	
	int i = 0;
	while(i < fileNum){
		if(ext == 1) getstuff(path,&procesi[i],1);
		else getstuff(path,&procesi[i],0);
		i += 1;
	}
}

void Ancestors(process *procesi,int *ancestors,int ppid,int numOfFiles,int *indexOfAdding){
	int tmp = *indexOfAdding;
	int i = 0;
	while(i < numOfFiles){
		if(procesi[i].ppid == ppid){
			ancestors[*indexOfAdding] = procesi[i].pid;
			*indexOfAdding += 1;		
		}
		i++;
	}
	i = tmp;
	int j = *indexOfAdding;
	while(i < j){
		Ancestors(procesi,ancestors,ancestors[i],numOfFiles,indexOfAdding);
		i += 1;
	}		
}

int isInAncestors(int *ancestors,int ppid,int numOfAncestors){
	int i = 0;	
	while(i < numOfAncestors){
		if(ancestors[i] == ppid){
		return 0;		
		}
		i += 1;	
	}
return 1;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ main
int main(int argc,char* argv[]){
	
	char *akcija = argv[1];
	char *def = "/proc"; 
	if(strcmp( akcija,"sys") == 0 ){

		if(argc < 3) sys(def);
		else sys(argv[2]);

	}else if( strcmp(akcija,"pids") == 0 ){
		
		const char *path;
		if(argc < 3) path = def;
		else path = argv[2];
		int numOfFiles = getNumOfFiles(path);		
		int *fileList = pids(path,numOfFiles);
		int i =0;
		while(i < numOfFiles){
			if(fileList[i] > 0){
				printf("%d\n",fileList[i]);
			}
		i+=1;
		}
		free(fileList);

	}else if( strcmp(akcija,"names") == 0 ){

		const char *path;
		if(argc < 3) path = def;
		else path = argv[2];
		int numOfFiles = getNumOfFiles(path);		
		int *fileList = pids(path,numOfFiles);
		int i = 0;
		while(fileList[i]==0){
			i+=1;	
		}
		numOfFiles -= i;
		process *procesi = names(path,numOfFiles,&fileList[i],1);
		i = 0;
		while(i < numOfFiles){
			printf("%d %s\n",procesi[i].pid,procesi[i].name);	
			i+=1;
		}
		free(fileList);
		free(procesi);

	}else if( strcmp(akcija,"ps") == 0 || strcmp(akcija,"psext") == 0){
		
		const char *path = def;
		if(argc >= 3 && atoi(argv[2]) == 0 ){
			path = argv[2];
		} 
		int numOfFiles = getNumOfFiles(path);		
		int *fileList = pids(path,numOfFiles);
		int specificPid = 0;
		if(argc == 4) specificPid = atoi(argv[3]);
		else if(argc == 3){
			if(atoi(argv[2]) != 0){
				specificPid = atoi(argv[2]);			
			}
		} 
		int ext = 0;
		if(strcmp(akcija,"psext") == 0) ext = 1;		
		int i = 0;
		while(fileList[i]==0){
			i+=1;	
		}
		numOfFiles -= i;
		process *procesi = names(path,numOfFiles,&fileList[i],0);
		psE(path,procesi,numOfFiles,ext);
		if(ext == 1){
			printf("%5s %5s %6s %6s %6s %s\n", "PID", "PPID", "STANJE", "#NITI", "#DAT.", "IME");
			if(specificPid != 0){
				int *ancestors =(int *) malloc(numOfFiles*sizeof(int));
				int *indx =(int *) malloc(sizeof(int));
				*indx = 0;
				Ancestors(procesi,ancestors,specificPid,numOfFiles,indx);				
				i = 0;
				while(i < numOfFiles){
					if(procesi[i].pid == specificPid){
							printf("%5d %5d %6c %6d %6d %s\n", procesi[i].pid, procesi[i].ppid, procesi[i].state, procesi[i].threads,procesi[i].fd, procesi[i].name);											
					}
					else if(isInAncestors(ancestors,procesi[i].ppid,*indx) == 0 || procesi[i].ppid == specificPid){
							printf("%5d %5d %6c %6d %6d %s\n", procesi[i].pid, procesi[i].ppid, procesi[i].state, procesi[i].threads,procesi[i].fd, procesi[i].name);
					}
				i += 1;
				}
				free(indx);
				free(ancestors);
			}else{
				i = 0;			
				while(i < numOfFiles){
					printf("%5d %5d %6c %6d %6d %s\n", procesi[i].pid, procesi[i].ppid, procesi[i].state, procesi[i].threads,procesi[i].fd, procesi[i].name);
					i+=1;
				}
			}
		}else{
			printf("%5s %5s %6s %s\n", "PID", "PPID", "STANJE", "IME");
			if(specificPid != 0){
				int *ancestors =(int *) malloc(numOfFiles*sizeof(int));
				int *indx =(int *) malloc(sizeof(int));
				*indx = 0;
				Ancestors(procesi,ancestors,specificPid,numOfFiles,indx);	
				i = 0;
				while(i < numOfFiles){
					if(procesi[i].pid == specificPid){
							printf("%5d %5d %6c %s\n", procesi[i].pid, procesi[i].ppid, procesi[i].state, procesi[i].name);					
					}
					else if(isInAncestors(ancestors,procesi[i].ppid,*indx) == 0 || procesi[i].ppid == specificPid){
							printf("%5d %5d %6c %s\n", procesi[i].pid, procesi[i].ppid, procesi[i].state, procesi[i].name);			
					}
				i += 1;
				}
				free(indx);
				free(ancestors);
			}else{
				i = 0;			
				while(i < numOfFiles){
					printf("%5d %5d %6c %s\n", procesi[i].pid, procesi[i].ppid, procesi[i].state, procesi[i].name);	
					i+=1;
				}
			}
		}
		free(fileList);
		free(procesi);
	}

return 0;
}

