//
// Student: Lior Erenreich
//
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>

/// FIELDS AND OBJECTS DECLARATIONS:

int typeOfInstruction, i, waiting, waitForPid, waitForSecondPid, fd;
pid_t pid, secondPid;
char** firstInstruction;
char** secondInstruction;
char* filename;

///FUNCTIONS DECLARATIONS:

/**
 *          ********OBLIGATORY********
 * prepare() is called before the first call to process_arglist().
 * @purpose to do all the preparations needed in order to process_arglist() to be executed properly.
 * @return: 0 if succeeded, any other returned values indicate an error.
 */
int prepare(void);


/**
 *         ********OBLIGATORY********
 * @param arglist - array of parsed instruction.
 * @param count - the amount of valid words that the command is built of.
 * @return 0 if succeeded, any other returned values indicate an error.
 */
int process_arglist(int count, char** arglist);


/**
 *         ********OBLIGATORY********
 * @purpose to make sure all data is cleaned up before exiting the program.
 * @return 0 if succeeded, any other returned values indicate an error.
 */
int finalize(void);


/**
 * @param arglist - array of parsed instruction.
 * @param count - the amount of valid words that the command is built of.
 * @purpose indicates which type of instruction it is:
 * @return 0 -> regular instruction (the program waits until it is finished to proceed to the next one).
 * @return 1 -> background instruction (the program executes it and doesn't wait for it to finish in order to proceed to the next one).
 * @return 2 -> piping instruction (the program executes both instructions concurrently, piping the first instruction's output as an input to the second one, and waits until both are finished to proceed to the next one).
 * @return 3 -> redirecting instruction (the program's output is printed to the file after the ">" sign, the program waits until it is finished to proceed to the next one)
 */
int determineInstructionType(int count, char** arglist);


/**
 * @param count - the amount of valid words that the command is built of.
 * @param arglist - array of parsed instruction.
 * @return 0 if succeeded, any other returned values indicate an error.
 */
void executeRegularInstruction(int count, char** arglist);


/**
 * @param handle - integer which indicates whether SIGINTs are supposed to be considered or ignored.
 * @param if handle==0 -> ignore SIGINT
 * @param if handle==1 -> handle SIGINT
 * @purpose setting the sigaction accordingly.
 */
 void handleSIGINT(int handle);


 /**
  * @param count - the amount of valid words that the command is built of.
  * @param arglist - array of parsed instruction.
  * @purpose to execute a background instruction.
  * @return 0 if succeeded, any other returned values indicate an error.
  */
void executeBackgroundInstruction(int count, char **arglist);


/**
 * @purpose to fork and than ensuring that forking didn't failed.
 * @return the pid (pid_t).
 */
pid_t forkProcessWithCheck(void);


/**
 * @param arglist - array of parsed instructions.
 * @param handle - an integer that indicates whether the child process should handle or ignore SIGINTs.
 * @purpose to fork and than ensuring that forking didn't failed and setting how the child process will handle SIGINT.
 * @return the pid (pid_t).
 */
pid_t forkProcessWithExecvpAndCheck(char** arglist,int handle);

/**
 * @param to execute a regular instruction.
 * @param count - the amount of valid words that the command is built of.
 * @param arglist - array of parsed instruction.
 * @return 0 if succeeded, any other returned values indicate an error.
 */
void executeRedirectingInstruction(int count, char **arglist);


/**
 * @purpose to execute a pipe type of instruction.
 * @param count - the amount of valid words that the command is built of.
 * @param arglist - array of parsed instruction.
 * @return 0 if succeeded, any other returned values indicate an error.
 */
void executePipingInstruction(int count, char **arglist);




/**
 * @purpose this function is called after execvp, which means that if the program reached it, there was an error while execvp.
 */
void printErrorOfExecvp(void);


/**
 * @param _FileHandleSrc
 * @param _FileHandleDst
 * @purpose generating dup2() and checking if succeeded, if not - prints a matching error.
 */
void dup2check(int _FileHandleSrc, int _FileHandleDst);


/**
 *
 */
void removeAllZombies(int signalNumber);


///IMPLEMENTATIONS:

int prepare(void){
    return 0;
}
int process_arglist(int count, char** arglist){
    handleSIGINT(0);
    typeOfInstruction = determineInstructionType(count, arglist);
    if (typeOfInstruction==0){
        executeRegularInstruction(count, arglist);
    }
    else if (typeOfInstruction==1){
        executeBackgroundInstruction(count, arglist);
    }
    else if (typeOfInstruction==2){
        executePipingInstruction(count, arglist);
    }
    else if (typeOfInstruction==3){
        executeRedirectingInstruction(count, arglist);
    }
    else{
        printf("There is an error in determining the instruction's type.");
        exit(1);
    }
    return 1;
}
int finalize(void){
    return 0;
}

int determineInstructionType(int count, char** arglist){
    for(i=0;i<count;i++){
        if ((i==count-1)&&(strcmp(arglist[i],"&")==0)){
            arglist[i]=NULL;
            return 1;
        }
        else if(strcmp(arglist[i],"|")==0){
            return 2;
        }
        else if(strcmp(arglist[i],">")==0){
            return 3;
        }
        else{
            continue;
        }
    }
    return 0;
}

