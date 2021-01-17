
#define INPUT_SIZE 32
#define COMMANDS 7
#define MAX_ARGS 2
#define MIN (char *)0x400
#define MAX (char *)0x7DFF
#define NULL ((void *)0)
#define EOF (-1)
#define STACK_SIZE 962
#define PROGRAM_SIZE 0x5756
#define LF_START (char *) (PROGRAM_SIZE + 500)
#define LF_MAX (char *)(MAX - STACK_SIZE - 200)
#define POT_MIDPOINT 1000
#define VERSION "1.1"

/*Author: Haydn Gynn
Company: Staffordshire University
Date: 05/12/2020
Version 1.1
Purpose: 68HC11 Monitor program
            Implements: Help
                        Go
                        MM
                        DM
                        DIS
                        LF
                        DEMO

Updates:
    Version     Author          Date            Purpose
    1.0         Haydn Gynn      05/12/2020      Initial Version
    1.1         Haydn Gynn      05/01/2021      Fixed bugs raised during testing
*/


typedef struct{
    int index;
    char key[8];
    char usage[32];
    char description[32];
    int (*handler)(void*, int, unsigned char**);
    int params;
}Command;

int goHandler(Command*, int, unsigned char** args), go(unsigned char *arg);
int helpHandler( Command*, int,  unsigned char** args), outputHelp(Command*);
int mmHandler(Command*, int, unsigned char** args), mm(unsigned char *arg, int);
int dmHandler(Command*, int, unsigned char** args), dm(unsigned char *arg, int);
int disHandler(Command*, int, unsigned char** args), dis(unsigned char *start, unsigned char *end);
int lfHandler(), lf();
int demoHandler(), demo();
int handleCommand(Command*, char*), clearString(char*, int), splitArgs(char*, char**),
        validateHexArgs(Command*, char*, unsigned char**,int), strToLower(char *),
        decodeInstruction(unsigned char *, char *),
        mgetchar(), trim(char*, char*), addOperand(int , char *, unsigned char *, int),
        strToHex(char *, int);

char *mgets(char*, int, int), *addSuffix(int , char *, unsigned char *, int *);


