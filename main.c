#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include "token.h"
#include "command.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#define BUFFER MAX_NUM_TOKENS

int main()
{
	//Takes shell to ignore
	//CTRL-C, CTRL-\, CTRL-Z
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);


	char prompt[32]="%";//Reconfigurable prompt
	char find[] = "/";//find last occurance of '/'
	char currentDir[PATH_MAX];
	char newpath[PATH_MAX];
	int numCommands=0;//Number of numbers in commandArray
	int n=1;
	int jobType;

	pid_t pid;
	int status;

	//Stores the command line
	char input[BUFFER];

	
	//Stores the command line tokens
 	//After be tokenised 
	char *tokenArray[BUFFER];
	
	//Creates 100 command structs
	Command commandArray[MAX_NUM_COMMANDS];
	

	printf("\033[2J\033[1;1H");
	//Loops until user exits
	//Displays prompt, takes in command line
	//Gets tokens and sets command array
	//Structure taken from Online Resource
	//"Notes on Implementation of Shell Project"
	//Original Author Hong Xie
	while(1)
	{
		//Initialises all members of array to null
		initialiseTokenArray(tokenArray);
		//Initialises all commands
		initialiseCommandArray(commandArray);
		//From "Notes on Implementation"
		//Original Author Hong Xie
		//Used so the slow system calles wont be
		//interrupted
		char* inputP=NULL;
		int again=1;

		printf("%s", prompt);
		while(again)
		{
			again=0;
			inputP=fgets(input, BUFFER, stdin);
			if(inputP==NULL)
			{
				if(errno==EINTR)
				{
					again=1;
				}
			}
		}
		//Tokenises command line
		tokenise(input, tokenArray);
		//Converts tokens into the command struct
		numCommands=separateCommands(tokenArray, commandArray);
		
		for(int i=0;i<numCommands;++i)
		{
	
			if(strcmp(commandArray[i].argv[0],"exit")==0)
			{
				exit(0);
			}
			else if(strcmp(commandArray[i].argv[0],"pwd")==0)
			{
				char cwd[128];
				printf("Current Path: %s\n", getcwd(cwd,128));
				continue;
			}
			else if(strcmp(commandArray[i].argv[0], "cd")==0)
			{
				if(commandArray[i].argc == 2)
				{
					chdir(commandArray[i].argv[1]);
					continue;
				}
				else if(commandArray[i].argc == 1)
				{
					char cwd[128];
					if(strcmp(getcwd(cwd,128), getenv("HOME"))==0)
					{
						printf("Current directory is home directory");
						continue;
					}
					else
					{
						chdir(getenv("HOME"));
						continue;
					}
				}
			}
			else if (strcmp(commandArray[i].argv[0], "cd..")==0)
			{
				if(getcwd(currentDir, sizeof(currentDir))!=NULL)
				{
					for(int i = 0; i < 1; i++)
					{
						char *position_ptr = strrchr(currentDir, find[i]);
						int r_position = (position_ptr == NULL ? - : postion_ptr - currentDir);	
						strncpy(newpath, currentDir, r_position);
						chdir(newpath);
			}
			else if(strcmp(commandArray[i].argv[0],"cwd")==0)
			{
				chdir(commandArray[i].argv[1]);
				continue;
			}
			else if (strcmp(commandArray[i].argv[0],"prompt")==0)
			{
				if(strlen(commandArray[i].argv[1])>31)
				{
					printf("Invalid prompt; Too many character\n");
					continue;
				}
				strcpy(prompt, commandArray[i].argv[1]);
				continue;
			}
			
			int more=1;
			while(more)
			{
				pid =waitpid(-1, &status, WNOHANG);
				if(pid<=0)
				{
					more=0;
				}
			}
		
			
			jobType=checkJobType(&commandArray[i]);

			if(jobType==1)
			{
				pid_t pid2;
				int p[2];
				int np=0;
				while(strcmp(&commandArray[i].commandSuffix, "|")==0)
				{
					i++;
					np++;
				}
				i-=np;
				if(pipe(p)<0)
				{
					perror("Error in piping\n");
					exit(1);
				}
				if((pid=fork())<0)
				{
					perror("Error in forking\n");
					exit(1);
				}
				if(pid==0)
				{
					dup2(p[1], STDOUT_FILENO);
					close(p[0]);
					close(p[1]);

					execvp(commandArray[i].argv[0],commandArray[i].argv);
					perror("Error in execvp");
					exit(1);
				}
				else if(pid>0)
				{
					while(strcmp(&commandArray[i].commandSuffix, "|")==0)
					{
						i++;
						int fd=-1;
						
						if(commandArray[i].stdout_file!=NULL)
						{
							fd= open(commandArray[i].stdout_file,O_WRONLY|O_CREAT, 0766);
						}

						if((pid2=fork())<0)
						{
							perror("Error in forking\n");
							exit(1);
						}
						if(pid2==0)
						{
							
							if(fd!=-1)
							{
								dup2(fd, STDOUT_FILENO);
								close(p[1]);
								close(p[0]);
								execvp(commandArray[i].argv[0], commandArray[i].argv);

							}
							dup2(p[0], STDIN_FILENO);
							close(p[1]);
							close(p[0]);
							
							execvp(commandArray[i].argv[0], commandArray[i].argv);
							perror("Error in execvp\n");
							exit(1);
						}
					}
				}
				if(pid>0)
				{
					close(p[0]);
					close(p[1]);
					while(np>0)
					{
						wait(&status);
						--np;
					}
				}
			}
			else if(jobType==2)
			{
			
				int fd;
				fd=open(commandArray[i].stdin_file, O_RDONLY|O_CREAT, 0766);

				if((pid=fork())<0)
				{
					perror("Error in forking");
					exit(1);
				}
				if(pid==0)
				{
					dup2(fd, STDIN_FILENO);
					execvp(commandArray[i].argv[0], commandArray[i].argv);
					perror("Error in execvp");
					exit(1);
				}

			}
			else if(jobType==3)
			{
				int fd;
				fd= open(commandArray[i].stdout_file,O_WRONLY|O_CREAT, 0766);
				if((pid=fork())<0)
				{
					perror("Error in forking");
					exit(1);
				}
				if(pid==0)
				{
					dup2(fd, STDOUT_FILENO);
					execvp(commandArray[i].argv[0], commandArray[i].argv);
					perror("Error in execvp");
					exit(1);
				}

			}
			else
			{
				if((pid=fork())<0)
				{
					perror("Error in forking\n");
				        exit(1);
				}
				if(pid==0)
				{
					execvp((commandArray[i].argv[0]),((commandArray[i].argv)));
				        perror("Error in execvp\n");
				        exit(1);
				}
			}
			if(strcmp(&(commandArray[i].commandSuffix),"&")==0)
			{
				continue;
			}
			else
			{
				wait(&status);
			}
		}
	}
	return 0;
}
