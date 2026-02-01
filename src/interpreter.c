#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "shellmemory.h"
#include "shell.h"
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <ctype.h>

int MAX_ARGS_SIZE = 3;

int badcommand() {
    printf("Unknown Command\n");
    return 1;
}

// For source command only
int badcommandFileDoesNotExist() {
    printf("Bad command: File not found\n");
    return 3;
}

int is_alphanum(char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
	if (!isalnum(str[i])) {
	    return 2;
	}
    }
    return 0; //is alphanumeric
}

int help();
int quit();
int set(char *var, char *value);
int print(char *var);
int source(char *script);
int badcommandFileDoesNotExist();
int echo(char *input);
int my_ls();
int my_mkdir(char *dirname);
int my_touch(char *filename);
int my_cd(char *subdir);
int run(char *args[], int args_size);

// Interpret commands and their arguments
int interpreter(char *command_args[], int args_size) {
    int i;

    if (args_size < 1 || args_size > MAX_ARGS_SIZE) {
        return badcommand();
    }

    for (i = 0; i < args_size; i++) {   // terminate args at newlines
        command_args[i][strcspn(command_args[i], "\r\n")] = 0;
    }

    if (strcmp(command_args[0], "help") == 0) {
        //help
        if (args_size != 1)
            return badcommand();
        return help();

    } else if (strcmp(command_args[0], "quit") == 0) {
        //quit
        if (args_size != 1)
            return badcommand();
        return quit();

    } else if (strcmp(command_args[0], "set") == 0) {
        //set
        if (args_size != 3)
            return badcommand();
        return set(command_args[1], command_args[2]);

    } else if (strcmp(command_args[0], "print") == 0) {
        if (args_size != 2)
            return badcommand();
        return print(command_args[1]);

    } else if (strcmp(command_args[0], "source") == 0) {
        if (args_size != 2)
            return badcommand();
        return source(command_args[1]);

    } else if (strcmp(command_args[0], "echo") == 0) {
        //echo
	if (args_size != 2)
	    return badcommand();
	return echo(command_args[1]);

    } else if (strcmp(command_args[0], "my_ls") == 0) {
	//my_ls
        if (args_size != 1)
            return badcommand();
        return my_ls();

    } else if (strcmp(command_args[0], "my_mkdir") == 0) {
	//my_mkdir    
        if (args_size != 2)
            return badcommand();
        return my_mkdir(command_args[1]);

    } else if (strcmp(command_args[0], "my_touch") == 0) {
	//my_touch    
        if (args_size != 2)
            return badcommand();
        return my_touch(command_args[1]);
    
    } else if (strcmp(command_args[0], "my_cd") == 0) {
	//my_cd    
        if (args_size != 2)
            return badcommand();
        return my_cd(command_args[1]);

    } else if (strcmp(command_args[0], "run") == 0) {
	//run
	if (args_size < 3)
	    return badcommand();
	return run(command_args, args_size);

    } else
        return badcommand();
}

int help() {

    // note the literal tab characters here for alignment
    char help_string[] = "COMMAND			DESCRIPTION\n \
help			Displays all the commands\n \
quit			Exits / terminates the shell with “Bye!”\n \
set VAR STRING		Assigns a value to shell memory\n \
print VAR		Displays the STRING assigned to VAR\n \
source SCRIPT.TXT	Executes the file SCRIPT.TXT\n \
echo STRING/VAR	Prints the STRING or the VAR value\n \
my_ls                  Lists all the files present in the current directory\n \
my_mkdir STRING/VAR    Creates a new directory with the name STRING or the VAR value\n \
my_touch STRING        Creates a new empty file inside current directory\n \
my_cd STRING           Changes current directory to directory STRING\n \
run COMMAND ARGS	Runs an external command using fork-exec-wait\n";
    printf("%s\n", help_string);
    return 0;
}

int quit() {
    printf("Bye!\n");
    exit(0);
}

