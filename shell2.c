#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <sys/types.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/times.h>
#include <sys/time.h>
#include <fcntl.h>

#define ARG_SIZE 32 
#define STR_SIZE 128
#define TASKNUM 100

struct timeval start;
struct timeval end;
char* argv[ARG_SIZE];
int argc=0;
int bgtask =0;
struct rusage new_rusage;
struct rusage old_rusage;
struct backgroundinfo{
  pid_t pid;
  char* command;
  char commandline[STR_SIZE];
};
struct backgroundinfo bgi[TASKNUM];

void printstats();
void childprocess(char* argv[], int argc);
int background();
void checkforchild();

void  main(){
  int i, c, l, n, m;
  int endpid, chpid;
  int j = 0;
  int end = 0;
  pid_t pid;
  int status;
  char str [STR_SIZE];
  char* result;
  new_rusage = old_rusage = (struct rusage) {0};

  while(end != 1){
    for(c=0; c<ARG_SIZE; c++){
      argv[c] =(char*) 0;}
    checkforchild();
    write(1, "==> ", 3);
    //read(0, str, STR_SIZE);
	if(fgets(str, STR_SIZE, stdin)==NULL){
		printf("End of File ...\n");
		end = 1;
		return;
	}
    for(i=0; str[i]!= '\n'; i++){}
    str[++i] = '\0';
    result = strtok(str, " \n:");
    for(argc = 0; ((result !=NULL)&&(argc <= 32)); argc++){
      argv[argc] = result;
      result = NULL;
      result = strtok(NULL, " \n:");
    }
    
    if(argc==0){
      printf("Not enough commandline arguments\n");
    }
    else if(strncmp(argv[argc-1], "&", STR_SIZE)==0){
      argv[argc-1] = "\0";
      end = background();}
    else if(strncmp(argv[0],"exit", STR_SIZE)==0){
      printf("waiting for all tasks to finish\n");
      while((endpid=wait(&status))>=0){
	for(n =1; n<=bgtask; n++){
	  if(bgi[n].command != NULL){
	    if(endpid == bgi[n].pid){
	      printf("background task complete.\n[%d] %d %s has completed\n", n, endpid, bgi[n].commandline);
	      printstats();
	      bgi[n].command = NULL;
	    }
	  }
	}
      }
      end = 1;
      return;
    }
    else if(strncmp(argv[0],"jobs", STR_SIZE)==0){
      for(l=1; l<=bgtask; l++){
	if(bgi[l].command != NULL){
	  printf("[%d] %d ", l, bgi[l].pid);
	  printf("%s\n", bgi[l].commandline);}
      }
    }
    else if(strncmp(argv[0], "cd", STR_SIZE)==0){ 
      if(chdir(argv[1])==0){
	printf("sucess\n");
	system("pwd");}
      else{
	printf("error\n");}
    }
    else{
      pid = fork();
      if(pid == 0){
	end = 1;
	childprocess(argv, argc);}
      else{
	gettimeofday(&start, NULL);
	if((chpid = wait3(&status, 0, &new_rusage))>0){
	  gettimeofday((struct timeval *)end, NULL); 
	  for(m =1; m<=bgtask; m++){
	    if((bgi[m].command != NULL) &&(chpid == bgi[m].pid)){
		printf("background task complete.\n[%d] %d %s has completed\n", m, endpid, bgi[m].commandline);
		printstats();
		bgi[m].command = NULL;
	      }
	    }
	  }
	gettimeofday((struct timeval *)end, NULL);
	printf("forground\n");
	printstats();
      }
    }
  }
}
void printstats(){
  long usertime = new_rusage.ru_utime.tv_sec*1000 + new_rusage.ru_utime.tv_usec/1000;
  long cputime = new_rusage.ru_stime.tv_sec*1000 + new_rusage.ru_stime.tv_usec/1000;
  printf("%ld number of page faults\n", new_rusage.ru_majflt);
  printf("%ld number of page faults resolved internal\n", new_rusage.ru_minflt);
  printf("%ld number of voluntary context switch\n", new_rusage.ru_nvcsw);
  printf("%ld number of involuntary context switch\n", new_rusage.ru_nivcsw);
  printf("%ld milliseconds of user cpu time\n", usertime);
  printf("%ld milliseconds of system cpu time\n", cputime);
  double starttime = start.tv_sec*1000000 + start.tv_usec;
  double endtime = end.tv_sec*1000000 + end.tv_usec;
  printf("%lf clocktime in milliseconds\n", (endtime-starttime)/1000);
  old_rusage = new_rusage;
 
  start = end = (struct timeval) {0};
}
void childprocess(char* argv[], int argc){
  int i;
  char* args[ARG_SIZE];

  for(i=0; i < ARG_SIZE; i++){
    args[i] = NULL;}
  for(i=0; i<argc; i++){
    args[i] = argv[i];
    args[i+1] = (char*)0;}
  int res = execvp(argv[0], args);
  printf("Error %d Invaild Command", res);
}
int background(){
  int bgstatus;
  int res;
  int morechildren = 0;
  pid_t pidbg = fork();
  
  if(pidbg == 0){
    childprocess(argv, argc);
    res=1;
  }
  else{ 
    bgtask++;
    strncpy((char*)bgi[bgtask].commandline, argv[0], sizeof(argv[0])+1);
    bgi[bgtask].command =(char*) 1;
    bgi[bgtask].pid =pidbg;
     
    while(morechildren!=1){
      gettimeofday(&start, NULL);
      if(wait3(&bgstatus, WNOHANG, &new_rusage)){
	gettimeofday(&end, NULL);  
	printstats();
      }
      else{
	morechildren = 1;}
    }
    res = 0;
  }
  return res;
}
void checkforchild(){
  int chstatus;
  int endpid =0;
  int m;
  endpid = wait3(&chstatus, WNOHANG, &new_rusage);
  while(endpid>0){
    for(m =1; m<=bgtask; m++){
      printf("%s\n", bgi[m].commandline);
      if(bgi[m].command != NULL){
	if(endpid == bgi[m].pid){
	  printf("[%d] %d %s has completed\n", m, endpid, bgi[m].commandline);
	  printstats();
	  bgi[m].command = NULL;
	}
      }
    }
    endpid = wait3(&chstatus, WNOHANG, &new_rusage);
  }
}
