#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> //getting the user id 
#include <pwd.h> // to get the home directory  
#include <sys/wait.h> //waitpid()
#include <signal.h>

#define MAX_INPUT 1000
#define MAX_ARGUMENTS 1000

pid_t bg_pids[100];
int bg_count = 0; 

struct Variable{
    char key[1000];
    char value[1000];
};

struct Variable var_list[100];
int var_count = 0; //keep tracing how many of the variables we already got

void set_TheVariable(char *new_key,char *new_value){
    //check the existance of the variable first 
    for(int i = 0 ; i<var_count;i++){
        if(strcmp(var_list[i].key, new_key)== 0){
            strcpy(var_list[i].value , new_value);
            return;
        }
    }
    if (var_count <100){
        strcpy(var_list[var_count].key ,new_key);
        strcpy(var_list[var_count].value , new_value);
        var_count++;

    }else{
        printf("the storage of our variables is full cannot add more and more: %s\n",new_key);
    }

}

char* get_TheVariable(char *search_key){
    for(int i = 0; i<var_count; i++){
        if(strcmp(var_list[i].key ,search_key)==0){
            return var_list[i].value; //ok when we should be getting it 
        }
    }
    return NULL; // variable doesn't exist unfortunitely  
}

void child_death(int sig){
    int status;
    
    while(waitpid(-1 , &status,WNOHANG)> 0 ){
        //open the file write nad clsoe 
        FILE *log_file = fopen("shell.log" , "a");
        if (log_file != NULL){
            fprintf(log_file, "Child process was terminated\n");
            fclose(log_file);
        }
    }
}

 

