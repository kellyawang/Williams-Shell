//(c)2015 Kelly Wang and Juan Mena

//This is a program that implements a Bourne-like shell from scratch called the Williams Shell. It has a few built in commands and job control similar to bash.  

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#define PTR_ADD(p,i) (((char*)p)+(i))

//A helpful definition to have whenever we're making a system call
//the 2 ()'s around call and docstring ensure that call makes an assignment if it is an operator
#define SYSCALL(call, docstring) if ((call) == -1) { perror((docstring)); exit(errno); }

//A helpful definition for debugging
#define debugPrint(args) if (debug) { fprintf(stderr, args) }

typedef struct job job; //struct job was the old type name, job is new type

struct job {
  int jobId; //id of each process used in the joblist; this is passed to kill command
  pid_t processId; //index of the process (as seen in top)
  char *description; //the command that started this job
  //  int fg; //foreground job or not
  job *next; //the next job in the list
  job *prev;// The previous job on the list
};
  
typedef job *joblist;//joblist is now a list of jobs, points to the start of the list

/*
 * Parse function breaks the input into tokens by incrementing a counter through the input, returns them one by one
 * size refers to the size of the bufffer
 */
char *parse(char *buffer, int *counter, int size, int *flag){

  int NUMBER_OF_TESTS = 9;
  char *test[] = {" ","\n",";",":","&", "#","|", ">","<"};

  int i;
  int tokenSize = 512;
  int temp = 512;

  /*
   *This is where we process the front part of the string. 
   * "next" is buffer minus the first "counter" amount of characters
   */
  char *next = PTR_ADD(buffer,*counter);
  
  /* 
   * This while loop is for ignoring all of the white space 
   * in the buffer
   * strcspn() calculates the length of the initial segment of s up to the reject character
*/
  while(*counter < size && strcspn(next, " ") == 0 ){
    
    next = PTR_ADD(next,1);
    *counter = *counter + 1;
    
  }
  /*
   *Here we check for tokens, the smallest sized token is returned
   *Because all token sizes start counting from the beginning
   *So the smallest token is the first token in the string
   */
  char *first = strndup(next, 1); 
  for(i = 0; i < NUMBER_OF_TESTS; i++) {
      if(i > 1 && strspn(first,test[i]) == 1 ){ //in case there's multiple ;'s or #'s 
	temp = 1;//strspn(next,test[i]);
      } else{
	temp = strcspn(next,test[i]);
      } 
      //temp is size of current token being processed
      if(temp < tokenSize){
	tokenSize = temp;
	
      }
    }

  if(tokenSize == 0){
    *flag = 1;
    return ";";
  }  
  *counter = tokenSize + *counter; //done processing first token, next iteration starts where we left off
  return strndup(next, tokenSize); 
  
}




/*
 * Identify and execute built in commands. If not built-in, its an executable
 */
int builtin(char *command, char *target, job *jobs) {
  //if "exit" "help" "jobs" or "kill" built in function, executes the command, return 1 for now
  //else command is an executable, return 0
  if (strcmp(command, "help") == 0) {
    printf("Type 'help' for documentation for all built in commands supported by the Williams shell.\n\n");
    printf("exit - halts the shell interpretation.\n");
    printf("kill - terminates jobs given an identifier of the targeted job.\n");
    printf("jobs - displays a list of all outstanding jobs, with an identifier.\n");
    printf("cd - changes the working directory of the shell.\n");  

    return 1;
    
  } else if (strcmp(command, "jobs") == 0) {
    job *temp = jobs->next;
    while(jobs != temp){
      printf("[%d]  %d  %s user\n", temp->jobId, temp->processId, temp->description);
      temp = temp->next;
    }
    
    return 1;
    //remember to print out jobId, processId, and report when a job is Done, or has been Killed, or has been zombie'd

  } else if (strcmp(command, "kill") == 0) {
    job *temp = jobs->next;
    
    int goal = atoi(target);
    while(jobs != temp){
      if(temp->processId == goal){
	printf("kill it\n");
	kill(temp->processId, 9);
	remove(temp);	
	break;
      }
      temp = temp->next;
    }
    
    
    return 1;

  } else if (strcmp(command, "cd") == 0) {    
    // chdir
    // getcwd
    int bSize = 512;
    char current[bSize];
    getcwd(current, bSize);

    printf("the Current Working Directory is: %s\n", current);
    printf("the Current Command is: %s\n", command);
    printf("the Current Target is: %s\n", target);
    
    int success;
    success = -1;
    printf("*********strcmp value: %d\n", strcmp(target, "~") == 0);
    if (strcmp(target, "~") == 0) { //if target is just ~
      char *home;
      home = getenv("HOME");
      printf("Home directory is: %s\n", home);
      success = chdir(home);
      perror("--Changing Directory to ~ --");

    } else if (target == '~') { //if target starts with ~ followed by something else eg: ~/Williams-Shell
      char *home;
      home = getenv("HOME");
      printf("HOME??%s\n", home);
      home = PTR_ADD(home,2);
      success = chdir(home);
      perror("--Change directory to home--");
      //target = ~/Williams-Shell/directory
      //
      success = chdir(target);
      perror("--Change directory to target--");

    } else {
      success = chdir(target);
    }

    if (success < 0) {
      printf("Error: %s is not a good directory\n", target);
      chdir(current);
    } 

    //printf("Success? %d\n", success);
    //char *new;
    //getcwd(new, bSize);
    //printf("After chdir: the New Working Directory is: %s\n", new);


    return 1;

  } else if(strcmp(command, "exit") == 0) {    
    exit(0);
    return 1;

  }
  return 0;
  
}
/*
void signal_handler(somethin somethin){
  
  
}
*/

