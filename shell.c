#include<sys/wait.h>
#include<stdio.h>
#include<errno.h>
#include<fcntl.h>
#include<sys/types.h>
//#include<unistd.h>
#include<proc/readproc.h>
#include<limits.h>
#include<sys/utsname.h>
#include<sys/types.h>
#include<pwd.h>
//#include<string.h>
//#include<stdio.h>
//#include<stdlib.h>
#include"mypwd.h"
#include"myecho.h"
#include"mycd.h"
//#include"retstat.h"
#ifndef _STDIO_H
#include<stdio.h>
#endif
#ifndef _UNISTD_H
#include<unistd.h>
#endif
#ifndef _STRING_H
#include<string.h>
#endif
//#ifndef _STDLIB_H
#include<stdlib.h>
//#endif

pid_t stack_pid[1000], curr_id = -2;
int pos[1000];
char name[1000][100], currname[100];
int top = 0;

static void hdl(int sig, siginfo_t *siginfo, void * context)
{
	if (sig == SIGCHLD) {
		int i;
		char buffer[100];
		for(i=0; i<top; i++)
		{
			if(siginfo->si_pid == stack_pid[i])
			{
				if(((int)siginfo->si_errno) > 0)
				{
					//itoa((int)siginfo->si_pid, buffer, 10);
					char msg[1000] = "\n";
					strcat(msg, "[");
					snprintf (buffer, sizeof(buffer), "%d", pos[i]);
					strcat(msg, buffer);
					strcat(msg, "] ");
					snprintf (buffer, sizeof(buffer), "%d",(int)siginfo->si_pid);
					strcat(msg, buffer);
					strcat(msg, " ");
					strcat(msg, name[i]);
					strcat(msg, " ");
					strcat(msg, strerror(siginfo->si_errno));
					strcat(msg, "\n");
					write(STDOUT_FILENO, msg, strlen(msg));
					//fprintf(stderr, "%d %s had error %s\n", (int)siginfo->si_pid, name[i], strerror(siginfo->si_errno));
					//printf("abcdef\n");
				}
				else
				{
					//itoa((int)siginfo->si_pid, buffer, 10);
					char msg[1000] = "\n";
					strcat(msg, "[");
					snprintf (buffer, sizeof(buffer), "%d", pos[i]);
					strcat(msg, buffer);
					strcat(msg, "] ");
					snprintf (buffer, sizeof(buffer), "%d",(int)siginfo->si_pid);
					strcat(msg, buffer);
					strcat(msg, " ");
					strcat(msg, name[i]);
					strcat(msg, " exited normally\n");
					write(STDOUT_FILENO, msg, strlen(msg));
					//fprintf(stderr, "\n%d %s exited normally\n", (int)siginfo->si_pid, name[i]);
					//printf("over\n");
				}
				stack_pid[i] = -2;
				break;
			}
		}
	}
	else if (sig == SIGINT)
	{
		//stack_pid[top++] = siginfo->si_pid;
		//strcpy(name[top-1], currname);
		//printf("%d\n", curr_id);
		if(curr_id != -2) {
			kill(curr_id, SIGINT);
			printf("\n");
			curr_id = -2;
		}
		else{
			printf("\n");
		}
	}
}

int redirect_out(char * myargs[100], int a, int flag_redirect_out)
{
	int loop, oldpos=0, out = 1;
	if (flag_redirect_out == 1)
	{
		for(loop = 0; loop < a; loop++)
		{
			if(strcmp(myargs[loop], ">") == 0)
				break;
		}
		out = open(myargs[loop+1], O_CREAT|O_TRUNC|O_RDWR, 0777);
	}
	else
	{
		for(loop = 0; loop < a; loop++)
		{
			if (strcmp(myargs[loop], ">>") == 0)
				break;
		}
		out = open(myargs[loop+1], O_CREAT|O_APPEND|O_RDWR, 0777);
	}

	if (out == -1)
	{
		perror("error opening file");
		return 1;
	}
	int save_out = dup(fileno(stdout));
	int dupper = dup2(out, fileno(stdout));
	if(dupper == -1)
	{
		perror("error in redirect");
		return 1;
	}
	pid_t forking = fork();
	if (forking == -1)
	{
		perror("fork failed");
		return 1;
	}
	else if (forking == 0)
	{
		char * argstoexec[100] = {NULL};
		int loop1;
		//printf("ERRRRR\n");
		for(loop1 = 0; loop1 < loop; loop1++)
		{
			//printf("%s\n", myargs[loop1]);
			argstoexec[loop1] = myargs[loop1];
		}
		argstoexec[loop] = '\0';
		int success = execvp(argstoexec[0], argstoexec);
		//printf("exec failed");
		if(success < 0)
		{
			perror("error in execvp");
			return 1;
		}
	}
	else
	{
		int statval;
		int wc = waitpid(forking, &statval, 0);
		fflush(stdout);
		close(out);
		dup2(save_out, fileno(stdout));
		close(save_out);
		if(wc < 0)
			perror(NULL);

	}
	return 0;
}

