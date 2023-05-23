# Mini-Shell-Operating-Systems-TAU

Welcome to the Mini-Shell-Operating-Systems-TAU project! This project aims to create a simple shell program that can execute various commands and handle different types of instructions, such as regular instructions, background instructions, piping instructions, and redirecting instructions. The project was developed as part of the Operating Systems course (0368-2162) at Tel Aviv University in semester 1/2021, taught by Prof. Adam Morrison.

## Table of Contents
1. [Introduction](#introduction)
2. [Project Structure](#project-structure)
3. [Project Flow](#project-flow)
4. [Instructions for Use](#instructions-for-use)
5. [Additional Notes](#additional-notes)

## Introduction
The Mini-Shell-Operating-Systems-TAU project is developed as part of an assignment for an operating systems course. The main goal of the project is to design and implement a mini shell program capable of processing user commands and executing them accordingly.

The shell program is implemented in the C programming language and provides functionalities such as executing regular commands, running commands in the background, handling piping between commands, and redirecting command output to a file.

## Project Structure
The project consists of the following files:
1. `shell.c`: This file contains the main logic and implementation of the shell program. It includes functions for processing user commands, determining the type of instruction, executing different types of instructions, handling signals, and managing child processes.
2. `myshell.c`: This file includes the main function and acts as the entry point for the shell program. It initializes the shell and calls the necessary functions to process user commands.
3. `README.md`: This file provides an overview of the project, its structure, and instructions for use.
4. `Assignment - shell.pdf`: This file contains the assignment instructions provided for the project.

## Project Flow
The flow of the Mini-Shell-Operating-Systems-TAU project can be summarized as follows:

1. The `myshell.c` file serves as the entry point for the shell program. It initializes the shell and calls the necessary functions to process user commands.
2. The main logic of the shell program is implemented in the `shell.c` file.
3. The `prepare()` function is called before the first user command is processed. It performs any necessary preparations for the program to run smoothly.
4. The `process_arglist()` function is called to process each user command. It determines the type of instruction (regular, background, piping, or redirecting), and calls the corresponding function to execute the instruction.
5. Depending on the type of instruction, the shell program executes the command(s) and handles the execution flow accordingly. For regular instructions, it waits for the command to finish before proceeding. For background instructions, it executes the command without waiting. For piping instructions, it executes two commands concurrently, piping the output of the first command as the input of the second command. For redirecting instructions, it redirects the command output to a specified file.
6. After executing each command, the program checks for any child processes and removes zombie processes to avoid resource leakage.
7. The `finalize()` function is called at the end of the program's execution to clean up any remaining resources and perform necessary finalization steps.

## Instructions for Use
To use the Mini-Shell-Operating-Systems-TAU program, follow these steps:

1. Compile the program using the following command:
   ```
   gcc -O3 -D_POSIX_C_SOURCE=200809 -Wall -std=c11 shell.c myshell.c -o mini-shell
   ```
2. Run the compiled program by executing the following command:
   ```
   ./mini-shell
   ```
3. The shell program will start and display a prompt, indicating that it is ready to accept commands.
4. Enter commands in the provided prompt, and the shell program will execute them accordingly. The program

 supports regular commands, background commands, commands with piping, and commands with redirecting.
5. To exit the shell program, you can use the `exit` command or press Ctrl+C.


Enjoy using the Mini-Shell-Operating-Systems-TAU! If you have any further questions or need assistance, feel free to ask.
