# Online Quiz Simulation
This project simulates the working of an online test system that implements a central server with multiple sub-servers to handle multiple clients simultaneously.

The project was given as a Computer Networks Lab Assignment to explain the working of TCP and sockets and for practising multi-threading in C.

## Project Overview
A user can select from 3 types of tests, each with 3 sub-types corresponding to a particular subject. The system then gives a score to the user (the actual Q&A is skipped as it involves the same tasks as in the server-subserver communication).

A user is first connected to the main server to select the type of quiz. The main server then directs the user (creates a new connection) to the appropriate sub server. A sub server is made to handle one particular type (not sub-type) of a quiz. The actual quiz takes place at one of the sub-servers.

The problem statement is in the docx file.

## How to Compile and Run
- Be sure to run the server.c (main server) first, then the subservers (sub1/2/3.c), then client.
- The subservers and server files use multithreading so be sure to compile with -pthread.
- Press q then enter to quit server and subservers properly to avoid crashes.
- User input validation is not handled. User is advised to enter the correct input only.
- Enter the choice of test in all caps (MATHS, SCI, ENG). Enter the choice of subtype of test by the indicated numbers.
- Each subserver runs on a different port but same ip.
- The client-server and server-server communication is done via string messages.


Semester: Spring 2018