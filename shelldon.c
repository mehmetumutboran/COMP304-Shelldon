/*
 * shelldon interface program

KUSIS ID: PARTNER NAME: Mehmet Taha Bastug
KUSIS ID: PARTNER NAME: Mehmet Umut Boran

 */

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <ctype.h>


#define MAX_LINE       80 /* 80 chars per line, per command, should be enough. */
#define ISFILE ''
#define LINE_LENGTH 500


struct hist
{
    char **comArray;
    int count;
    int comsize;
};

typedef struct hist Hist;
typedef Hist *hpt;

struct node
{
    Hist command;
    struct node *next;
};

typedef struct node Node;
typedef Node *nPtr;

int listsize;

nPtr head;
int ccounter = 0;
char old_pid[10] = "-1";

int parseCommand(char inputBuffer[], char *args[], int *background);

void fastExec(int);

void execute(char **, int);

void execHistory(char **);

void execFile(char **, int);

void addHistory(hpt);

void codesearch(char **);

void birdakika(char **);

void execWebgo(char **);

void execOldestChild(char **pString);

void exitWork();

void execTranslate(char **pString);

int main(void)
{
    char inputBuffer[MAX_LINE];            /* buffer to hold the command entered */
    int background;                        /* equals 1 if a command is followed by '&' */
    char *args[MAX_LINE / 2 + 1];            /* command line (of 80) has max of 40 arguments */
    pid_t child;                    /* process id of the child process */
    int status;                /* result from execv system call*/
    int shouldrun = 1;

    int upper;

    head = malloc(sizeof(Node));
    listsize = 0;
    exitWork();

    while (shouldrun)
    {                    /* Program terminates normally inside setup */
        background = 0;

        shouldrun = parseCommand(inputBuffer, args, &background);       /* get next command */

        if (strncmp(inputBuffer, "exit", 4) == 0)
            shouldrun = 0;     /* Exiting from shelldon*/

        int counter = 0;

        char **args_cpy = (char **) malloc(sizeof(char *) * MAX_LINE);
        for (int i = 0; i < MAX_LINE; ++i)
        {
            args_cpy[i] = (char *) malloc(sizeof(char) * (LINE_LENGTH));
        }

        for (char **ptr = args; *ptr != NULL; ptr++, counter++)
        {
            strcpy(args_cpy[counter], *ptr);
        }
        args_cpy[counter] = NULL;

        if (shouldrun)
        {
            /*
          After reading user input, the steps are
          (1) Fork a child process using fork()
          (2) the child process will invoke execv()
          (3) if command included &, parent will invoke wait()
             */
            hpt newC = malloc(sizeof(Hist));
            newC->comArray = args_cpy;
            newC->count = ccounter;
            newC->comsize = counter;
            if (args[0][0] != '!')
            {
                addHistory(newC);
            } else
            {
                nPtr current = head;
                int j;
                if (args[0][1] == '!')
                    for (j = 0; j < listsize; j++)
                    {
                        current = current->next;
                    }
                else
                {
                    for (j = 0; j < (args[0][1] - '0'); j++)
                    {
                        current = current->next;
                    }
                }
                newC = malloc(sizeof(Hist));
                newC->comArray = current->command.comArray;
                newC->comsize = current->command.comsize;
                newC->count = ccounter;
                addHistory(newC);
            }
            ccounter++;
            if (strcmp(args[0], "cd") == 0)
            {
                execute(args, counter);
                continue;
            }

            if (strcmp(args[0], "oldestchild") == 0)
            {
                execute(args, counter);
                continue;
            }

            if (strcmp(args[0], "exit") == 0)
                break;


            child = fork();

            if (child == 0)
            {
                execute(args, counter);
                kill(getpid(), 1);
            } else if (child > 0)
            {
                if (!background)
                    wait(NULL);
            } else
            {
                perror("Unsuccessful Fork!");
            }

        }
    }

    exitWork();
    printf("Exiting Shelldon...\n");
    return 0;
}

