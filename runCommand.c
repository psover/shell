//Penelope Over psover, Stefan Utamaru sualexander
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <stdio.h>

int main(int argc, char* argv[]){
	int status, i;
	struct rusage usage;
	struct timeval time;
	char* args[100];
	long int starttime, endtime;
	int getr;
	long invol, vol;
	long minflt, majflt;
	long int utime, stime;

	gettimeofday(&time, NULL);
	starttime = (time.tv_sec * 1000) + (time.tv_usec / 1000);
	if(argc<=1){
    	printf("Not enough command line arguments\n");
    	return;}

	if(fork()!=0){
		//parent
		wait(&status);
		getr = getrusage(RUSAGE_CHILDREN, &usage);if (getr == 0){
			utime = (usage.ru_utime.tv_sec * 1000) + (usage.ru_utime.tv_usec / 1000);
			stime = (usage.ru_stime.tv_sec * 1000) + (usage.ru_stime.tv_usec / 1000);
			invol = usage.ru_nivcsw;
			vol = usage.ru_nvcsw;
			minflt = usage.ru_minflt;
			majflt = usage.ru_majflt;
		} else {
			printf("error for getrusage\n");
			return 0;
		}
		gettimeofday(&time, NULL);
		endtime = (time.tv_sec * 1000) + (time.tv_usec / 1000);

		// wall clock time for the command to execute in milliseconds
		endtime = endtime - starttime;

		printf("\nElapsed wall-clock time: %ld", endtime);
		printf("\nCPU User time: %ld\nCPU System time: %ld", utime, stime);
		printf("\nNumber of times the process was preempted involuntarily: %ld", invol);
		printf("\nNumber of times the process gave up voluntarily: %ld", vol);
		printf("\nNumber of page faults: %ld", majflt);
		printf("\nNumber of page faults that could be satisfied using unreclaimed pages: %ld\n", minflt - majflt);
	}
	else{
  		for(i=1; i<argc; i++){
    		args[i-1] = argv[i];
    		args[i] = (char*) 0;}
		execvp(argv[1], args);
	}
}

