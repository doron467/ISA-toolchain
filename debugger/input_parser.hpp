#ifndef INPUT_PARSER_H
#define INPUT_PARSER_H

#define MAX_STEP 500

enum Action {
    INVALID,
    START, // set pc to the entry address of the program
    STEP, // step one assembly instruction
    RUN, // run the program until end or break point
    CPU_INFO, // print cpu info (registers and stuff)
    DISASSEMBLE, // print the disassembled code
    BREAK, // add a break point
    QUIT // quit from the debugger
};

typedef struct _Command {
    Action action;
    int counter;
    int offset;
} Command;

Command getNextCommand();

#endif