void exitWork()
{
    char cmd[MAX_LINE] = "grep -inhR oldestchild /proc/modules | wc -l";
    FILE *fp;
    int bsize = 128;
    char out[bsize];
    fp = popen(cmd, "r");
    fgets(out, bsize, fp);
    //printf("%s", out);
    fclose(fp);
    pclose(fp);
    if (strcmp(out, "0\n") == 0) // No loaded modules found
        return;
    char **args_cpy = (char **) malloc(sizeof(char *) * 4);
    int i;
    for (i = 0; i < 3; ++i)
    {
        args_cpy[i] = (char *) malloc(sizeof(char) * (LINE_LENGTH));
    }

    strcpy(*(args_cpy), "/usr/bin/sudo");
    strcpy(*(args_cpy + 1), "rmmod");
    strcpy(*(args_cpy + 2), "oldestchild");

    /*for (i = 0; *(args_cpy + i) != NULL; ++i)
    {
        printf("Rmmod: %s\n", *(args_cpy + i));
    }*/

    int pid = fork();

    if (pid == 0)
    {
        execv(*(args_cpy), args_cpy); // sudo rmmod oldestchild
        //printf("Remove output:%d\n", output);
    } else if (pid > 0)
        wait(NULL);
    else
        printf("Fork error!\n");

    for (i = 0; i < 3; ++i)
    {
        free(args_cpy[i]);
    }
}

void execute(char **args, int comsize)
{
    //printf("In execute!\n");
    if (comsize >= 2 && args[comsize - 2][0] == '>')
    {
        execFile(args, comsize);
    } else
    {
        char exe[LINE_LENGTH] = "/bin/";
        strcat(exe, args[0]);
        int output = 0;

        output = execv(exe, args);

        if (output == -1)
        {
            strcpy(exe, "/usr/bin/");
            strcat(exe, args[0]);
        }
        output = execv(exe, args);

        if (output == -1 && strcmp(args[0], "cd") == 0)
        {
            if (chdir(args[1]) != 0)
            {
                printf("There is no such directory!!!\n");
            }
            return;
        }

        if (output == -1 && strcmp(args[0], "codesearch") == 0)
        {
            codesearch(args);
            return;
        }

        if (output == -1 && strcmp(args[0], "birdakika") == 0)
        {
            birdakika(args);
            return;
        }

        if (output == -1 && strcmp(args[0], "history") == 0)
        {
            execHistory(args);
            return;
        }

        if (output == -1 && strcmp(args[0], "oldestchild") == 0)
        {
            execOldestChild(args);
            return;
        }

        if (output == -1 && strcmp(args[0], "webgo") == 0)
        {
            execWebgo(args);
            return;
        }

        if (output == -1 && strcmp(args[0], "translate") == 0)
        {
            execTranslate(args);
            return;
        }


        if (output == -1 && args[0][0] == '!')
        {
            if (args[0][1] == '!')
            {
                fastExec(0);
            } else
            {
                fastExec((args[0][1] - '0'));
            }
            return;
        }

        if (output == -1)
        {
            printf("Couldn't find the exe!\n");
        }

    }
}

