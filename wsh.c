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
  int fg; //foreground job or not
  job *next; //the next job in the list
};
  
typedef job *joblist;//joblist is now a list of jobs, points to the start of the list

/*
 * Parse function breaks the input into tokens, returns them one by one
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
    //printf("In loop \n");
    next = PTR_ADD(next,1);
    *counter = *counter + 1;
    
  }
  //printf("after loop: %i \n", *counter);
  
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
  //printf("Size of token: %i\n", tokenSize);

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
int builtin(char *command) {
  //if "exit" "help" "jobs" or "kill" built in function, executes the command, return 1 for now
  //else command is an executable, return 0
  if (strcmp(command, "help") == 0) {
    //  help();
    printf("Type 'help' for documentation for all built in commands supported by the Williams shell.\n\n");
    
    printf("exit - halts the shell interpretation.\n");
    printf("kill - terminates jobs given an identifier of the targeted job.\n");
    printf("jobs - displays a list of all outstanding jobs, with an identifier.\n");
    printf("cd - changes the working directory of the shell.\n");  
    return 1;
    
  } else if (strcmp(command, "jobs") == 0) {
    return 1;

  } else if (strcmp(command, "kill") == 0) {
    return 1;

  } else if (strcmp(command, "cd") == 0) {
    return 1;

  } else if(strcmp(command, "exit") == 0) {
    printf("TRY TO EXIT\n");
    exit(0);
    perror("Exit failed");
    return 1;
  }
  
  return 0;
  
}


//method to execute commands? 
//find semicolon sep commands, handling redirection
//***change name of tokenArray in main method
void executeCommand(char *tokenArray[], int tIndex, pid_t *parentIds, int *pidSize) { 
  int pidIndex = 0;
  //  int pidSize = 1;
  int bTest;
  //printf("TRY THE [%s] BUILT IN\n", tokenArray[0]);  
  if (tokenArray[0] != 0) {
    bTest = builtin(tokenArray[0]);
  }
  //printf("If I got here after calling exit, FAILED TO EXIT\n");

  if(bTest){    
    return 0;
  }


  //char *fullPath = (char*)ht_get(pathTable, tokenArray[0]);
  
  //This is a hard coded LS, here we will call a function modified from kind.c
  //to give us the file path if the command exists
  //if(fullPath != 0){ //not needed anymore
   
    
  //printf("Size of Parent list before resize is: %d\n", (int)sizeof(parentIds));
    
  if(pidIndex >= (*pidSize)){ //if pidIndex ever exceeds capacity, resize array of ids 
    (*pidSize) = 2 * (*pidSize);
      //printf("NEEDED SIZE: %d\n", sizeof(pid_t)*pidSize);
      //printf("REsize parentIds!\n");
      
      parentIds = (pid_t*)realloc(parentIds, sizeof(pid_t)*(*pidSize));
      
      //printf("Size of Parent list is: %d\n", (int)sizeof(parentIds));
    }
    
    //printf("Size of needed list is: %d\n", (int)sizeof(pid_t)*pidIndex);
    //printf("pidIndex is: %d\n", pidIndex);
	  
    pidIndex++;
    //increment the size of the parent ids array
    (*pidSize)++;
    parentIds[pidIndex-1] = fork(); //these are actually child ids
    //joblist->next->processId = fork(); 

    if(parentIds[pidIndex-1] == 0){ //child
     
      //if (!joblist->next->processId) {//child

      //too late at this point to check if 
      //if (tokenArray[0]) { //(fullPath) {
      int ret;
	//	int err = errno;
	SYSCALL(ret = execvp(tokenArray[0], tokenArray) ,"Execvp");
	
	//if(ret == -1){
	//perror("Execvp");
  
	//}
	
	//exit(1); //child exits no matter what happens 
	//printf("Return value: %i   error number: %i\n",ret, err);
	//}
    } else { //parent
      pid_t commandId = wait(0); 

      /*
      while(commandId != parentIds[pidIndex-1]) {
	commandId = wait(0);
	//remove process ids from parentIDs ANYTIME a child dies 
	//if a child that's not your own died, cross it off the list of processes 
	//ls | head starts two fg processes
	//separate fg and bg lists with counts...
	//every time you call wait if any process died, either a fg or a bg
	//*top command shows you all your processes --> use for debugging from within wsh
	//run top
	//type u
	//type 16kw6
	//shows all my processes
	//open another panic terminal session - everything done there should show up in top
	//wait command should
      }
      */
      //printf("pidSize: %d\n", (*pidSize));
      int z = 0;
      while((*pidSize) > 1){
	for(z = 0; z < pidIndex; z++){
	  if(commandId == parentIds[z]){
	    // decrease the number of parent ids, and set the parent Id, there to 0?
	    //printf("Process found \n");
	    (*pidSize)--;
	    parentIds[z] = 0;
	    commandId = wait(0);
	  }
	}
      }
      



    }	
    //} //if fullPath...
  
  //  tokenIndex = 0; //reset the array to accept a new command <-- do this in main
	

}