int redirect_in(char * myargs[100], int a)
{
	int loop = 0;
	for (loop = 0; loop < a; loop ++)
	{
		if (strcmp(myargs[loop], "<") == 0)
			break;
	}
	//printf("%s\n", myargs[loop+1]);
	int in = open(myargs[loop+1], O_RDONLY);
	if(in == -1)
	{
		perror("error in opening");
		//cont_flag = 1;
		return 1;
	}
	int save_in = dup(fileno(stdin));
	//printf("%d %d %d\n", fileno(stdin), in, save_in);
	int dupper = dup2(in, fileno(stdin));
	if (dupper == -1)
	{
		perror("cannot redirect");
		return 1;
	}
	pid_t forking = fork();
	if (forking == -1)
	{
		perror("error in fork");
		return 1;
	}
	else if(forking == 0)		//child process
	{
		char * argstoexec[100] = {NULL};
		int loop1;
		//printf("ERRORRR!!\n");
		for(loop1 = 0; loop1 < loop; loop1++)
		{
			//printf("%s\n", myargs[loop1]);
			argstoexec[loop1] = myargs[loop1];
		}
		argstoexec[loop] = '\0';
		int success = execvp(argstoexec[0], argstoexec);
		//printf("exec failed");
		if(success < 0)
		{
			perror("error in execvp");
			return 1;
		}
	}
	else
	{
		int statval;
		int wc = waitpid(forking, &statval, 0);
		fflush(stdin);
		close(in);
		dup2(save_in, fileno(stdin));
		close(save_in);
		if(wc < 0) {
			perror(NULL);
			return 1;
		}
	}
	return 0;
}


int redirect_in_out(char * myargs[100], int a, int flag_redirect_out)
{
	int loop, rein, reout;
    for (loop = 0; loop < a; loop ++)
    {
    	if (strcmp(myargs[loop], "<") == 0)
    		rein = loop + 1;
    	if (strcmp(myargs[loop], ">") == 0 || strcmp(myargs[loop], ">>") == 0)
    	{
    		reout = loop + 1;
    		break;
    	}
    }
    int in = open(myargs[rein], O_RDONLY);
    int out;
    if (flag_redirect_out == 1)
    	out = open(myargs[reout], O_CREAT|O_TRUNC|O_RDWR, 0700);
    else if (flag_redirect_out == 2)
    	out = open(myargs[reout], O_CREAT|O_APPEND|O_RDWR, 0700);
    if(in == -1 || out == -1) {
    	perror("error in opening file");
    	return 1;
    }
    int save_in = dup(fileno(stdin));
    int save_out = dup(fileno(stdout));
    if (dup2(in, fileno(stdin)) == -1 || dup2(out, fileno(stdout)) == -1) {
    	perror("error in redirect");
    	return 1;
    }
    int forking = fork();
    if (forking == -1) {
    	perror("error in fork");
    	return 1;
    }
    else if (forking == 0) {
    	char * argstoexec[100] = {NULL};
		int loop1;
		for(loop1 = 0; loop1 < rein - 1; loop1++)
		{
			argstoexec[loop1] = myargs[loop1];
		}
		argstoexec[loop] = '\0';
		int success = execvp(argstoexec[0], argstoexec);
		if(success < 0)
		{
			perror("error in execvp");
			return 1;
		}
    }
    else {
    	int statval;
    	int wc = waitpid(forking, &statval, 0);
		fflush(stdin);
		fflush(stdout);
		close(in);
		close(out);
		dup2(save_in, fileno(stdin));
		dup2(save_out, fileno(stdout));
		close(save_in);
		close(save_out);
		if(wc < 0) {
			perror(NULL);
			return 1;
		}
    }
    return 0;
}