void execTranslate(char **args)
{
    //translate araba -tren -c 5
    if(strcmp(*(args + 1),"--help") == 0){
        printf("This program uses tureng.com to translate from Turkish to English or English to Turkish.\n");
        printf("--help -> to get help.\n");
        printf("-tren -> to translate from Turkish to English.\n");
        printf("-entr -> to translate from English to Turkish.\n");
        printf("-c #num -> to translate #num many results for the query,\n"
               "if #num isn't specified it returns all the results\n"
               "also if -c isn't used it returns only one result.\n");
        printf("Example syntax for translating the word \"car\" from English to Turkish;\n");
        printf("translate car -entr\n");
        printf("Example syntax for translating the word \"yular\" from Turkish to English with 5 results;\n");
        printf("translate yular -tren -c 5\n");
        return;
    }
    char specialChr[LINE_LENGTH] = "123456789!@#$%^&*()_+{}[]/|.,><`~";
    char searchBase[LINE_LENGTH];
    if (strstr(*(args + 2), "-entr") != NULL) strcpy(searchBase, "lang=\"tr\"><a");
    else if (strstr(*(args + 2), "-tren") != NULL) strcpy(searchBase, "lang=\"en\"><a");
    else
    {
        printf("Please enter a either Turkish or English! (-entr or -tren)\n");
        return;
    }
    char cmd[LINE_LENGTH] = "GET https://tureng.com/en/turkish-english/";
    strcat(cmd, *(args + 1));
    int translateCount = 1;

    if (*(args + 3) != NULL && strcmp(*(args + 3), "-c") == 0)
    {
        if (*(args + 4) != NULL)
            //assumes the number input after c is integer
            translateCount = atoi(*(args + 4));
        else
            translateCount = LINE_LENGTH;
    }

    FILE *fp;
    int bsize = 128;
    char out[bsize];
    fp = popen(cmd, "r");
    int countLoop = 0;
    while (countLoop < translateCount && fgets(out, bsize, fp) != NULL)
    {
        char *translation = strstr(out, searchBase);
        char *lastPtr;
        if (translation != NULL)
        {
            translation = strstr(translation, "href");
            translation = strstr(translation, "\">");
            if (strstr(translation, "</a>") == NULL) continue;
            lastPtr = strstr(translation, "</a>");
            char *result = malloc(sizeof(char) * 100);
            strncpy(result, translation, lastPtr - translation);

            const char *PATTERN1 = "\">";
            const char *PATTERN2 = "</a>";

            char *target = NULL;
            char *start, *end;

            start = strstr(translation, PATTERN1);
            if (start)
            {
                start += strlen(PATTERN1);
                end = strstr(start, PATTERN2);
                if (end)
                {
                    target = (char *) malloc(end - start + 1);
                    memcpy(target, start, end - start);
                    target[end - start] = '\0';
                }
            }


            if (target)
            {
                int illegal = 0;
                for (int i = 0; i < strlen(specialChr); ++i)
                {
                    if (strchr(target, specialChr[i]) != NULL) illegal = 1;
                }
                if (!illegal)
                {
                    printf("%s\n", target);
                    countLoop++;
                }
            }

            free(target);
        }
    }
    fclose(fp);
    pclose(fp);
}

