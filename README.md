# Shelldon

This is the readme file for COMP304 Project 1

This is a custom Linux shell program.

To compile shelldon.c please type the following in the shell:
$ gcc -o shelldon shelldon.c

To compile oldestchild.c just use make:
$ sudo make

For crontab, this program assumes that the user has full access to crontab file

To run shelldon type the following in the shell:
$ ./shelldon

Please note that there are some unhandled type checks, such as usage of atoi without error handling.