void Close(int fd) {
	if(close(fd) == -1) {
		perror("close");
		//return 1;
	}
	//return 0;
}

void REdirect(int oldfd, int newfd) {
	if (oldfd != newfd){
		if (dup2(oldfd, newfd) != -1) {
			Close(oldfd);
			//return 0;
		}
		else {
			perror("redirect");
			//return 1;
		}
	}
	//return 0;
}

void piping(char * myargs[100], int a, int nextpos, int currpos, int in_fd){
	if (myargs[nextpos] == '\0') {
		//last command in pipe
		REdirect(in_fd, STDIN_FILENO);
		char * argstoexec[100];
		int i, j;
		for(i=currpos, j=0; i < a; i++, j++) {
			argstoexec[j] = myargs[i];
		}
		argstoexec[j] = '\0';
		int forking = fork();
		if (forking == -1) {
			perror("fork");
			//return;
		}
		else if(forking == 0) {
			int success = execvp(argstoexec[0], argstoexec);
			if(success == -1) {
				perror("execvp");
				//return ;
			}
		}
		else {
			int statval;
    		int wc = waitpid(forking, &statval, WUNTRACED);
    		//fflush(stdin);
			//fflush(stdout);
			//__fpurge(stdout);
			//__fpurge(stdin);
			//close(STDIN_FILENO);
			//printf("%d\n", in_fd);
			//dup2(in_fd, STDIN_FILENO);
			close(in_fd);
		}
	}
	else {
		int fd[2];
		if(pipe(fd) == -1) {
			perror("pipe");
			//return ;
		}
		int forking = fork();
		if (forking == -1) {
			perror("fork");
			//return ;
		}
		else if(forking == 0) {
			Close(fd[0]);
			REdirect(in_fd, STDIN_FILENO);
			REdirect(fd[1], STDOUT_FILENO);
			char * argstoexec[100] = {NULL};
			int i, j;
			for(i=currpos, j=0; i<nextpos-1; i++, j++) {
				argstoexec[j] = myargs[i];
			}
			argstoexec[j] = '\0';
			int success = execvp(argstoexec[0], argstoexec);
			if(success == -1){
				perror("execvp");
				//return ;
			}
		}
		else {
			int statval;
			int wc = waitpid(forking, &statval, 0);
			Close(fd[1]);
			Close(in_fd);
			fflush(stdout);
			fflush(stdin);
			int i;
			for(i = nextpos; i<a ; i++){
				if (strcmp(myargs[i], "|") == 0) {
					break;
				}
			}
			piping(myargs, a, i+1, nextpos, fd[0]);
		}
	}
	//return ;
}