void execOldestChild(char **args)
{
    int isNew = 1;
    if (*(args + 1) == NULL)
    {
        printf("Please supply a PID!\n");
        return;
    }
    if (atoi(*(args + 1)) < 0)
    {
        printf("Please enter a valid PID!\n");
        return;
    }

    //printf("Old pid is : %s, new pid is: %s \n", old_pid, *(args + 1));
    if (strcmp(*(args + 1), old_pid) == 0)
    {
        printf("Module is already loaded!\n");
        isNew = 0; //pid == old_pid -> old
    } else
    { // new pid
        if (strcmp(old_pid, "-1") != 0) // if not first given pid
        {
            //printf("In remove with pid=%s\n", *(args + 1));
            char **args_cpy = (char **) malloc(sizeof(char *) * 4);
            int i;
            for (i = 0; i < 3; ++i)
            {
                args_cpy[i] = (char *) malloc(sizeof(char) * (LINE_LENGTH));
            }

            strcpy(*(args_cpy), "/usr/bin/sudo");
            strcpy(*(args_cpy + 1), "rmmod");
            strcpy(*(args_cpy + 2), "oldestchild");

            /*for (i = 0; *(args_cpy + i) != NULL; ++i)
            {
                printf("Rmmod: %s\n", *(args_cpy + i));
            }*/

            int pid = fork();

            if (pid == 0)
            {
                execv(*(args_cpy), args_cpy); // sudo rmmod oldestchild
                //printf("Remove output:%d\n", output);
            } else if (pid > 0)
                wait(NULL);
            else
                printf("Fork error!\n");

            //printf("After remove!\n");

            for (i = 0; i < 3; ++i)
            {
                free(args_cpy[i]);
            }
        }

        strcpy(old_pid, *(args + 1));
    }

    int pid;

    if (isNew) // sudo insmod oldestchild.ko pid=arg+1
    {
        char **args_cpy = (char **) malloc(sizeof(char *) * 4);
        for (int i = 0; i < 4; ++i)
        {
            args_cpy[i] = (char *) malloc(sizeof(char) * (LINE_LENGTH));
        }

        strcpy(*(args_cpy), "/usr/bin/sudo");
        strcpy(*(args_cpy + 1), "insmod");
        strcpy(*(args_cpy + 2), "oldestchild.ko");
        strcpy(*(args_cpy + 3), "pid=");
        strcat(*(args_cpy + 3), *(args + 1));

        /*for (int i = 0; *(args_cpy + i) != NULL; ++i)
        {
            printf("Insmod :%s\n", *(args_cpy + i));
        }*/

        pid = fork();

        if (pid == 0)
            execv(*(args_cpy), args_cpy);
        else
            wait(NULL);

        for (int i = 0; i < 4; ++i)
        {
            free(args_cpy[i]);
        }
    }

}

void execFile(char **args, int comsize)
{
    char cmd[LINE_LENGTH] = "";
    int c = 0;
    for (char **ptr = args; c < comsize - 2; ptr++, c++)
    {
        strcat(cmd, *ptr);
        if (c < comsize - 3)
        {
            strcat(cmd, " ");
        }
    }
    char filename[100];
    strcpy(filename, args[comsize - 1]);
    FILE *fp;
    int bsize = 128;
    char out[bsize];
    fp = popen(cmd, "r");
    FILE *com = NULL;
    if (strcmp(args[comsize - 2], ">>") == 0)
    {
        com = fopen(filename, "a");
    } else if (strcmp(args[comsize - 2], ">") == 0)
    {
        com = fopen(filename, "w");
    }
    while (fgets(out, bsize, fp) != NULL)
    {
        fprintf(com, "%s", out);
        printf("%s", out);
    }
    fclose(com);
    fclose(fp);
    pclose(fp);
}

void addHistory(hpt newC)
{
    nPtr current = head;
    while (current->next != NULL)
    {
        current = current->next;
    }
    nPtr new = malloc(sizeof(Node));
    new->command = *newC;
    current->next = new;
    new->next = NULL;
    listsize++;
}

void execHistory(char **args)
{
    int i;
    nPtr current = head;
    if (listsize <= 10)
    {
        for (i = 0; i < listsize - 1; i++)
        {
            current = current->next;
            printf("%d ", current->command.count + 1);
            for (char **ptr = current->command.comArray; *ptr != NULL; ptr++)
            {
                printf("%s ", *ptr);
            }
            printf("\n");
        }
    } else
    {
        for (i = 0; i < listsize - 1; i++)
        {
            current = current->next;
            if (listsize - i <= 11)
            {
                printf("%d ", current->command.count + 1);
                for (char **ptr = current->command.comArray; *ptr != NULL; ptr++)
                {
                    printf("%s ", *ptr);
                }
                printf("\n");
            }
        }
    }
}

void fastExec(int i)
{
    if (ccounter - 1 == 0)
    {
        printf("Command history is clear!!!\n");
        return;
    }
    if (i > listsize)
    {
        printf("There is no such command!!!\n");
        return;
    }
    nPtr current = head;
    if (i == 0)
    {
        int j;
        for (j = 0; j < listsize - 1; j++)
        {
            current = current->next;
        }
        execute(current->command.comArray, current->command.comsize);
    } else
    {
        int j;
        for (j = 0; j < i; j++)
        {
            current = current->next;
        }
        execute(current->command.comArray, current->command.comsize);
    }
}

