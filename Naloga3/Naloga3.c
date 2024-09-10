
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<sys/wait.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

#define MAX_TOKENS 32

char name[64];
char line[128];
int line_size = 128;
char* tokens[MAX_TOKENS];
int token_count;
int last_status; 
void RunMysh();


int main(int argc, char* args[]){
	strcpy(name,"mysh");
	int iSt = isatty(0);	
	RunMysh(iSt);
return 0;
}

int Name(){
	if(token_count == 2){
		strcpy(name,tokens[1]);
	}
	else{
		printf("%s\n",name);
		
	}
return 0;
}

int Help(){

	printf("------UPORABA %s :\nname ime - nastavi ime lupine, če imena ne podamo, izpiše ime lupine,\n\
help - izpiše spisek podprtih ukazov, format izpisa je po vaši želji - se ne preverja avtomatsko,\n\
status - izpiše izhodni status zadnjega (v ospredju) izvedenega ukaza,\n\
exit status - konča lupino s podanim izhodnim statusom,\n\
print args... - izpiše podane argumente na standardni izhod (brez končnega skoka v novo vrstico),\n\
echo args... - kot print, le da izpiše še skok v novo vrstico,\n\
pid - izpiše pid procesa (kot $BASHPID),\n\
ppid - izpiše pid starša.\n\
dirchange imenik - zamenjava trenutnega delovnega imenika, če imenika ne podamo, skoči na /,\n\
dirwhere - izpis trenutnega delovnega imenika,\n\
dirmake imenik - ustvarjanje podanega imenika,\n\
dirremove imenik - brisanje podanega imenika,\n\
dirlist imenik - preprost izpis vsebine imenika (le imena datotek, ločena z dvema presledkoma), če imena ne podamo, se privzame trenutni delovni imenik.\n",name);
return 0;	
}

int status(){
	printf("%d\n",last_status);
return 0;
}

int print(){	
	int i = 1;
	while(i < token_count-1){
		printf("%s",tokens[i]);
		i++;
	}
	printf("%s",tokens[i]);
return 0;
}

int echo(){
	int i = 1;
	while(i < token_count-1){
		printf("%s ",tokens[i]);
		i++;
	}
	printf("%s\n",tokens[i]);
return 0;
}

int pid(){
	if(token_count > 1){ // če so podani dodatni argumenti 
		int fp = -1;
		int fileIs = 0;
		int background = 0;
		char str[20];
		if(*tokens[token_count-1]=='&'){ // zadnji argument signalizira izvajanje v ozadju
			background = 1;
			token_count --;
		}
		if(*tokens[token_count-1] == '>'){ // drugi argument file v katerega pisemo
			fileIs = 1;
			fp = open(tokens[1]+1, O_WRONLY| O_TRUNC | O_CREAT, 00777); // descriptor fila z dovoljenji				
		}
		if(background){
			int p = fork();
			if(p == 0){					
				exit(0);
			}else{
				wait(NULL);
				if(fp != -1){ // če smo lahko odprli datoteko
					sprintf(str,"%d",p);
					write(fp,str,strlen(str));
					close(fp);
				}else{
					if(fileIs){
						printf("%s: %s\n", tokens[0], strerror(errno));
			 			return errno;
					}else{
						printf("%s>%d\n\n",name,p);
					}
				}					
			}
		}
		else{ // ni izvajanja v ozadju
			if(fp != -1){ // če smo lahko odprli datoteko
				sprintf(str,"%d",(int)getpid());
				write(fp,str,strlen(str));
				close(fp);
			}else{
				if(fileIs){
					printf("%s: %s\n", tokens[0], strerror(errno));
			 		return errno;
				}else{
					printf("%d\n",getpid());
				}
			}
		}
	}else{ // podan ni noben dodaten argument
		printf("%d\n",getpid());
	}
return 0;
}

int ppid(){
	if(token_count > 1){ // če so podani dodatni argumenti 
		int fp = -1;
		int fileIs = 0;
		int background = 0;
		char str[20];
		if(*tokens[token_count-1]=='&'){ // zadnji argument signalizira izvajanje v ozadju
			background = 1;
			token_count --;
		}
		if(*tokens[token_count-1] == '>'){ // drugi argument file v katerega pisemo
			fileIs = 1;
			fp = open(tokens[1]+1, O_WRONLY| O_TRUNC | O_CREAT, 00777); // descriptor fila z dovoljenji				
		}
		if(background){
			if(fork() == 0){					
				exit(0);
			}else{
				wait(NULL);
				if(fp != -1){ // če smo lahko odprli datoteko
					sprintf(str,"%d",(int)getpid());
					write(fp,str,strlen(str));
					close(fp);
				}else{
					if(fileIs){
						printf("%s: %s\n", tokens[0], strerror(errno));
			 			return errno;
					}else{
						printf("%s>%d\n\n",name,getpid());
					}
				}					
			}
		}
		else{ // ni izvajanja v ozadju
			if(fp != -1){ // če smo lahko odprli datoteko
				sprintf(str,"%d",(int)getppid());
				write(fp,str,strlen(str));
				close(fp);
			}else{
				if(fileIs){
					printf("%s: %s\n", tokens[0], strerror(errno));
			 		return errno;
				}else{
					printf("%d\n",getppid());
				}
			}
		}
	}else{ // podan ni noben dodaten argument
		printf("%d\n",getppid());
	}
return 0;
}