int set(char *var, char *value) {
    // Challenge: allow setting VAR to the rest of the input line,
    // possibly including spaces.

    // Hint: Since "value" might contain multiple tokens, you'll need to loop
    // through them, concatenate each token to the buffer, and handle spacing
    // appropriately. Investigate how `strcat` works and how you can use it
    // effectively here.

    mem_set_value(var, value);
    return 0;
}


int print(char *var) {
    printf("%s\n", mem_get_value(var));
    return 0;
}

int source(char *script) {
    int errCode = 0;
    char line[MAX_USER_INPUT];
    FILE *p = fopen(script, "rt");      // the program is in a file

    if (p == NULL) {
        return badcommandFileDoesNotExist();
    }

    fgets(line, MAX_USER_INPUT - 1, p);
    while (1) {
        errCode = parseInput(line);     // which calls interpreter()
        memset(line, 0, sizeof(line));

        if (feof(p)) {
            break;
        }
        fgets(line, MAX_USER_INPUT - 1, p);
    }

    fclose(p);

    return errCode;
}

int echo(char *input) {
    //Check if variable
    if (input[0] == '$') {
	char *varName = input + 1;
	char *value = mem_get_value(varName);
		
    	// Check if exists
    	if (strcmp(value, "Variable does not exist") == 0) {
	    printf("\n");

	} else {
	    printf("%s\n", value);
	    free(value);
	}

    } else {
	printf("%s\n", input);
    }

    return 0;
}    

int compare(const void* a, const void* b) {
	return strcmp(*(const char **)a, *(const char **)b);
}

int my_ls() {
    DIR *dir = opendir(".");
    struct dirent *entry;
    char *dirnames[300];
    int count = 0;

    if (dir == NULL) {
        printf("opendir failed\n");
        return 1;
    }

    while ((entry = readdir(dir)) != NULL) {
	dirnames[count++] = strdup(entry->d_name);
    }

    closedir(dir);

    qsort(dirnames, count, sizeof(char *), compare);

    for (int i = 0; i < count; i++) {
        printf("%s\n", dirnames[i]);
        free(dirnames[i]);
    }
    return 0;
}

int my_mkdir(char *dirname) {
    if (dirname[0] == '$'){
        char *varName = dirname + 1;
        char *value = mem_get_value(varName);
        if (strcmp(value, "Variable does not exist") == 0){
            printf("Bad_command: my_mkdir\n");
            return 1;
        }
        else {
	    if (is_alphanum(value) == 0) {
                int result = mkdir(value, 0755);
                if (result != 0) {
                    printf("Failed to create directory with var\n");
                    return 1;
                }
            } 
	    else {
	        printf("Bad command: my_mkdir\n");
	        return 1;
	    }
	}
    }
    else {
	if (is_alphanum(dirname) != 0) {
	    printf("Bad command: my_mkdir\n");
	    return 1;
	}
        int result = mkdir(dirname, 0755);
        if (result != 0) {
            printf("Failed to create directory with var\n");
            return 1;
        }
    }
    return 0;
}

int my_touch(char *filename) {
    if (is_alphanum(filename) == 0) {
        FILE *file = fopen(filename, "a");
        if (file == NULL) {
            printf("Failed to open/create file\n");
            return 1;
        }
        fclose(file);
        return 0;
    }
    printf("Bad_command: my_touch\n");
    return 1;
}

int my_cd(char *subdir) {
    if (chdir(subdir) != 0) {
        printf("Bad_command: my_cd\n");
        return 1;
    }
    return 0;
}

int run(char *args[], int args_count){
    pid_t pid = fork();

    // if fork fails
    if (pid < 0) {
        perror("fork");
	return 1;
    }
    
    //Child
    if (pid == 0) {

	//Execute list with null ending
	char *exec_args[args_count];
        for (int i = 1; i < args_count; i++) {
            exec_args[i - 1] = args[i];
	}
        exec_args[args_count - 1] = NULL;
        execvp(exec_args[0], exec_args);

	//If error
	perror("execvp");
	exit(1);

    //Parent
    } else {
         int status;
	 waitpid(pid, &status, 0);

	 //Check child exit state
	 if (WIFEXITED(status)) {
	     return WEXITSTATUS(status);
	 }
	    
	 return 1;
    }
}