void birdakika(char **args)
{
    // birdakika hh.mm music_file

    if (*(args + 1) == NULL)
    {
        printf("Please enter the time argument!\n");
        return;
    }

    if (*(args + 2) == NULL)
    {
        printf("Please enter the song argument!\n");
        return;
    }

    char time[6];
    strcpy(time, *(args + 1)); // time = hh.mm
    const char s[2] = "."; // seperator
    char *token;
    token = strtok(time, s);
    int minute = -1, hour = -1, count = 0;

    while (token != NULL)
    { // 7.15
        int time_unit = atoi(token);
        if (count == 0) // when count 0, time_unit indicates hour
        {
            if (time_unit >= 24 || time_unit < 0)
            {
                printf("Please supply a correct hour info!\n");
                return;
            }
            hour = time_unit;

        } else if (count == 1) // when count 1, time_unit indicates minute
        {
            if (time_unit >= 60 || time_unit < 0)
            {
                printf("Please supply a correct minute info!\n");
                return;
            }
            minute = time_unit;

        } else
        {
            printf("You supplied too many time info!\n");
        }
        //printf("Strtok: %s\n", token);

        token = strtok(NULL, s);
        count++;
    }

    // args_cpy = mm hh * * * mpg321 path/to/music_file
    // size = 7

    char input[LINE_LENGTH];
    char file[LINE_LENGTH] = "./";
    strcat(file, *(args + 2)); // ./file

    char *real_path;
    real_path = realpath(file, NULL);

    if (real_path == NULL)
    {
        printf("Couldn't find the file %s!\n", file + 2); // to get rid of ./
        return;
    }

    /*FILE *shPtr;
    shPtr = fopen("./submit_job.sh", "w");

    fprintf(shPtr,"mpg321 %s",real_path);

    char *real_path_sh;
    real_path_sh = realpath("./submit_job.sh", NULL);

    if (real_path_sh == NULL)
    {
        printf("Couldn't find the file %s!\n", file + 2); // to get rid of ./
        return;
    }*/

    sprintf(input, "%d %d %c %c %c %s %s", minute, hour, '*', '*', '*', "mpg321", real_path);
    //sprintf(input, "%d %d %c %c %c %s", minute, hour, '*', '*', '*', real_path_sh);
    printf("Input is %s\n", input);

    FILE *fp;
    DIR *d;
    struct dirent *dir;
    d = opendir("/var/spool/cron/crontabs");
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
                continue;
            char directory[LINE_LENGTH] = "/var/spool/cron/crontabs/";
            strcat(directory, dir->d_name);
            fp = fopen(directory, "a");
            fprintf(fp, "%s\n", input);
            fclose(fp);
        }
    }
    printf("Job submitted!\n");
}