int dirchange(){
	char path[64];
	if(token_count == 1){
		strcpy(path,"/");
	}else{
		strcpy(path,tokens[1]);
	}
	if(chdir(path) == -1){
		printf("%s: %s\n", tokens[0], strerror(errno));
		return errno;
	}
return 0;
}

int dirwhere(){
	if(fork() == 0){
		execl("/bin/pwd","pwd",NULL);
		exit(0);
	}else{
		wait(NULL);
	}
return 0;
}

int dirmake(){
	if(mkdir(tokens[1],ACCESSPERMS) == -1){
		printf("%s: %s\n", tokens[0], strerror(errno));
		return errno;	
	}	
return 0;
}

int dirremove(){
	int p = fork();
	if(p == 0){
		int fp = open("/dev/null",O_WRONLY);
		dup2(fp,2);
		close(2);
		int e = 0;
	 	e = execl("/bin/rm","rm","-r",tokens[1],NULL);
		close(fp);
		if( e == -1){
			exit(errno);	
		}else{
			exit(0);
		}
	}else{
		int wstatus = 0;	
		wait(&wstatus);
		if(wstatus != 0){
			errno = 2;
			printf("%s: %s\n", tokens[0], strerror(errno));
			return errno;
		}
	}
return 0;
}

int dirlist(){
	char cwd[64];
	DIR* d = NULL;	
	if(token_count == 1){
		getcwd(cwd,sizeof(cwd));
		d = opendir(cwd);
	}else{
		d = opendir(tokens[1]);
		
	}
	if(d == NULL){
		printf("%s: %s\n", tokens[0], strerror(errno));
		return errno;
	}
	struct dirent* drnt;
	while((drnt = readdir(d)) != NULL){
			printf("%s  ",drnt->d_name);
	}
	printf("\n");
return 0;
}

int Rename(){
	if(rename(tokens[1],tokens[2]) != 0){
		printf("%s: %s\n", tokens[0], strerror(errno));
		return errno;
	}

return 0;
}

int tokenize(){
	token_count = 0;
	int index = 0;
	int start = 0;
	while(line[index] != '\n'){
		if(isspace(line[index])){
			line[index] = '\0';
			index++;
			while(isspace(line[index])){
				if(index == line_size-1 || line[index] == '\n'){
					if(start == 0){
						return 0;				
					}else{
						line[index] = '\0';
						return 1;
					}										
				}
				index++;			
			}
			if(line[index] == '#' && start == 0){
				return 0;
			}else if(start == 0){
				start = 1;
				if(line[index] == '\"'){
					index++;
					tokens[token_count] = &line[index];
					token_count+=1;
					while(line[index] != '\"'){
						index++;
					}
					index++;
					if(line[index] == '\n'){
						line[index] = '\0';
						return 1;
					}else{
						line[index] = '\0';		
					}
				}else{
					tokens[token_count] = &line[index];
					token_count+=1; 
				}
				
			}else{
				if(line[index] == '\"'){
					index++;
					tokens[token_count] = &line[index];
					token_count+=1;
					while(line[index] != '\"'){
						index++;
					}
					line[index] = '\0';
				}else{
					tokens[token_count] = &line[index];
					token_count+=1;
				}
			} 
		}else if(start == 0){
			start = 1;
			tokens[token_count] = &line[index];
			token_count+=1;		
		}
		index++;	
	}
	line[index] = '\0';
	return 1;
}

void printIt(){
	int i = 0;
	while(i < token_count){
		printf("Token %d: %s\n",i,tokens[i]);				
		i++;
	}

}

void RunMysh(int iSt){	
	while(1){
		if(iSt){
		printf("%s>",name);
		}
		if(fgets(line,line_size,stdin) == NULL){
			exit(0);
		}if(line[0] == '\n'){
			continue;	
		}else{			
			int result = tokenize();
			//printIt();
			if(result){
				if(strcmp(tokens[0],"name") == 0){
					last_status = Name();
				}
				else if(strcmp(tokens[0],"help") == 0){
					last_status = Help();
				}
				else if(strcmp(tokens[0],"status") == 0){
					last_status = status();
				}
				else if(strcmp(tokens[0],"print") == 0){
					last_status = print();
				}
				else if(strcmp(tokens[0],"echo") == 0){
					last_status = echo();
				}
				else if(strcmp(tokens[0],"pid") == 0){
					last_status = pid();
				}
				else if(strcmp(tokens[0],"ppid") == 0){
					last_status = ppid();
				}
				else if(strcmp(tokens[0],"dirchange") == 0){
					last_status = dirchange();
				}
				else if(strcmp(tokens[0],"dirwhere") == 0){
					last_status = dirwhere();
				}
				else if(strcmp(tokens[0],"dirmake") == 0){
					last_status = dirmake();
				}
				else if(strcmp(tokens[0],"dirremove") == 0){
					last_status = dirremove();
				}
				else if(strcmp(tokens[0],"dirlist") == 0){
					last_status = dirlist();
				}
				else if(strcmp(tokens[0],"rename") == 0){
					last_status = Rename();
				}
				else if(strcmp(tokens[0],"exit") == 0){
					if(token_count == 2){
						exit(atoi(tokens[1]));
					}
					else{
						exit(0);
					}
				}				
			}
		}
	}
}	