/*
  Whether or not we found a special character, add a 0 to the array to indicate end of the token
*/
int isSpecial(char *token) {
  //printf("isSpecial is started here!\n");  
  //printf("the one character token is: %c\n", *token);

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

  //  int pidIndex = 0;
  //int pidSize = 1;

  printf("*******Welcome! You are now running the Williams Shell*******\n(c) 2015 Juan Mena and Kelly Wang.\n->");  
  
  //Begin processing buffer
  //while there is something in the buffer continue prompting the user, read in bSize characters or up to \n or EOF
  while(buffer == fgets(buffer, bSize, stdin)){
    //printf("%s \n", buffer);
    flag = 0; //set "flag" = 0 each time you start reading the buffer 
    counter = 0; //index along the buffer we're looking at  
    tokenIndex = 0;
    
    while(counter < bSize) {    
      token = parse(buffer, &counter, bSize, &flag);
      //      printf("counter: %i\n", counter);

      //add each token to array
      tokenArray[tokenIndex] = token;
      tokenArraySize++;
      /*     if(token != 0){
	printf("token array at index %d: %s\n", tokenIndex, tokenArray[tokenIndex]);
      }
      printf("Past token 0 \n");        */    

      //First try and see if the first token is a built in
      //if(token != 0){
      //	  builtin(token);

      //}
      
      //; separated comands: if the current token is an ; then this is the end of the command
      //Execute the command, and set tokenIndex to zero
      //Or else just increment tokenIndex
      printf("-----------------------------------\nIs a special token?\n");

      //if t == "#", if == ";"...etc do different things...
      if(isSpecial(token)) {
	tokenArray[tokenIndex] = 0;	
	//printf("IN MAIN: will > be found? %d\n", *token == '>');
	//semicolon separated commands
	if(*token == ';') {
	  executeCommand(tokenArray, tokenIndex, parentIds, &sizeParentIds); 
	  perror("Failed to exit");
	  

	} else if (*token == '>') {
	  int out = dup(1); //save stdout file descriptor
	  char *target = parse(buffer, &counter, bSize, &flag); //read in the next word AFTER the >
	  //printf("\t The token after '>' is: %s\n", target);
	  //add that token to the token array
          //tokenArray[tokenIndex] = 0;//target;
          //tokenArraySize++;
	  
	  int fd;
	  //see definition at top of wsh
	  SYSCALL(fd = open(target, (O_WRONLY | O_CREAT | O_TRUNC), 0666), "Output descripton");

	  dup2(fd, 1); //set target file to receive things written to stdout
	  executeCommand(tokenArray, tokenIndex, parentIds, &sizeParentIds); //execute thing in tokenArray before the >
	  dup2(out, 1); //reconnect stdout

	}

	tokenIndex = 0; //reset the tokenIndex to begin executing the next command, if it exists?
	
      } else {
	//Command has not ended yet, therefore
	//Increment the token index.
	tokenIndex++;
	
	//if incrementing means we exceeds capacity, realloc
	//if (tokenIndex >= tokenArraySize) {
	  //tokenArraySize = 2*tokenArraySize;
	  //bSize = 2*bSize;
	  //tokenArray = (char**)realloc(tokenArray, sizeof(char*)*tokenArraySize);
	//}
      }

      //if flag is set, we have finished parsing      
      if(flag == 1){ 
	break;
      }
    } // while counter < bSize
    
    printf("\n->"); 
  }
  return 0;
}



/* Questions for Duane */
// Clarify 0 stdin, 1 stdout, 2 stderr
// :)