void codesearch(char **args)
{
    //printf("In the code search\n");
    /*for (int i = 0; *(args + i) != NULL; ++i)
    {
        printf("%s\n", *(args + i));
    }*/

    if (*(args + 1) == NULL)
    {
        printf("Supply the keyword!\n");
        return;
    }

    if (strcmp(*(args + 1), "-r") == 0) // codesearch -r "foo"
    {
        if (*(args + 2) == NULL)
        {
            printf("Supply the keyword!\n");
            return;
        }
        char *keyword = (char *) malloc((sizeof(char)) * (LINE_LENGTH));
        strcpy(keyword, *(args + 2));
        //printf("Looking for %s recursively...\n", keyword);
        DIR *d;
        struct dirent *dir;
        d = opendir(".");
        pid_t child;
        if (d)
        {
            while ((dir = readdir(d)) != NULL)
            {
                //printf("In -r ./ %s (%c) %d\n", dir->d_name, dir->d_type, dir->d_type == ISFILE
                // );
                if (strcmp(dir->d_name, "..") == 0 || strcmp(dir->d_name, ".") == 0)
                    continue;
                if (dir->d_type == ISFILE)
                {
                    //printf("In the recursive call %s (%c) %d\n", dir->d_name, dir->d_type, dir->d_type == ISFILE
                    // );
                    char **args_cpy = (char **) malloc(sizeof(char *) * 4);
                    for (int i = 0; i < 4; ++i)
                    {
                        args_cpy[i] = (char *) malloc(sizeof(char) * (LINE_LENGTH));
                    }
                    strcpy(*(args_cpy), "codesearch");
                    strcpy(*(args_cpy + 1), keyword);
                    strcpy(*(args_cpy + 2), "-rf");
                    strcpy(*(args_cpy + 3), "./"); // "./"
                    strcat(*(args_cpy + 3), dir->d_name); // "./dir_name"

                    //child = fork();
                    //if (child == 0)
                    codesearch(args_cpy);
                    //else
                    //wait(NULL);

                    for (int i = 0; i < 4; ++i)
                    {
                        free(args_cpy[i]);
                    }
                } else
                {
                    FILE *fp;

                    char line[LINE_LENGTH];

                    fp = fopen(dir->d_name, "r");
                    int lineNum = 1;
                    while (fgets(line, LINE_LENGTH, (FILE *) fp) != NULL)
                    {
                        if (strstr(line, keyword) != NULL)
                        {
                            printf("%d: ./%s -> %s", lineNum, dir->d_name, line);
                        }
                        lineNum++;
                    }

                    fclose(fp);
                }
            }
            closedir(d);
        }
        return;
    }
    //recursive codesearch

    if (*(args + 2) != NULL && strcmp(*(args + 2), "-f") == 0) // codesearch "foo" -f dir
    {
        if (*(args + 3) == NULL)
        {
            printf("Supply the target directory!\n");
            return;
        }
        //printf("Looking for %s in the target %s...\n", *(args + 1), *(args + 3));
        DIR *d;
        struct dirent *dir;
        d = opendir(*(args + 3));
        int size = 0;
        if (d)
        {
            while ((dir = readdir(d)) != NULL)
            {
                size++;
                //printf("In %s %s\n", *(args + 3), dir->d_name/*, dir->d_type, dir->d_type == ISFILE
                // */);
                if (strcmp(dir->d_name, "..") == 0 || strcmp(dir->d_name, ".") == 0 || dir->d_type == ISFILE)
                    continue;
                FILE *fp;

                char line[LINE_LENGTH];

                char directory[LINE_LENGTH]; //""
                strcpy(directory, *(args + 3)); // "dir"
                strcat(directory, "/"); // "dir/"
                strcat(directory, dir->d_name); // "dir/dir->d_name"

                fp = fopen(directory, "r");
                int lineNum = 1;
                while (fgets(line, LINE_LENGTH, (FILE *) fp) != NULL)
                {
                    if (strstr(line, *(args + 1)) != NULL)
                    {
                        printf("%d: %s -> %s", lineNum, directory, line);
                    }
                    lineNum++;
                }

                fclose(fp);
            }
            closedir(d);
        }
        //printf("Size of this directory is %d\n",size);
        return;
    }

    if (*(args + 2) != NULL && strcmp(*(args + 2), "-rf") == 0) // codesearch "foo" -rf dir // recursive call from -r
    {
        if (*(args + 3) == NULL)
        {
            printf("Supply the target directory!\n");
            return;
        }
        //printf("Looking for %s in the target %s...\n", *(args + 2), *(args + 3));
        DIR *d;
        pid_t child;
        struct dirent *dir;

        d = opendir(*(args + 3));
        if (d)
        {
            while ((dir = readdir(d)) != NULL)
            {
                //printf("In -rf/ %s %s %d\n", *(args + 3), dir->d_name, dir->d_type /*dir->d_type == ISFILE
                // */);
                if (strcmp(dir->d_name, "..") == 0 || strcmp(dir->d_name, ".") == 0)
                    continue;
                if (dir->d_type == ISFILE)
                {
                    char **args_cpy = (char **) malloc(sizeof(char *) * 4);
                    for (int i = 0; i < 4; ++i)
                    {
                        args_cpy[i] = (char *) malloc(sizeof(char) * (LINE_LENGTH));
                    }
                    strcpy(*(args_cpy), "codesearch");
                    strcpy(*(args_cpy + 1), *(args + 1));
                    strcpy(*(args_cpy + 2), "-rf");
                    strcpy(*(args_cpy + 3), *(args + 3)); // "dir"
                    strcat(*(args_cpy + 3), "/"); // "dir/"
                    strcat(*(args_cpy + 3), dir->d_name); //"dir/dir_name"

                    //child = fork();
                    //if (child == 0)
                    codesearch(args_cpy);
                    //else
                    //    wait(NULL);

                    for (int i = 0; i < 4; ++i)
                    {
                        free(args_cpy[i]);
                    }
                } else
                {
                    FILE *fp;

                    char line[LINE_LENGTH];

                    char directory[LINE_LENGTH];
                    strcpy(directory, *(args + 3));
                    strcat(directory, "/");
                    strcat(directory, dir->d_name);

                    fp = fopen(directory, "r");
                    int lineNum = 1;
                    while (fgets(line, LINE_LENGTH, (FILE *) fp) != NULL)
                    {
                        if (strstr(line, *(args + 1)) != NULL)
                        {
                            printf("%d: %s -> %s", lineNum, directory, line);
                        }
                        lineNum++;
                    }

                    fclose(fp);
                }

            }
            closedir(d);
        }
        return;
    }


    // codesearch foo
    DIR *d;
    struct dirent *dir;
    d = opendir(".");
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            if (strcmp(dir->d_name, "..") == 0 || strcmp(dir->d_name, ".") == 0 || dir->d_type == ISFILE)
                continue;
            FILE *fp;

            char line[LINE_LENGTH];

            fp = fopen(dir->d_name, "r");
            int lineNum = 1;
            while (fgets(line, LINE_LENGTH, (FILE *) fp) != NULL)
            {
                if (strstr(line, *(args + 1)) != NULL)
                {
                    printf("%d: ./%s -> %s", lineNum, dir->d_name, line);
                }
                lineNum++;
            }

            fclose(fp);
        }
        closedir(d);
    }
}

