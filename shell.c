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
struct timeval start;
struct timeval end;
char* argv[ARG_SIZE];
int argc=0;
struct rusage new_rusage;
struct rusage old_rusage;
void parentprocess();
void childprocess(char* argv[], int argc);

void  main(){
  int i, c;
  int j = 0;
  int end = 0;
  pid_t pid;
  int status;
  char str [STR_SIZE];
  char* result;
  char* str2;
  new_rusage = old_rusage = (struct rusage) {0};

  while(end != 1){
    for(c=0; c<ARG_SIZE; c++){
      argv[c] =(char*) 0;}
    write(1, "==>", 3);
    read(0, str, STR_SIZE);
    //write(1, str, STR_SIZE);
    for(i=0; str[i]!= '\n'; i++){}
    str[++i] = '\0';
    //str2 = str;
    result = strtok(str, " \n");
    //write(1, result, sizeof(result));
    for(argc = 0; ((result !=NULL)&&(argc <= 32)); argc++){
      argv[argc] = result;
      result = NULL;
      //printf("argv %d: %s\n", argc, argv[argc]);
      result = strtok(NULL, " \n");
    }
    
    if(argc==0){
      printf("Not enough command line arguments\n");
    }
    else if(strncmp(argv[0],"exit", STR_SIZE)==0){
      end = 1;
      return;}
    else if(strncmp(argv[0], "cd", STR_SIZE)==0){ 
      //printf("%s string\n", argv[1]);
      if(chdir(argv[1])==0){
	printf("sucess\n");
	system("pwd");}
      else{
	printf("error\n");}
    }
    else{
      for(i=0; i<argc; i++){
	printf("%d argument: %s\n", i, argv[i]);}
      pid = fork();
      if(pid == 0){
	end = 1;
	childprocess(argv, argc);}
      else{
	gettimeofday(&start, NULL);
	wait(&status);
	gettimeofday(&end, NULL);    
	parentprocess();
      }
    }
  }
}
void parentprocess(){

  int result = getrusage(RUSAGE_CHILDREN, &new_rusage);

  new_rusage.ru_utime.tv_sec -= old_rusage.ru_utime.tv_sec;
  new_rusage.ru_stime.tv_sec -= old_rusage.ru_stime.tv_sec;
  new_rusage.ru_majflt -= old_rusage.ru_majflt;
  new_rusage.ru_minflt -= old_rusage.ru_minflt;
  new_rusage.ru_nvcsw -= old_rusage.ru_nvcsw;
  new_rusage.ru_nivcsw -=  old_rusage.ru_nivcsw; 

  if(result != -1){
    long usertime = new_rusage.ru_utime.tv_sec*1000 + new_rusage.ru_utime.tv_usec/1000;
    long cputime = new_rusage.ru_stime.tv_sec*1000 + new_rusage.ru_stime.tv_usec/1000;
    printf("%ld number of page faults\n", new_rusage.ru_majflt);
    printf("%ld number of page faults resolved internal\n", new_rusage.ru_minflt);
    printf("%ld number of voluntary context switch\n", new_rusage.ru_nvcsw);
    printf("%ld number of involuntary context switch\n", new_rusage.ru_nivcsw);
    printf("%ld milliseconds of user cpu time\n", usertime);
    printf("%ld milliseconds of system cpu time\n", cputime);
  } 
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
  execvp(argv[0], args);   
}
