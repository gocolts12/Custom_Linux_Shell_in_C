#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <setjmp.h>
#include <string.h>
#include <stdint.h>

#include <sys/types.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

void signalHandler(int signal)
{
  if (signal == SIGINT)
    printf("SIGINT handled");
  else if (signal == SIGTSTP)
    printf("SIGSTP handled");
}

int main()
{

  if (signal(SIGINT, signalHandler) == SIG_ERR)
  {
    printf("SIGINT FAILURE");
  }
  if (signal(SIGTSTP, signalHandler) == SIG_ERR)
  {
    printf("SIGTSTP FAILURE");
  }



  while (fputs("361> ", stdout) > 0)
    {

      char *command[50];
      char input[500];
      int numOfCommands = 0;
      int redirection = -1;
      char filename[50];
      char mainCommand[50];

      int i = 0;
      if(numOfCommands != 0)
      {
        for (i = 0; i < numOfCommands; i++)
        {
          command[i] = '\0';
        }
        //input = '\0';
        numOfCommands = 0;
      }
      fgets(input, sizeof(input), stdin);

      if (input[0] == '\n'){
        continue;
      }
      command[0] = strtok(input, " \n");
      numOfCommands = 1;

      if (strcmp(input, "exit") == 0)
      {
        exit(1);
      }

      while ((command[numOfCommands] = strtok(NULL, " \n")) != NULL)
      {
        numOfCommands++;
      }
      // printf("num = %d", numOfCommands);
      // printf("contains %s",command[numOfCommands] );
      // for (i = 0; i < numOfCommands; i++)
      // {
      //   printf("command[%d] = %s", i, command[i]);
      // }
      // printf("number of commands = %d", numOfCommands);
      fflush(stdout);
      if (numOfCommands > 1 && strcmp(command[numOfCommands-2],">") == 0)
      {
        redirection = 0;
        strcpy(filename, command[numOfCommands-1]);
        command[numOfCommands - 1] = NULL;
        command[numOfCommands - 2] = NULL;
      }
      else if (numOfCommands > 1 && strcmp(command[numOfCommands-2],">>") == 0)
      {
        redirection = 1;
        strcpy(filename, command[numOfCommands-1]);
        //strcpy(command[numOfCommands - 1], "\0");
        //strcpy(command[numOfCommands - 2], "\0");
        command[numOfCommands - 1] = NULL;
        command[numOfCommands - 2] = NULL;
      }
      else if (numOfCommands > 1 && strcmp(command[numOfCommands-2],"<") == 0)
      {
        redirection = 2;
        strcpy(filename, command[numOfCommands-1]);
        strcpy(mainCommand, command[0]);
        command[numOfCommands - 1] = NULL;
        command[numOfCommands - 2] = NULL;
      }

      pid_t childPid;
      if((childPid = fork()) < 0){
        perror("fork error");
        exit(-1);
      }
      if (childPid == 0)
      {
        //child

        // > redirect
        if (redirection == 0)
        {
          int filedesc = open(filename, O_RDWR | O_TRUNC | O_CREAT, 0777);
          dup2(filedesc, 1);
        }
        // >> redirect
        else if (redirection == 1)
        {
          int filedesc = open(filename, O_RDWR | O_APPEND | O_CREAT, 0777);
          dup2(filedesc, 1);
        }
        // < redirect
        else if (redirection == 2)
        {

          FILE *ptr = fopen(filename, "r");
          if (ptr == NULL)
          {
            puts("Couldn't open file");
            exit(-7);
          }
          fgets(input, sizeof(input), ptr);


          numOfCommands = 1;
          command[numOfCommands] = strtok(input, " \n");
          numOfCommands++;

          while ((command[numOfCommands] = strtok(NULL, " \n")) != NULL)
          {
            numOfCommands++;
          }

          strcpy(command[0], mainCommand);
        }

        if ( execvp(command[0], command) < 0 ) {
            printf("Unable to process command\nError: %s\n", strerror(errno));
            exit(-2);
        }

      }
      else
      {
        //parent
        int status;
        printf("child PID is %d \n", childPid);
        wait(&status);
        printf("My child has died with status %d. :(\n", WEXITSTATUS(status));
      }
    }
}