void execWebgo(char **args)
{
    char go[200];
    char siteS[200];
    if (args[2] != NULL && strcmp(args[2], "-s") == 0)
    {
        if (strcmp(args[1], "google") == 0)
        {
            strcpy(siteS, "https://www.google.com/search?source=hp&ei=DnueXPLjM4Po6QTj86voDQ&q=");
            for (char **ptr = args + 3; *ptr != NULL; ptr++)
            {
                strcat(siteS, *ptr);
                if (*(ptr + 1) != NULL)
                {
                    strcat(siteS, "+");
                }
            }
            strcat(siteS, "&btnK=Google+Search&oq=");
            for (char **ptr = args + 3; *ptr != NULL; ptr++)
            {
                strcat(siteS, *ptr);
                if (*(ptr + 1) != NULL)
                {
                    strcat(siteS, "+");
                }
            }
            strcat(siteS,
                   "&gs_l=psy-ab.3..0l10.282346.282968..283924...0.0..1.302.1066.0j5j1j1......0....1..gws-wiz.....0.DPSTkoV3-tU");
            strcpy(go, siteS);
        } else if (strcmp(args[1], "youtube") == 0)
        {
            strcpy(siteS, "https://www.youtube.com/results?search_query=");
            for (char **ptr = args + 3; *ptr != NULL; ptr++)
            {
                strcat(siteS, *ptr);
                if (*(ptr + 1) != NULL)
                {
                    strcat(siteS, "+");
                }
            }
            strcpy(go, siteS);
        } else if (strcmp(args[1], "reddit") == 0)
        {
            strcpy(siteS, "https://www.reddit.com/search?q=");
            for (char **ptr = args + 3; *ptr != NULL; ptr++)
            {
                strcat(siteS, *ptr);
                if (*(ptr + 1) != NULL)
                {
                    strcat(siteS, "%20");
                }
            }
            strcpy(go, siteS);
        } else if (strcmp(args[1], "stackoverflow") == 0)
        {
            strcpy(siteS, "https://stackoverflow.com/search?q=");
            for (char **ptr = args + 3; *ptr != NULL; ptr++)
            {
                strcat(siteS, *ptr);
                if (*(ptr + 1) != NULL)
                {
                    strcat(siteS, "+");
                }
            }
            strcpy(go, siteS);
        }
    } else
    {
        strcpy(go, "http://");
        strcat(go, args[1]);
        strcat(go, ".com");
    }

    printf("Site Opened: %s\n", go);
    execlp("xdg-open", " ", go, NULL);
}