int main()
{
	struct sigaction act;
	memset(&act, '\0', sizeof(act));
	sigemptyset(&act.sa_mask);
	act.sa_sigaction = &hdl;
	act.sa_flags = SA_SIGINFO;
	if(sigaction(SIGCHLD, &act, NULL) < 0)
		perror("sigaction");
	if(sigaction(SIGINT, &act, NULL) < 0)
		perror("sigaction");
	//get prompt
	struct passwd *passwd;
	passwd =  getpwuid(getuid());
	//printf("%d\n", (int)passwd);
	char sysName[1000]={'\0'}, homeDir[1000]={'\0'}, currDir[1000]={'\0'}, input[10000]={'\0'}, input1[10000]={'\0'}, input2[10000]={'\0'};
	gethostname(sysName, sizeof(sysName));
	//set homedir
	getcwd(homeDir, sizeof(homeDir));
	int lenHome = strlen(homeDir), lenCwd;
	strcpy(input, "");
	strcpy(input1, "");
	int flag_bg =0, cont_flag=0, pipeset=0;
	while(strcmp(input1, "exit") != 0)
	{
		//find current directory and print accordingly
		flag_bg=0;
		cont_flag = 0;
		fflush(stdout);
		fflush(stdin);
		getcwd(currDir, sizeof(currDir));
		lenCwd = strlen(currDir);
		if(lenCwd >= lenHome)
		{
			int flag = 1, i;
			for(i=0; i<lenHome; i++)
				if(currDir[i] != homeDir[i])
				{
					flag=0;
					break;
				}
			if (flag == 0)
				printf("%s@%s:%s/> ", passwd->pw_name, sysName, currDir);
			else
			{
				printf("%s@%s:~/", passwd->pw_name, sysName);
				for(i = lenHome+1; i<lenCwd; i++)
					printf("%c", currDir[i]);
				printf("> ");
			}
		}
		else
			printf("%s@%s:%s> ", passwd->pw_name, sysName, currDir);
		//get input for command(s)
		//if (pipeset == 0){
			strcpy(input, "");
			strcpy(input1, "");
			fgets(input, sizeof(input), stdin);
		//}
		/*else {
			strcpy(input, "pipe is set");
			fgets(input, sizeof(input),stdin);
		}*/
		//printf("%d\n", calue);
		int i, x=1;
		if(strcmp(input, "") != 0)
		{
			for(i=0; input[i] != '\n'; i++);		//remove trailing '\n' generated by fgets
			input[i]='\0';
		}
		else {
			printf("\n");
			exit(1);
		}

		//parsing the input
		strcpy(input2, input);
		char * args[100] = {NULL}, *ab=input2;
		args[0] = ab;
		while(*ab!='\0')
		{
			if(*ab == ';')				//parsing with ';'
			{
				*ab = '\0';
				int z=0;
				for(z=1; *(ab+z)==' ' || *(ab+z) == '\t'; z++)	//removing and handling tabs and spaces after ;
				*(ab+z) = '\0';
				args[x++]=ab+z;
				ab = ab+z;
			}
			ab++;
		}
		args[x] = '\0';

		//handling command(s)
		x=0;
		while(args[x++]!='\0')
		{
			char * myargs[100] = {NULL}, *abcd=args[x-1];		//input has spaces, input1 has '\0' in place of spaces
			int a = 1, flag_redirect_out = 0, flag_pipe = 0, flag_redirect_in = 0, oldpipe = 0;
			myargs[0] = abcd;
			while(*abcd != '\0')
			{
				if(*abcd == '&')
					flag_bg += 1;
				if((*abcd == '\n' || *abcd == ' ' || *abcd == '\t' || *abcd == '&') && *(abcd-1)!= '\\')
				{
					*abcd = '\0';
					if(*(abcd+1)!='\0')
						myargs[a++] = abcd+1;
				}
				if((*abcd == '<'))		//input redirect
				{
					flag_redirect_in = 1;
				}
				if(*abcd == '>')	//output redirect
				{
					flag_redirect_out = 1;
					if(*(abcd-1) == '>')
					{
						flag_redirect_out = 2;		// ">>" case
					}
				}
				if(*abcd == '|')
				{
					flag_pipe += 1;
				}
				abcd++;
			}
			myargs[a] = '\0';
            if(flag_redirect_in == 1 && flag_redirect_out >= 1)
            {
                if (redirect_in_out(myargs, a, flag_redirect_out) != 0)
                    break;
                flag_redirect_out = 0;
                flag_redirect_in = 0;
            }
			else if(flag_redirect_out >= 1)
			{
				//handle output redirect ">"
				if (redirect_out(myargs, a, flag_redirect_out) != 0)
					break;
				flag_redirect_out = 0;
			}
			else if(flag_redirect_in == 1)
			{
				//handle input redirect
				if (redirect_in(myargs, a) != 0)
					break;
				flag_redirect_in = 0;
			}
			else if(flag_pipe > 0)
			{
				//handle pipecase
				int loop;
				for(loop = 0; loop < a; loop ++)
				{
					if (strcmp(myargs[loop], "|") == 0)
						break;
				}
				int forking = fork();
				if(forking == -1) {perror("fork"); }
				else if (forking == 0){
					piping(myargs, a, loop+1, 0, STDIN_FILENO);
					exit(1);
					//strcpy(input, "back from pipe");
				}
				else {
					int statval;
					int wc = waitpid(forking, &statval, 0);
				}
				flag_pipe = 0;
				pipeset = 1;
				//printf("back!!\n");
				dup2(0, STDIN_FILENO);
				close(STDIN_FILENO);
				fflush(stdout);
				fflush(stdin);
				continue;
			}
			else if(strcmp(myargs[0], "echo ") == 0)
			{
				int success = echoCommand(args[x-1]);
				if(success < 0)
					perror(NULL);
				//cont_flag = 1;
			}
			else if(strcmp(myargs[0], "pwd ") == 0)
			{
				if(myargs[1] != '\0')
					fprintf(stderr, "pwd expects no arguments\n");
				else
				{
					int success = getPwd();
					if(success < 0)
						perror(NULL);
				}
				cont_flag = 1;
			}
			else if(strcmp(myargs[0], "cd") == 0)
			{
				if(myargs[2] != NULL)
					fprintf(stderr, "cd expects only one argument, multiple given\n");
				else
				{
					int success = changeDir(myargs[1]);
					if(success<0)
						perror(NULL);
				}
				cont_flag = 1;
			}
			else if(strcmp(myargs[0], "pinfo") == 0)
			{
				int found = 0;
				//printf("ghus gya!\n");
				PROCTAB* proc = openproc(PROC_FILLMEM | PROC_FILLSTAT | PROC_FILLSTATUS);
				pid_t neededPid = (pid_t)atoi(myargs[1]);
				proc_t proc_get;
				memset(&proc_get, 0, sizeof(proc_get));
				while(readproc(proc, &proc_get) != NULL)
				{
					if(proc_get.tid == neededPid)
					{
						found = 1;
						char path[PATH_MAX];
						char dest[PATH_MAX];
						struct stat info;
						sprintf(path, "/proc/%d/exe", proc_get.tid);
						if(readlink(path, dest, PATH_MAX) == -1)
							perror("readlink");
						fprintf(stdout, "pid -- %d\nProcess Status -- %c\nMemory -- %lld\nExecutable Path -- %s\n", (int)proc_get.tid, proc_get.state, (unsigned long long int)proc_get.vm_size, dest);
					}
				}
				closeproc(proc);
				if(found == 0)
					fprintf(stderr, "No such process exists\n");
				cont_flag = 1;
				strcpy(input, "");
				strcpy(input1, "");
			}
			else if (strcmp(myargs[0], "jobs") == 0) {
				int loop;
				for(loop = 0; loop < top; loop ++) {
					if(stack_pid[loop] != 0){
						printf("[%d] %d %s\n", pos[loop], (int)stack_pid[loop], name[loop]);
					}
				}
			}
			else if (strcmp(myargs[0], "kjob") == 0) {
				int tokill = atoi(myargs[1]), tosignal = atoi(myargs[2]);
				int loop;
				for(loop = 0; loop<top; loop++)
				{
					if(pos[loop] == tokill && stack_pid[loop] > 0) {
						kill(stack_pid[loop], tosignal);
						stack_pid[loop] = -2;
						break;
					}
				}
			}
			else if (strcmp(myargs[0], "killallbg") == 0) {
				int loop;
				for(loop = 0; loop<top; loop++) {
					if(stack_pid[loop] > 0){
						kill(stack_pid[loop], SIGKILL);
						printf("[%d] %s killed\n", stack_pid[loop], name[loop]);
						stack_pid[loop] = -2;
					}
				}
			}
			else if(strcmp(myargs[0], "") == 0)
			{
				continue;
			}
			else if(strcmp(myargs[0], "quit") == 0)
			{
				exit(1);
			}
			else
			{
				int rc = fork();
				if(rc<0)
				{
					fprintf(stderr, "\nError fork\n");
				}
				else if (rc == 0)		//child for command execution
				{
					int success = execvp(myargs[0], myargs);
					if (success == -1)
					{
						perror(NULL);
					}
				}
				else		//parent, has the shell
				{
					if(flag_bg == 0)
					{
						//printf("inside no bg\n");
						int statval;
						curr_id = rc;
						strcpy(currname, myargs[0]);
						int wc = waitpid(rc, &statval, WUNTRACED);
						if(WSTOPSIG(statval))
							cont_flag = 1;
						if(wc < 0)
							perror(NULL);
					}
					else			//needs working!!
					{
						//printf("hello\n");			//use sigaction if child exits
						fprintf(stdout, "[%d] %d %s\n", flag_bg, rc, myargs[0]);
						stack_pid[top++] = rc;
						pos[top-1] = flag_bg;
						strcpy(name[top-1], myargs[0]);
					}
				}
			}
			if (cont_flag==1)
				break;
		}
		strcpy(input, "");
		strcpy(input1, "");
		//printf("loop over\n");
	}
	return 0;
}