void addJob(job *alpha, char *desc, joblist jobs, int *jid){
  if(jobs->next == jobs){
    //Job list is empty restart the count
    *jid = 1;
  }else{
    //job list has stuff so continue the count.
    *jid = *jid + 1;
  }
  //set up the new job
  alpha->jobId = *jid;
  alpha->description = desc;
  //put into list
  (jobs->next)->prev = alpha; 
  alpha->next = jobs->next;
  alpha->prev = jobs;
  jobs->next = alpha;

  printf("[%d]  %d  %s user\n", alpha->jobId, alpha->processId, alpha->description);
  
}

void removeJob(job *thing){

  (thing->prev)->next = thing->next; 
  (thing->next)->prev = thing->prev;
  printf("[%d]  terminated  %s user\n", thing->jobId, thing->description);
  free(thing);
  
}

//method to execute commands? 
//find semicolon sep commands, handling redirection
//***change name of tokenArray in main method
void executeCommand(char *tokenArray[], int tIndex, pid_t *parentIds, int *pidSize, int fg, joblist jobs, int *jid) { 
  int pidIndex = 0;

  int bTest;
  printf("in execute Command, command: %s and target: %s are passed to builtin()\n", tokenArray[0], tokenArray[1]);
  if (tokenArray[0] != 0) {
    bTest = builtin(tokenArray[0], tokenArray[1], jobs);
  }
  
  if(bTest){    
    printf("bTest = %d; found a built in\n", bTest);
    return 0;
  }
  printf("bTest = %d; did not find a built in\n", bTest);

  //This is a hard coded LS, here we will call a function modified from kind.c
  //to give us the file path if the command exists
  //if(fullPath != 0){ //not needed anymore
   
  if(pidIndex >= (*pidSize)){ //if pidIndex ever exceeds capacity, resize array of ids 
    (*pidSize) = 2 * (*pidSize);
    parentIds = (pid_t*)realloc(parentIds, sizeof(pid_t)*(*pidSize));
  }    

  if(fg){
    //increment the size of the parent ids array
    
      pidIndex++;
      (*pidSize)++;
      parentIds[pidIndex-1] = fork(); //these are actually child ids
      //printf("post fork\n");
      
      if( parentIds[pidIndex-1] == 0){ //child
	
	int ret;
	SYSCALL(ret = execvp(tokenArray[0], tokenArray) ,"Execvp");	
      }else { //parent
	
	pid_t commandId;
	job *temp = jobs->next;
	int z = 0;
	
	while((*pidSize) > 1){
	  commandId = wait(0); 
	  for(z = 0; z < pidIndex; z++){
	    if(commandId == parentIds[z]){
	      // decrease the number of parent ids, and set the parent Id, there to 0?
	      (*pidSize)--;
	   
	      parentIds[z] = 0;
	    }
	    
	  }
	  //Check through the bg jobs to see if any terminated.
	  while(temp != jobs){
	    if(temp->processId == commandId){
	   
	      removeJob(temp);
	      
	    }  
	    temp = temp->next;
	  }
	}
      }
  }else{
    job *bgJob = (job*)malloc(sizeof(job));
    bgJob->processId = fork();
    
    if( bgJob->processId == 0){ //child
	int ret;
	
	SYSCALL(ret = execvp(tokenArray[0], tokenArray) ,"Execvp");	
	dup2(13,0);
    }else{
      
      addJob(bgJob, tokenArray[0], jobs, jid);
		
    }
  }
}


/*
  Whether or not we found a special character, add a 0 to the array to indicate end of the token
*/
int isSpecial(char *token) {
  
  return (*token == '\n' || *token == ';' || *token == ':' || *token == '&' || *token == '#' || *token == '|' || *token == '<' || *token == '>' );

}