/**
 * The parseCommand function below will not return any value, but it will just: read
 * in the next command line; separate it into distinct arguments (using blanks as
 * delimiters), and set the args array entries to point to the beginning of what
 * will become null-terminated, C-style strings.
 */

int parseCommand(char inputBuffer[], char *args[], int *background)
{
    int length,        /* # of characters in the command line */
            i,        /* loop index for accessing inputBuffer array */
            start,        /* index where beginning of next command parameter is */
            ct,            /* index of where to place the next parameter into args[] */
            command_number;    /* index of requested command number */

    ct = 0;

    /* read what the user enters on the command line */
    do
    {
        printf("shelldon>");
        fflush(stdout);
        length = read(STDIN_FILENO, inputBuffer, MAX_LINE);
    } while (inputBuffer[0] == '\n'); /* swallow newline characters */

    /**
     *  0 is the system predefined file descriptor for stdin (standard input),
     *  which is the user's screen in this case. inputBuffer by itself is the
     *  same as &inputBuffer[0], i.e. the starting address of where to store
     *  the command that is read, and length holds the number of characters
     *  read in. inputBuffer is not a null terminated C-string.
     */
    start = -1;
    if (length == 0)
        exit(0);            /* ^d was entered, end of user command stream */

    /**
     * the <control><d> signal interrupted the read system call
     * if the process is in the read() system call, read returns -1
     * However, if this occurs, errno is set to EINTR. We can check this  value
     * and disregard the -1 value
     */

    if ((length < 0) && (errno != EINTR))
    {
        perror("error reading the command");
        exit(-1);           /* terminate with error code of -1 */
    }

    /**
     * Parse the contents of inputBuffer
     */

    for (i = 0; i < length; i++)
    {
        /* examine every character in the inputBuffer */

        switch (inputBuffer[i])
        {
            case ' ':
            case '\t' :               /* argument separators */
                if (start != -1)
                {
                    args[ct] = &inputBuffer[start];    /* set up pointer */
                    ct++;
                }
                inputBuffer[i] = '\0'; /* add a null char; make a C string */
                start = -1;
                break;

            case '\n':                 /* should be the final char examined */
                if (start != -1)
                {
                    args[ct] = &inputBuffer[start];
                    ct++;
                }
                inputBuffer[i] = '\0';
                args[ct] = NULL; /* no more arguments to this command */
                break;
            default :             /* some other character */
                if (start == -1)
                    start = i;
                if (inputBuffer[i] == '&')
                {
                    *background = 1;
                    inputBuffer[i - 1] = '\0';
                }
        } /* end of switch */
    }    /* end of for */

    /**
     * If we get &, don't enter it in the args array
     */

    if (*background)
        args[--ct] = NULL;

    args[ct] = NULL; /* just in case the input line was > 80 */

    return 1;

} /* end of parseCommand routine */