void main() {
    char input[INPUT_SIZE];
    int c;

    // Key, Usage, Description, Handler
    Command commands[COMMANDS] = {
            {0,"help"   ,"<help>"                           ,"Monitor help"             , helpHandler,    0},   //0
            {1,"go"     ,"<go 'start addr'>"                ,"Execute program"          , goHandler,      1},   //1
            {2,"mm"     ,"<mm 'start addr'>"                ,"Memory modify"            , mmHandler,      1},   //2
            {3,"dm"     ,"<dm 'start addr'>"                ,"Display memory"           , dmHandler,      1},   //3
            {4,"dis"    ,"<dis 'start addr' 'stop addr'>"   ,"Disassemble into assembly", disHandler,     2},   //4
            {5,"lf"     ,"<lf>"                             ,"Load S19 file"            , lfHandler,      0},   //5
            {6,"demo"   ,"<demo>"                           ,"Stepper motor program"    , demoHandler,    0}};  //6


    printf("\r\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
    printf("##########################################################################\n\n");
    printf("68HC11 HMonitor %s\n", VERSION);
    printf("Copyright Haydn Gynn\n\n");
    printf("Type help for commands\n");
    outputHelp(commands);
    //Pad with newline based on amount of commands
    for(c = 0; c < 15 - COMMANDS; c++){
        printf("\n");
    }

    do{
        clearString(input, INPUT_SIZE);
        printf("\nCommand :> ");
        if(mgets(input,INPUT_SIZE - 1, 0) !=NULL){
            if (!handleCommand(commands, input)){
                printf("\nFailed to execute command");
            }
        }
    }while(1);
}

int handleCommand(Command *commands, char *input)
/* Author Haydn Gynn
Company: Staffordshire University
Created: 04/12/2020
Purpose: Handles the parsing and execution of a command, given a string input
Functions used: trim(), splitArgs(), strToLower(), strcmp(), validateHexArgs(), and handler command
Version: 1.0
*/{
    char trimmedInput[INPUT_SIZE];
    int partsCount, i;
    char **args;
    unsigned char *argsList[MAX_ARGS];

    if((partsCount = trim(input, trimmedInput)) <= 0){
        return 0;
    }
    //separate the command and args with \0, args pointer set to start of args string
    if (partsCount > 1){
        if(splitArgs(trimmedInput, args) == 0){
            return 0;
        }
    }

    if(strToLower(trimmedInput) == 0){
        return 0;
    }

    // Handle command - Using the commandHandler approach allows the monitor to be easily expanded up by adding more commands
    // It allows easy dropping in of commands, provided they have the correct definitions
    // Although a lot may seem like unnecessary code, especially for a monitor program that is required to be small,
    // i believe a method like this might be more maintainable and allow for future expansions
    for(i = 0; i < COMMANDS; i++){
        if (!strcmp(commands[i].key, trimmedInput)){

            if(!validateHexArgs(&commands[i], *args, (unsigned char **)&argsList, partsCount)){
                return 0;
            }

            //Execute commandHandler function found
            return (*commands[i].handler)(commands, i, (unsigned char **)&argsList);
        }
    }
}

// ### Command Handlers ###

// Help
int helpHandler(Command *commands, int index, unsigned char** args)
/* Author Haydn Gynn
Company: Staffordshire University
Created: 04/12/2020
Purpose: Handles the help command
Functions used: outputHelp()
Version: 1.0
*/{
    return outputHelp(commands);
}

// Go
int goHandler(Command *command, int index, unsigned char** args)
/* Author Haydn Gynn
Company: Staffordshire University
Created: 04/12/2020
Purpose: Handles the go command
Functions used: go()
Version: 1.0
*/{
    return go(args[0]);
}

// Memory modify
int mmHandler(Command *command, int index, unsigned char** args)
/* Author Haydn Gynn
Company: Staffordshire University
Created: 04/12/2020
Purpose: Handles the modify memory command.
Functions used: mm()
Version: 1.0
*/{
    return mm(args[0], 1); //InstantMode enabled for quick input
}

// Display memory
int dmHandler(Command *command, int index, unsigned char** args)
/* Author Haydn Gynn
Company: Staffordshire University
Created: 04/12/2020
Purpose: Handles the display memory command,
Functions used: dm()
Version: 1.0
*/{
    return dm(args[0], 15);
}

// Disassemble
int disHandler(Command *command, int index, unsigned char** args)
/* Author Haydn Gynn
Company: Staffordshire University
Created: 04/12/2020
Purpose: Handles the disAssembly command,
Functions used: dis()
Version: 1.0
*/{
    if (args[0] > args[1]){
        printf("\nPlease ensure the End value is greater than the Start value.\n");
        return 0;
    }

    return dis(args[0], args[1]);
}

// Load file
int lfHandler()
/* Author Haydn Gynn
Company: Staffordshire University
Created: 04/12/2020
Purpose: Handle the load file command.
Functions used: lf()
Version: 1.0
*/{
    return lf();
}


// Demo
int demoHandler()
/* Author Haydn Gynn
Company: Staffordshire University
Created: 04/12/2020
Purpose: Handle the demo command.
Version: 1.0
*/{
    return demo();
}

// ########################## Commands ####################################

int outputHelp(Command *commands)
/* Author Haydn Gynn
Company: Staffordshire University
Created: 04/12/2020
Purpose: Outputs all the useful help command information.
Functions used: printf()
Version: 1.0
*/{
    int c;

    printf("\n");
    for(c = 0; c < COMMANDS; c++){
        printf("%-34s** %-34s **\n", commands[c].usage, commands[c].description);
    }

    return 1;
}

int go(unsigned char* args)
/* Author Haydn Gynn
Company: Staffordshire University
Created: 04/12/2020
Purpose: Handles the go command,
            Executes the specified place in memory
Functions used: anonymous function
Version: 1.0
*/{
    ((unsigned char *(*)()) args)();

    return 1;
}

int mm(unsigned char *startPos, int instantMode)
/* Author Haydn Gynn
Company: Staffordshire University
Created: 04/12/2020
Purpose: Allows the modifying of memory, byte by byte. Terminating with '.'
            The input mode can be changed to InstantMode(1) or slowMode(0)
                If in instantMode, it doesnt wait for carriageReturn upon entering values.
Functions used: printf(), sscanf(), mgets()
Version: 1.0
*/{
    char hexInput[3];
    unsigned int value;

    printf("Address     Hex Data\n");
    do{
        printf("%04X      %02X    : ", startPos, *startPos);

        if(mgets(hexInput,2,instantMode) !=NULL){
            if(hexInput[0] == '.' || hexInput[1] == '.'){
                break;
            }else if (sscanf(hexInput, "%2x", &value) != 1){
                printf("\nPlease enter in <.> to terminate, <cr> to skip, <Hex data> to input\n");
                continue;
            }
            *startPos = value;
        }
        startPos++;
        if (startPos < MIN || startPos > MAX){
            printf("\nCannot surpass maximum address (%04X)", MAX);
            return 1;
        }

    }while (1);

    return 1;
}

int dm(unsigned char *pointer, int lineCount)
/* Author Haydn Gynn
Company: Staffordshire University
Created: 04/12/2020
Purpose: Displays a specified section of memory based on the startPointer and how many lines specified, along with the equivalent ascii data
Functions used: printf()
Version: 1.0
*/{
    unsigned char *lineStart;

    printf("\nAddress             Hexdata               ASCII");

    for(lineCount = 0; lineCount <= 15 && pointer <= MAX; lineCount++){
        printf("\n %04X    ", pointer);

        //Hex Output
        for(lineStart = pointer;pointer < lineStart + 10 && pointer <= MAX; pointer++){
            printf("%02X ", *pointer);
        }

        //Padding
        if (pointer != lineStart + 10){
            printf("%*c", (10 - (pointer - lineStart)) * 3, ' ');
        }
        printf("    ");

        //ASCII output
        for(pointer = lineStart; pointer < lineStart + 10 && pointer <= MAX; pointer++){
            if (*pointer > 127 || *pointer < 32){
                printf(".");
            }else{
                printf("%c", *pointer);
            }
        }
    }

    return 1;
}

dis(unsigned char* start, unsigned char* end)
/* Author Haydn Gynn
Company: Staffordshire University
Created: 04/12/2020
Purpose: Disassembles machine code with a given start and end address.
Functions used: clearString(), printf(), decodeInstruction()
Version: 1.0
*/{
    char instruction[16];
    int iCount = 1;

    clearString(instruction, 10);

    printf("\n %04X                 %3d    ORG  $%04X",start,iCount++, start);
    while(start <= end){
        printf("\n %04X  ", start);

        start += decodeInstruction(start, instruction);

        printf("%3d    %s",iCount++, instruction);
    }
    printf("\n %04X                 %3d   END", start,iCount);
    return 1;
}

int decodeInstruction(unsigned char *pos, char *instruction)
/* Author Haydn Gynn
Company: Staffordshire University
Created: 04/12/2020
Purpose: Used by disHandler
            Given a start address, the machine code will be disassembled into assembly.
            Returning back how many bytes were consumed within the single command,
            signaling how many bytes to jump ahead for the next command.
Functions used: sprintf(), addSuffix(), addOperand(), printf()
Version: 1.0
*/{
    int offset = 0, before = 0, is16Bit = 0;
    char *instructionStart = instruction;

    //Prefix
    while(instructionStart == instruction){
        if (offset > 3) break;
        printf("%02X ", *pos);
        switch (*pos & 0x85) { // Apply different masks to find first part of command
            case 0x84:
                instruction += sprintf(instruction,"ld");
                break;
            case 0x85:
                if(*pos == 0xCD){
                    before = *pos;
                    pos++;
                    offset++;
                }else{
                    instruction += sprintf(instruction,"st");
                }
                break;
            case 0x80:
                instruction += sprintf(instruction,"sub");
                break;
            case 0x81:
                if ((*pos & 0xCF) == 0x83){
                    instruction += sprintf(instruction,"sub");
                }else {
                    instruction += sprintf(instruction, "ad");
                }
                break;
            case 0x00:
                before = *pos;
                pos++;
                offset++;
                break;
            default:
                printf("%*c", (4 * 3)-(offset * 3), ' ');
                sprintf(instruction,"        NULL");
                return ++offset;
        }
    }
    //Suffix
    instruction = addSuffix(before, instruction, pos, &is16Bit);
    //Padding
    instruction += sprintf(instruction, "%*c", 5-(instruction - instructionStart), ' ');
    //AddressingMode operand
    offset += addOperand(before, instruction, pos, is16Bit);

    //Padding
    printf("%*c", (4 * 3)-(offset * 3), ' ');

    return ++offset;
}

char *addSuffix(int before, char * instruction, unsigned char *pos, int * is16Bit)
/* Author Haydn Gynn
Company: Staffordshire University
Created: 04/12/2020
Purpose: Used by decodeInstruction
            Adds the suffix (CommandType / register) onto the assembly instruction
Functions used: sprintf()
Version: 1.0
*/{
    switch (*pos & 0xCF) { //Apply mask to find suffix of command
        case 0xC3:
            *is16Bit = 1;
            instruction += sprintf(instruction,"dd");break;
        case 0xCB:
            instruction += sprintf(instruction,"db");break;
        case 0x89:
            instruction += sprintf(instruction,"ca");break;
        case 0xC9:
            instruction += sprintf(instruction,"cb");break;
        case 0x8B:
            instruction += sprintf(instruction,"da");break;
        case 0xC0:
            instruction += sprintf(instruction,"b");break;
        case 0x80:
            instruction += sprintf(instruction,"a");break;
        case 0x87:case 0x86:
            instruction += sprintf(instruction,"aa");break;
        case 0xC7:case 0xC6:
            instruction += sprintf(instruction,"ab");break;
        case 0xCC:case 0xCD:case 0x83:
            *is16Bit = 1; //Mask as that command uses a 16bit register
            instruction += sprintf(instruction,"d");break;
        case 0x8E:case 0x8F:
            *is16Bit = 1;
            instruction += sprintf(instruction,"s");break;
        case 0xCF:case 0xCE:
            if (before == 0 || before == 0xCD){
                instruction += sprintf(instruction,"x");
            }else{
                instruction += sprintf(instruction,"y");
            }
            *is16Bit = 1;
            break;
    }
    return instruction;
}

int addOperand(int before, char * instruction, unsigned char *pos, int is16Bit)
/* Author Haydn Gynn
Company: Staffordshire University
Created: 04/12/2020
Purpose: Used by decodeInstruction
            Adds the operand (Value) onto the assembly instruction
            Figures out how to display the numerical part of the command
Functions used: sprintf()
Version: 1.0
*/{
    int mask, offset = 0;
    mask = *pos & 0x30;
    if(mask == 0x00){ // IMM mode
        instruction += sprintf(instruction, "#");
        if(is16Bit == 1 && *(pos+1) == 0){ //If data is 0010 only show as 10 (skips over byte)
            printf("%02X ", *(pos + 1));
            offset++;
            pos++;
            is16Bit = 0;
        }
    }
    instruction += sprintf(instruction, "$%02X", *(pos + 1));
    printf("%02X ", *(pos + 1));
    if(mask == 0x20){
        if (before == 0 || before == 0x1A){ //IND X
            instruction += sprintf(instruction, ",X");
        } else if(before == 0x18 || before == 0xCD){ //IND Y
            instruction += sprintf(instruction, ",Y");
        }
    }else if((is16Bit == 1 && mask == 0x00) || mask == 0x30){ //2 BYTE data (EXT and REG D, S, LDX, LDY)
        instruction += sprintf(instruction, "%02X", *(pos + 2));
        printf("%02X ", *(pos + 2));
        offset++;
    }
    return ++offset;
}

int lf()
/* Author Haydn Gynn
Company: Staffordshire University
Created: 04/12/2020
Purpose: Decodes an S record file, loading it into memory
Functions used: printf(), strToHex(), mgetchar()
Version: 1.0
*/{
    int data, lineLength, count = 0, checksum = 0, sum, lineCount = 1;
    char buffer[10],*startAddr,*pointer, *lastAddr;

    printf("\n%*cMotorola S decoder program\n", 10, ' ');
    printf("%*c______________________", 12, ' ');
    printf("%*c\n\n", 10, '_');
    printf("Start the download for the file (Min Address: %04X, Max Address: %04X)\n\n", LF_START, LF_MAX);

    while(1){
        count = 0;
        //Read in first 8 characters, ensuring first value is an 'S'
        while (count < 8){
            buffer[count++] = mgetchar();
            if(buffer[0] != 'S'){
                count = 0;
            }
        }
        if(buffer[1] != '1' && buffer[1] != '9'){
            printf("\nInvalid start of line - Line: %d",lineCount);
            return 0;
        }

        lineLength = strToHex(buffer + 2, 1);
        startAddr = (char *) strToHex(buffer + 4, 2);

        if (lineLength == -1 || startAddr == (char *)-1){
            printf("\nInvalid hex digits in 'length' or 'start address' - Line: %d",lineCount);
            return 0;
        }
        if (startAddr < LF_START || startAddr > LF_MAX){
            printf("\nThe line startAddress(%X) is out of bounds (%04X -> %04X) - Line: %d", startAddr,LF_START, LF_MAX, lineCount);
            return 0;
        }

        sum = ((int)startAddr >> 8) + ((int)startAddr & 0xFF)  + lineLength;
        pointer = startAddr;
        //Read the rest of the line
        while (startAddr < (pointer + lineLength-2)){
            buffer[8] = mgetchar();
            buffer[9] = mgetchar();
            if ((data = strToHex(buffer + 8, 1)) == -1){
                printf("\nInvalid hex digits in the 'data' - Line: %d - (%c%c)",lineCount, buffer[8],buffer[9]);
                return 0;
            }
            if(startAddr == (pointer + lineLength-3)){
                checksum = data;
                break;
            }
            sum = sum + data;
            *startAddr++ = data;
        }

        sum = (~(sum)) & 0xFF;
        if (checksum == 0 || sum != checksum){
            printf("\nChecksum failed Expected: %02X, Actual: %02X - Line: %d", checksum,sum, lineCount);
            return 0;
        }

        putchar('>');
        if(buffer[1] == '9'){
            printf("\n\nFile sucessfully uploaded. Start address: %X, End address: %X", pointer, lastAddr);
            break;
        }
        lineCount++;
        lastAddr = startAddr;
    }
    return 1;
}

int demo()
/* Author Haydn Gynn
Company: Staffordshire University
Created: 04/12/2020
Purpose: A simple program which uses a potentiometer to control the speed of a motor
Version: 1.0
*/{
    unsigned char * portA, *ddrA, *adctl,*adr1;
    int timer = 0, counter = 0, sequence[8] = {1, 2, 3, 6, 4, 12, 8, 9};
    double delay;
    adctl=(unsigned char*)0x30;
    *adctl=0x20;
    adr1=(unsigned char*)0x31;
    portA=(unsigned char *)0x00;	/*Port A Data register*/
    ddrA=(unsigned char *)0x01;	    /*Port A Data Direction register*/
    *ddrA = 0x0F;                   /* PortA Input=0/Output=1 */

    printf("Motor demo (BI-Directional) - Potentiometer control\n\n");
    printf("Plug RED Power wire to 5Volt on board\n");
    printf("Plug BLACK Power wire to GND on board\n\n");
    printf("Plug Stepper RED wire to PORT A0\n");
    printf("Plug Stepper WHITE wire to PORT A1\n");
    printf("Plug Stepper GREEN wire to PORT A2\n");
    printf("Plug Stepper BLACK wire to PORT A3\n\n");
    printf("Plug Potentiometer RED Power wire to 5Volt on board\n");
    printf("Plug Potentiometer BLACK Power wire to GND on board\n");
    printf("Plug Potentiometer GREEN wire to E0\n\n");
    printf("Watch motor spin, the potentiometer will alter speed and direction\n");

    for(;;){
        while(((*adctl) & 0x80) == 0x00);
        delay = (*adr1 * 9);

        *portA = sequence[counter];

        //Alter direction of motor
        counter += (delay > POT_MIDPOINT) ? 1 : -1;

        if(counter > 8){
            counter = 0;
        } else if(counter < 0){
            counter = 8;
        }
        //flatten curve at bottom
        delay = delay >= 800 && delay <= 1200 ? 1000 : delay;
        //Map value onto a curve, scales the inputs to more reasonable values.
        //The curve allows the speed control for the forward/reverse options
        delay = ((delay - POT_MIDPOINT) * (delay - POT_MIDPOINT)) / 15000;

        for(timer = 0; timer < delay; timer++);

    }
}

// ################# Helper Functions ######################

int strToHex(char *start, int bytes)
/* Author Haydn Gynn
Company: Staffordshire University
Created: 04/12/2020
Purpose: A specialised lightweight function used when loading a file.
        Converts an ascii string into a validated hex value.
        Supports multiple byte hex values.
        Provide a start address, and how many bytes the Hex number is expected to have
Version: 1.0
*/
{
    int total = -1, value;
    char *p;

    for (p = start;start <= p + ((bytes*2)-1); start++) {
        value = *start;
        if(value >= 'A' && value <= 'F'){
            value = value - 55;
        }else if(value >= '0' && value <= '9'){
            value = value - 48;
        } else{
            return -1;
        }
        if(total != -1){
            total = (total << 4) + value;
        } else{
            total = value;
        }
    }
    return total;
}

int validateHexArgs(Command *command, char* input, unsigned char** args, int partsCount)
/* Author Haydn Gynn
Company: Staffordshire University
Created: 04/12/2020
Functions used: sscanf(), printf()
Purpose: a universal function for parsing the hex command parameters,
        Expects memory from the args pointer to already be allocated with enough room to store all the params.
        Validates input is correct Hex digits, and ensures its within the correct range.
Version: 1.0
*/{
    int i, bytes = 0;

    if (partsCount == 1 && command->params == 0){
        return 1;
    }

    if (partsCount > command->params + 1){
        printf("\nToo many arguments specified, Usage: %s\n", command->usage);
        printf("\nContinuing command execution\n");
        if (command->params == 0){
            return 1;
        }
    } // Continue command execution after warning

    if (partsCount <= command->params){
        printf("\nIncorrect usage. Please use %s", command->usage);
        return 0; // Stop command execution, ensure correct usage.
    }

    //Max 3 number of reads currently, can increase if needed


    for(i = 0; i < command->params; i++){
        if (sscanf(input, "%x%n", args + i, &bytes) != 2){
            printf("\nAddress must be in hex i.e 0-9 A-F");
            return 0;
        }
        input += bytes;

        if (*(args + i) < MIN || *(args + i) > MAX){
            printf("\nThe address range is 400 -> 7DFF");
            return 0;
        }
    }

    return 1;
}

char *mgets(char *pointer, int maxlength, int instantMode)
/* Author Haydn Gynn
Company: Staffordshire University
Created: 27/02/2019
Modified: 28/11/2020
Functions used: mgetchar()
Purpose: Custom gets function, only captures string for the specified length.
		Enables backspace to work.
		Prevents exceeding stated length.
		Function modified to fit current purpose.
		Contains 'instantMode', this means it will not wait for \n  even after the limit is reached.
Version: 2.0
*/
{
    char *string = pointer;
    int Input, length = 0;

    while(1){
        if ((Input = mgetchar()) == EOF)
            return (NULL);
        if (Input == '\n'){
            putchar(Input);
            break;
        }else if(Input == '\b'){ /*Allow backspace to work*/
            if(length <= 0) continue;
            putchar('\b');
            putchar(' ');
            putchar('\b');
            *(--string) = ' ';
            length--;
        }else if(length < maxlength){
            *string++ = Input;
            length++;
            putchar(Input);
            if(length >= maxlength && instantMode == 1) {
                putchar('\n');
                break;
            }
        }
    }

    if (pointer == string)
        return (NULL);

    *string = '\0';
    return (pointer);
}

int mgetchar()
/* Author Haydn Gynn
Company: Staffordshire University
Created: 27/02/2019
Modified: 28/11/2020
Purpose: Custom mgetchar function.
		Waits for input buffer.
		If a character is received from the input buffer, it is returned as a char.
		Modified to fit current purpose.
Version: 2.0
*/
{
    unsigned char *SCDR, *SCSR,data;

    SCDR = (unsigned char*) 0x2F;
    SCSR = (unsigned char*) 0x2E;

    while((((*SCSR) & 0x20) == 0));
    data = *SCDR;

    if (data == '\r')
        data = '\n';

    return data;
}

int trim(char *string, char * trimmedString)
/* Author Haydn Gynn
Company: Staffordshire University
Created: 13/03/2019
Modified: 28/11/2020
Purpose: Trims the string of leading and trailing spaces and returns the number of parts separated by spaces
 (e.g. "  Hi this is a test " = 5)
Version: 1.0
*/{
    char * Start = NULL,* Finish, *temp;
    int partsCount = 0;
    temp = string;

    while(*temp != '\0'){
        if(*temp != ' '){
            if((*(temp+1) == ' ' || *(temp+1) == '\0')){
                Finish = temp + 1;
                partsCount++;
            }
            if (Start == NULL){
                Start = temp;
            }
        }
        temp++;
    }
    if(Start == NULL) {
        Start = string;
        Finish = temp;
    }

    //Copy from string to trimmedString from start to finish
    while(Start != Finish){
        *trimmedString = (char)*(Start++);
        trimmedString++;
    }
    *trimmedString = '\0';

    return partsCount;
}


int splitArgs(char* command, char **args)
/* Author Haydn Gynn
Company: Staffordshire University
Created: 04/12/2020
Purpose: Splits up the given command into 'command' and 'args'.
            '\0' is used to signify the end of the command.
            Requires input to already be trimmed.
Example: Input(command = "dis 500 600") Output(command = "dis", args="500 600")
Version: 2.0
*/
{
    int pointer = 0, commandEnd = 0;

    while(command[pointer] != '\0') {
        if (command[pointer] == ' '){
            commandEnd = pointer;
            break;
        }
        pointer++;
    }

    if (command[pointer] == '\0'){
        return 0; // Command has no arguments
    }
    command[commandEnd] = '\0';

    *args = &command[commandEnd + 1];

    return 1;
}

int clearString(char *string, int length)
/* Author Haydn Gynn
Company: Staffordshire University
Date: 27/02/2019
Functions used: None
Purpose: Pre-pads out the string to remove unwanted data and to set to spaces.
Version: 1.0
*/
{
    int i;

    for(i = 0;i<length-1;i++){
        string[i] = ' ';
    }
    string[length-1] = '\0';

    return 1;
}

int strToLower(char *pointer)
/* Author Haydn Gynn
Company: Staffordshire University
Created: 04/12/2020
Purpose: Makes all characters in a string lowercase
Example: Input(HELLO) Output(hello)
Version: 1.0
*/
{
    int counter = 0;

    for(; *pointer != '\0'; pointer++){
        if (counter == 100) return 0; // Make sure loop doesnt run forever if no \0 found

        *pointer = tolower(*pointer);
        counter++;
    }

    return 1;
}