int main() {

    signal(SIGCHLD, child_death);

    signal(SIGINT , SIG_IGN);

    char input[MAX_INPUT]; // ARRAY TO HOLD THE STRING USER TYPE DOWN 
    char *args[MAX_ARGUMENTS]; //POINTER TO HOLD EACH OF THE PARSED 
    
    //THE SHELL LOOPING 
    while (1){ 

        char cwd[1000]; // the current work directory 
        if(getcwd(cwd,sizeof(cwd)) != NULL){
            printf("\033[1;31m%s\033[0m> ",cwd);
        }else{
            printf("The Shell> ");
        }
        fflush(stdout);

        //READING THE INPUT FROM HERE 
        if (fgets(input,sizeof(input), stdin) ==NULL){
            printf("\n");
            break; 
            //EXIT ONLY IF THE USER TYPED CTRL+D
        }

        //REPLACE THE /N WITH THE NULL TERMINATOR /0
        input[strcspn(input,"\n")] =0;

        if (strlen(input) ==0){
            continue;
        }

        //parsing the input into seperated strings 
        int i = 0;
        args[i] =strtok(input, " ");

        //KEEP DOING THAT UNTIL THE GOOD NULL 
        while (args[i] !=NULL){
            i++;
            args[i] = strtok(NULL, " ");
        }

        if(args[0] ==NULL){
            continue;
        }

        if(strcmp(args[0] , "exit")==0){
            for (int k =  0 ; k<bg_count;k++){
                kill(bg_pids[k],SIGTERM);

            } 
            printf("exitin' MyShell.........\n");
            exit(0);

        } 
        if(strcmp(args[0],"cd") ==0){
            char *path =args[1];

            if (path ==NULL || strcmp(path, "~")==0){
                //we may ask the OS FOR THE current user profile so we can get the home from it 
                struct passwd *pw = getpwuid(getuid());
                if(pw != NULL){
                    path = pw->pw_dir ; //how we extract the home directory string
                }else{
                    printf("We an't able to determine the directory of the home try again later.\n");
                    continue;
                }
            }

            //attempting to change the directory 
            if(chdir(path) !=0){
                perror("ERROR tryna execute the cd");
            }
            continue;
        
        }

        //THE EXPORT COMMAND 
        if (strcmp(args[0],"export")==0){
            if(args[1]==NULL){
                printf("ERROR export requires some kinda arguments like(export x = 19)\n");
                continue;
            }

            //handle spaces inside of the quotations 
            char export_string[1000] ="";
            for(int j=1;j<i ;j++){
                strcat(export_string,args[j]);
                if(j<i-1){
                    strcat(export_string, " "); // adding the space back between em'
                }
            } 
            //find the = 
            char *equal = strchr(export_string , '=');

            if(equal != NULL){
                // split the string into a key and a value
                *equal = '\0';
                char *key = export_string;
                char *value = equal+1; 

                char *end = equal - 1;

                while(end >= key && *end ==' '){
                    *end = '\0';
                    end--;
                }

                while (*value == ' ') value++;
                int vlen = strlen(value);

                //check the "" and remove em'
                if (value[0] =='"' &&value[strlen(value)-1]=='"'){
                    value[strlen(value)-1] = '\0'; 
                    value++;
                }
                //save to the hashmap 
                set_TheVariable(key , value);
            }else{
                printf("ERROR invalid export maybe ur using invalid key or value or sth.");
            }
            continue;

        }

        //the echo command 
        if(strcmp(args[0],"echo")==0){
            char echo_string[1000] = "";
            for(int j =1 ; j<i ;j++){
                strcat(echo_string , args[j]);
                if(j<i-1){
                    strcat(echo_string," ");
                }
            }

            //remove the "" 
            int len =strlen(echo_string);
            char *read = echo_string;

            if(len >= 2 && echo_string[0] =='"' && echo_string[len-1] =='"'){
                echo_string[len-1] ='\0'; //errase the last quote
                read++; //move the ptr past the first quote
            }

            //print the characters and validating the $
            while(*read != '\0'){
                if(*read == '$'){
                    //handle the variable 
                    read++;
                    char variable_name[1000];
                    int variable_idx = 0 ; 

                    //geting the variable name 
                    while (*read != '\0' && *read != ' ' && *read !='"'){
                        variable_name[variable_idx++] = *read;
                        read++;
                    }
                    variable_name[variable_idx] = '\0'; 

                    char *val = get_TheVariable(variable_name);
                    if(val != NULL){
                        printf("%s",val); //print the saved value in the hash map 

                    }
                }else{
                    //just like a normal letter ,print it tho 
                    printf("%c" , *read);
                    read++;
                }
            }
            printf("\n");
            continue;
        }

        char *execute_arguments[MAX_ARGUMENTS];
        int exec_i = 0 ; 
        char temp_vals[10][1000];
        int temp_count = 0 ; 

        for(int j = 0 ; j <i ; j++){
            if(args[j][0] =='$'){
                char *var_name = args[j] +1 ;  //skip the $
                char *val = get_TheVariable(var_name);

                if(val != NULL){
                    strcpy(temp_vals[temp_count],val);

                    char *words = strtok(temp_vals[temp_count]," \t");
                    while (words !=NULL){
                        execute_arguments[exec_i] = words;
                        exec_i++;
                        words = strtok(NULL, " \t");
                    }
                    temp_count++;

                }

            }else{
                //not a variable just pass it 
                execute_arguments[exec_i] = args[j];
                exec_i++;
            }
        }
        execute_arguments[exec_i] =NULL;


        int in_background = 0 ; 
        if(exec_i> 0 && strcmp(execute_arguments[exec_i  -1], "&") == 0 ){
            in_background = 1 ; 
            execute_arguments[exec_i-1] =NULL; //hide the &
        }

        pid_t child_pid = fork();

        if(child_pid < 0 ){
            perror("FAILD TO FORK");
        }
        else if (child_pid==0){

            signal(SIGINT,SIG_DFL);
            if(execvp(execute_arguments[0],execute_arguments)==-1){
                printf("ERROR Command '%s' is not found\n",execute_arguments[0]);
                exit(1);
            }

        }
        else{
            //parent process 
            if(in_background){
                // don't wait print the pid an continue 
                printf("[Runnin' in the background] PID: %d\n",child_pid);
                if(bg_count<100){
                    bg_pids[bg_count++] =child_pid;
                }

            }else{
                //wait for the child here
                int status ; 
                waitpid(child_pid,&status, 0 );
            }
        }

    }
    return 0;
}