void executeRegularInstruction(int count, char** arglist){
    pid = forkProcessWithExecvpAndCheck(arglist,1);
    i=0;
    //pid is not -1 or 0 => it is the parent process.
    if((pid!=0)&&(pid!=-1)){
        waiting = waitpid(pid, &i, WUNTRACED);
        if((waiting<0) && (errno!=ECHILD) && (errno!=EINTR)){
            fprintf(stderr, "%s", "An error was occurred while parent process was waiting to child.");
            exit(1);
        }
    }
}
void executeBackgroundInstruction(int count, char **arglist) {
    pid = forkProcessWithExecvpAndCheck(arglist, 0);
    struct sigaction removeZombies={
            .sa_handler = removeAllZombies,
            .sa_flags = SA_NOCLDSTOP | SA_RESTART
    };
    memset(&removeZombies, 0, sizeof(removeZombies));
    if(sigaction(SIGCHLD,&removeZombies,NULL)==-1){
        fprintf(stderr, "%s", "SIGCHLD Error has occurred.");
        exit(1);
    }
}
void executePipingInstruction(int count, char **arglist) {
    ///first the program needs to parse the command line into two instructions:
    i=0;
    while(arglist[i]!=NULL){
        if(strcmp(arglist[i],"|")==0){
            arglist[i]=NULL;
            break;
        }
        i++;
    }
    int j;
    int df[2];
    if(pipe(df) == -1){
        fprintf(stderr, "%s","Failed piping.");
        close(df[0]);
        close(df[1]);
        exit(1);
    }
    pid = fork();
    //when folk is resulted with -1 it means that an error was occurred.
    if (pid == -1){
        fprintf(stderr, "%s", "An error was occurred while forking.");
        close(df[0]);
        close(df[1]);
        exit(1);
    }
    ///this is the child process of the first fork ->
    ///it should see SIGINTs and it's output (stdout) should be the writing side of the pipe.
    ///it will execute the first instruction and put it's output into the writing side of the pipe.
    else if (pid==0){
        handleSIGINT(1);
        close(df[0]);
        dup2check(df[1],1);
        close(df[1]);
        execvp(arglist[0],arglist);
        printErrorOfExecvp();
    }
        ///(pid!=0 && pid!=-1) --> means this is the parent process.
    else{
        secondPid = fork();
        //when folk is resulted with -1 it means that an error was occurred.
        if (secondPid == -1){
            fprintf(stderr, "%s", "An error was occurred while forking.");
            close(df[0]);
            close(df[1]);
            exit(1);
        }
        ///this is the son process of the second fork ->
        ///it should see SIGINTs and it's input (stdin) should be the reading side of the pipe.
        ///it will read the pipe and put it as an input for the second instruction and will execute it.
        else if (secondPid==0){
            handleSIGINT(1);
            close(df[1]);
            dup2check(df[0],0);
            close(df[0]);
            execvp(arglist[i+1], arglist+(i+1));
            printErrorOfExecvp();
        }
        ///this is the parent process
        else{
            close(df[0]);
            close(df[1]);
            waitForPid=waitpid(pid, &j, WUNTRACED);
            waitForSecondPid=waitpid(secondPid, &j, WUNTRACED);
            if ((waitForPid<0)||(waitForSecondPid<0)){
                if((errno!=EINTR)&&(errno!=ECHILD)){
                    fprintf(stderr, "%s","waiting failed.");
                    exit(1);
                }
            }
        }
    }
}
void executeRedirectingInstruction(int count, char **arglist) {
    ///first the program needs to parse the command line into an instruction and a filename:
    i=0;
    while(arglist[i]!=NULL){
        if(strcmp(arglist[i],">")==0){
            arglist[i]=NULL;
            filename=arglist[i+1];
            break;
        }
        i++;
    }
    fd = open(filename,O_WRONLY | O_CREAT,0777);
    pid = forkProcessWithCheck();
    if (pid==0){
        dup2check(fd, STDOUT_FILENO);
        execvp(arglist[0],arglist);
        printErrorOfExecvp();
    }
    close(fd);
    i=0;
    //pid is not -1 or 0 => it is the parent process.
    if((pid!=0)&&(pid!=-1)){
        waiting = waitpid(pid, &i, WUNTRACED);
        if((waiting<0) && (errno!=ECHILD) && (errno!=EINTR)){
            fprintf(stderr, "%s", "An error was occurred while parent process was waiting to child.");
            exit(1);
        }
    }
}

void removeAllZombies(int signalNumber){
    while((i=waitpid(-1,NULL,WNOHANG))>0){
    }
    if (pid<0){
        if(errno!=ECHILD){
            fprintf(stderr,"%s","failed removing zombies.");
            exit(1);
        }
    }
}

pid_t forkProcessWithCheck(){
    pid = fork();
    //when folk is resulted with -1 it means that an error was occurred.
    if (pid == -1){
        fprintf(stderr, "%s", "An error was occurred while forking.");
        exit(1);
    }
    return pid;
}

pid_t forkProcessWithExecvpAndCheck(char** arglist,int handle){
    pid = forkProcessWithCheck();
    //if pid is 0 it means this is a child process.
    if (pid==0){
        handleSIGINT(handle);
        execvp(arglist[0],arglist);
        printErrorOfExecvp();
    }
    return pid;
}

void handleSIGINT(int handle){
    struct sigaction sig= {
            .sa_flags = SA_RESTART
    };
    if (handle){
        sig.sa_handler = SIG_DFL;
    }
    else{
        sig.sa_handler = SIG_IGN;
    }
    if (sigaction(SIGINT, &sig, NULL) == -1){
        fprintf(stderr, "%s", "An error was occurred while SIGINT.");
        exit(1);
    }
}

void printErrorOfExecvp(){
    printf("execvp failed.");
    exit(1);
}
void dup2check(int _FileHandleSrc, int _FileHandleDst){
    if(dup2(_FileHandleSrc,_FileHandleDst)==-1){
        //fprintf("Error while dup2. ", "%s", strerror(errno));
        fprintf(stderr, "%s", "Error while dup2. ");
        exit(1);
    }
}
/*
 * compiling command line:
 * gcc -O3 -D_POSIX_C_SOURCE=200809 -Wall -std=c11 shell.c myshell.c
 */