/*
 * Main method to run the shell, parse each command from the user
 * We read in and execute commands one by one as we get to them because they may modify 
 * later commands
*/
int main (int argc, char **argv) {  
  int bSize = 512;    //default buffer size
  char buffer[bSize]; //create a buffer to store strings
  char *token;  
  int counter;
  int flag = 0;

  //store tokens for current command
  char *tokenArray[bSize];
  int tokenIndex;
  int tokenArraySize = bSize;

  //store ids of each running process
  pid_t *parentIds = (pid_t*)malloc(sizeof(pid_t));
  int sizeParentIds = 1;
  //allJobs
  //Create job list
  joblist jobs = (job*)malloc(sizeof(job));
  int jid = 1;

  jobs->next = jobs;
  jobs->prev = jobs;
  jobs->jobId = 0;
  jobs->processId = 0;
  jobs->description = "init";
  
  //for redirection
  int in = dup(0); //save stdin file descriptor
  int out = dup(1); //save stdout file descriptor
  
  printf("*******Welcome! You are now running the Williams Shell*******\n(c) 2015 Juan Mena and Kelly Wang.\n->");  
  
  //Begin processing buffer
  //while there is something in the buffer continue prompting the user, read in bSize characters or up to \n or EOF
  while(buffer == fgets(buffer, bSize, stdin)){
  
    flag = 0; //set "flag" = 0 each time you start reading the buffer 
    counter = 0; //index along the buffer we're looking at  
    tokenIndex = 0;
    while(counter < bSize) {    
      token = parse(buffer, &counter, bSize, &flag);
  
      //add each token to array
      tokenArray[tokenIndex] = token;
      tokenArraySize++;
      
      //; separated comands: if the current token is an ; then this is the end of the command
      //Execute the command, and set tokenIndex to zero
      //Or else just increment tokenIndex
  

      //if t == "#", if == ";"...etc do different things...
      if(isSpecial(token)) {

	//semicolon separated commands
	if(*token == ';') {
	  tokenArray[tokenIndex] = 0;	
	  
	  executeCommand(tokenArray, tokenIndex, parentIds, &sizeParentIds, 1, jobs, &jid); 
	  printf("FOUND a semicolon, finished executing\n");
	  tokenArraySize = 0;
	  dup2(in, 0); //reconnect stdout
	  dup2(out, 1);//reconnect stdout
	  tokenIndex = 0; //reset the tokenIndex to begin executing the next command, if it exists?

	} else if (*token == '>') {
	  char *target = parse(buffer, &counter, bSize, &flag); //read in the next word AFTER the >
	  
	  if(!isSpecial(target)){
	    int fd;
	    SYSCALL(fd = open(target, (O_WRONLY | O_CREAT | O_TRUNC), 0666), "Output descripton");
	    
	    dup2(fd, 1); //set target file to receive things written to stdout

	  }else {
	    printf("%s not a valid filename \n" , target);
	  }

	}else if(*token == '<'){
	  
	  char *target = parse(buffer, &counter, bSize, &flag); //read in the next word AFTER the >
	  
	  if(!isSpecial(target)){
	    int fd;

	    SYSCALL(fd = open(target, (O_RDONLY), 0666), "Output descripton");
	 
	    dup2(fd, 0); //set target file to receive things written to stdout
	 	  
	  }else {
	    printf("%s not a valid filename \n" , target);
	  }
	}else if(*token == '&'){
	  // fg == 0 means in the back ground, fg == 1 means in the foregronud	  
	  tokenArray[tokenIndex] = 0;	
	  
	  executeCommand(tokenArray, tokenIndex, parentIds, &sizeParentIds, 0, jobs, &jid); 
	  //perror("Failed to exit");
	  dup2(in, 0); //reconnect stdout
	  dup2(out, 1); //reconnect stdout
	  tokenArraySize = 0;
	  tokenIndex = 0; //reset the tokenIndex to begin executing the next command
	  
	} else if (*token == '|') {
	  tokenArray[tokenIndex] = 0; //place a 0 to indicate end of a command
	  
	  int pipefd[2]; // create a pipe
	  pipe(pipefd);	  

	  //execute cmd1
	  dup2(pipefd[1], 1); //set files to write stdoutput to pipe
	  executeCommand(tokenArray, tokenIndex, parentIds, &sizeParentIds, 1, jobs, &jid);	  	  
	  close(pipefd[1]);

	  dup2(pipefd[0], 0); //set read end of pipe to receive things written to stdout	  
	  close(pipefd[0]);

	  //now execute cmd2 like normal
	  dup2(out, 1);//reconnect stdout
	  tokenArraySize = 0; 
	  tokenIndex = 0;      

	} else if(*token == '#'){

	  tokenArray[tokenIndex] = 0;	
	  
	  executeCommand(tokenArray, tokenIndex, parentIds, &sizeParentIds, 1, jobs, &jid); 
	  tokenArraySize = 0;
	  dup2(in, 0); //reconnect stdout
	  dup2(out, 1);//reconnect stdout
	  tokenIndex = 0; //reset the tokenIndex to begin executing the next command, if it exists?
	  flag = 1; // if # then execute and ignore everything after the #
	}
	
	//if not a special character, Command has not ended yet, therefore increment the token index.
      } else {
	//Command has not ended yet, therefore
	//Increment the token index.
	tokenIndex++;
      }

      //if flag is set, we have finished parsing      
      if(flag == 1){ 
	break;
      }
    } // while counter < bSize
    
    //Check for background jobs
    pid_t bgTemp = waitpid(-1,0,WNOHANG);
    job *jobTemp = jobs->next;
    while(bgTemp != 0 && jobs != jobTemp){
      if(bgTemp == jobTemp->processId){
	removeJob(jobTemp);

      }
      
      jobTemp = jobTemp->next;
    }
    
    printf("\n->"); 
  }
  return 0;
}

