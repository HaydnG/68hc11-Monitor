//#include <stdio.h>

#define INPUT_SIZE 32
#define COMMANDS 7
#define MIN (char *)0x400
#define MAX (char *)0x7DFF
#define NULL ((void *)0)
#define EOF (-1)
#define STACK_SIZE 872
#define PROGRAM_SIZE 0x49bc
#define LF_START (char *) (PROGRAM_SIZE + 1000)
#define LF_MAX (char *)(MAX - STACK_SIZE - 200)

typedef struct{
    char key[8];
    char usage[32];
    char description[32];
    int (*handler)(void*,int, char*, int);
}Command;

int helpHandler(Command*,int, char*, int),goHandler(Command*,int, char*, int), mmHandler(Command*,int, char*, int), dmHandler(Command*,int, char*, int),
        disHandler(Command*,int, char*, int),lfHandler(Command*,int, char*, int),
        demoHandler(Command*,int, char*, int), outputHelp(Command*),clearString(char*, int), splitArgs(char*, char**),
        validateHexArgs(Command*, char*, unsigned char**,int , int ), strToLower(char *),
        decodeInstruction(unsigned char *, char *),
        mgetchar(), trim(char*, char*), addOperand(int , char *, unsigned char *, int),
        strToHex(char *, int);

char *mgets(char*, int, int), *addSuffix(int , char *, unsigned char *, int *), *decodeMode(int , char *, unsigned char *);


void main() {
    char input[INPUT_SIZE],trimmedInput[INPUT_SIZE], *pointer;
    char **args;
    int partsCount, c, i;

    // Key, Usage, Description, Handler
    Command commands[COMMANDS] = {
            {"help"   ,"<help>"                           ,"Monitor help"             , helpHandler},   //0
            {"go"     ,"<go 'start addr'>"                ,"Execute program"          , goHandler},     //1
            {"mm"     ,"<mm 'start addr'>"                ,"Memory modify"            , mmHandler},     //2
            {"dm"     ,"<dm 'start addr'>"                ,"Display memory"           , dmHandler},     //3
            {"dis"    ,"<dis 'start addr' 'stop addr'>"   ,"Disassemble into assembly", disHandler},    //4
            {"lf"     ,"<lf 'filename'>"                  ,"Load S19 file"            , lfHandler},     //5
            {"demo"   ,"<demo>"                           ,"Stepper motor program"    , demoHandler}};  //6

    printf("\r\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
    printf("##########################################################################\n\n");
    printf("68HC11 HMonitor V1.0\n");
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

            if((partsCount = trim(input, trimmedInput)) <= 0){
                continue;
            }
            //separate trimmedInput and args with \0, args pointer set to start of args
            if (partsCount > 1){
                if(splitArgs(trimmedInput, args) == 0){
                    continue;
                }
            }

            if(strToLower(trimmedInput) == 0){
                continue;
            }

            // Handle command
            for(c = 0; c < COMMANDS; c++){
                if (!strcmp(commands[c].key, trimmedInput)){
                    if((*commands[c].handler)(commands,c,*args, partsCount) == 0){
                        printf("\nFailed to execute command");
                    }
                    break;
                }
            }
        }
    }while(1);
}

// ### Command Handlers ###

// Help
int helpHandler(Command *commands,int index, char* input, int partsCount)
/* Author Haydn Gynn
Company: Staffordshire University
Created: 04/12/2020
Purpose: Handles the help command, checks for correct usage and outputs relevant helpful command information
Functions used: outputHelp(), printf()
Version: 1.0
*/{
    if (partsCount > 1){
        printf("\nNo arguments required, Usage: %s\n", commands[index].usage);
    } // Continue command execution after warning
    outputHelp(commands);

    return 1;
}
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

// Go
int goHandler(Command *commands,int index, char* input, int partsCount)
/* Author Haydn Gynn
Company: Staffordshire University
Created: 04/12/2020
Purpose: Handles the go command,
            Executes the specified place in memory
Functions used: validateHexArgs()
Version: 1.0
*/{
    unsigned char*(*startPos)();

    if(!validateHexArgs(&commands[index], input, (unsigned char **) &startPos, partsCount, 1)) return 0;

    startPos();

    return 1;
}

// Memory modify
int mmHandler(Command *commands,int index, char* input, int partsCount)
/* Author Haydn Gynn
Company: Staffordshire University
Created: 04/12/2020
Purpose: Handles the modify memory command,
            Allows the modifying of memory, byte by byte. Terminating with '.'
Functions used: printf(), validateHexArgs(), sscanf(), mgets()
Version: 1.0
*/{
    char hexInput[3];
    unsigned char *startPos;
    unsigned int value;

    if(!validateHexArgs(&commands[index], input, &startPos,partsCount, 1)) return 0;

    printf("Address     Hex Data\n");
    do{
        printf("%04X      %02X    : ", startPos, *startPos);

        if(mgets(hexInput,2,1) !=NULL){//InstantMode enabled for quick input
            if(hexInput[0] == '.'){
                break;
            }else if (sscanf(hexInput, "%2x", &value) != 1){
                printf("\nPlease enter in either <.> to terminate <cr> to skip or <Hex data> to change");
                continue;
            }
            *startPos = value;
        }
        startPos++;
    }while (1);

    return 1;
}

// Display memory
int dmHandler(Command *commands,int index, char* input, int partsCount)
/* Author Haydn Gynn
Company: Staffordshire University
Created: 04/12/2020
Purpose: Handles the display memory command,
            Displays a specified section of memory, along with the equivalent ascii data
Functions used: printf(), validateHexArgs()
Version: 1.0
*/{
    unsigned char *startPos, *lineStart;
    int lineCount;

    if(!validateHexArgs(&commands[index], input, &startPos,partsCount, 1)) return 0;

    printf("\nAddress             Hexdata               ASCII");

    for(lineCount = 0; lineCount <= 15 && startPos <= MAX; lineCount++){
        printf("\n %04X    ", startPos);

        //Hex Output
        for(lineStart = startPos;startPos < lineStart + 10 && startPos <= MAX; startPos++){
            printf("%02X ", *startPos);
        }

        //Padding
        if (startPos != lineStart +10){
            printf("%*c", lineStart - startPos, ' ');
        }
        printf("    ");

        //ASCII output
        for(startPos = lineStart; startPos < lineStart + 10 && startPos <= MAX; startPos++){
            if (*startPos > 127 || *startPos < 32){
                printf(".");
            }else{
                printf("%c", *startPos);
            }
        }
    }

    return 1;
}

// Disassemble
int disHandler(Command *commands,int index, char* input, int partsCount)
/* Author Haydn Gynn
Company: Staffordshire University
Created: 04/12/2020
Purpose: Handles the disAssembly command,
            Disassembles machine code with a given start and end address.
Functions used: clearString(), validateHexArgs(), printf(), decodeInstruction()
Version: 1.0
*/{
    unsigned char *startPos[2];
    char instruction[16];
    int iCount = 1;

    clearString(instruction, 10);

    if(!validateHexArgs(&commands[index], input, &startPos[0],partsCount, 2)) return 0;

    printf("\n %04X                 %3d    ORG  $%04X",startPos[0],iCount++, startPos[0]);
    while(startPos[0] <= startPos[1]){
        printf("\n %04X  ", startPos[0]);

        startPos[0] += decodeInstruction(startPos[0], instruction);

        printf("%3d    %s",iCount++, instruction);
    }
    printf("\n %04X                 %3d   END", startPos[0],iCount);
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
*/
{
    int offset = 0, before = 0, is16Bit = 0;
    char *instructionStart = instruction;

    //Prefix
    while(instructionStart == instruction){
        if (offset > 3) break;

        printf("%02X ", *pos);
        switch (*pos & 0x85) {
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
    switch (*pos & 0xCF) {
        case 0xC3:
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
            *is16Bit = 1;
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
Functions used: sprintf()
Version: 1.0
*/{
    int mask, offset = 0;
    mask = *pos & 0x30;
    if(mask == 0x00){
        instruction += sprintf(instruction, "#");
        if(is16Bit == 1 && *(pos+1) == 0){
            printf("%02X ", *(pos + 1));
            offset++;
            pos++;
        } else{
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
    }else if((is16Bit == 0 && mask == 0x00) || mask == 0x30){
        instruction += sprintf(instruction, "%02X", *(pos + 2));
        printf("%02X ", *(pos + 2));
        offset++;
    }
    return ++offset;
}

// Load file
int lfHandler(Command *commands,int index, char* input, int partsCount)
/* Author Haydn Gynn
Company: Staffordshire University
Created: 04/12/2020
Purpose: Handle the load file command.
            Decodes an S record file, loading it into memory
Functions used: printf(), strToHex(), mgetchar()
Version: 1.0
*/{
    int data, lineLength, count = 0, checksum = 0, sum, lineCount = 1, mode;
    char buffer[10],*startAddr,*pointer;

    printf("\n%*cMotorola S decoder program\n%*c%*c\n\nStart the download for the file\n\n", 10, ' ', 10, ' ', 10, '_');

    while(1){
        //Read in first 8 characters, ensuring first value is an 'S'
        while (count < 8){
            buffer[count++] = mgetchar();
            if(buffer[0] != 'S'){
                count = 0;
            }
        }
        mode = buffer[1];
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

        sum = ((int)startAddr & 0xFF) + ((int)startAddr >> 8) + lineLength;
        pointer = startAddr;
        //Read the rest of the line
        while (startAddr < (pointer + lineLength-2)){
            buffer[0] = mgetchar();
            buffer[1] = mgetchar();
            if ((data = strToHex(buffer, 1)) == -1){
                printf("\nInvalid hex digits in the 'data' - Line: %d - (%c%c)",lineCount, buffer[0],buffer[1]);
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

        if(mode == '9'){
            printf("\n\nFile sucessfully uploaded. Start address: %X", pointer);
            break;
        }
        lineCount++;
        count = 0;
    }
    return 1;
}

int strToHex(char *start, int bytes)
/* Author Haydn Gynn
Company: Staffordshire University
Created: 04/12/2020
Purpose: A specialised lightweight function used when loading a file.
        Converts an ascii string into a validated hex value.
        Supports multiple byte hex values.
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

// Demo
int demoHandler(Command *commands,int index, char* input, int partsCount)
/* Author Haydn Gynn
Company: Staffordshire University
Created: 04/12/2020
Purpose: Handle the demo command.
            A simple program which uses a potentiometer to control the speed of a motor
Version: 1.0
*/{
    unsigned char * portA, *ddrA, *adctl,*adr1;
    int counter, delay;

    adctl=(unsigned char*)0x30;
    *adctl=0x20;
    adr1=(unsigned char*)0x31;
    portA=(unsigned char *)0x00;	/*Port A Data register*/
    ddrA=(unsigned char *)0x01;	  /*Port A Data Direction register*/
    *ddrA = 0x0F; /* PortA Input=0/Output=1 */

    for(;;){
        delay = (*adr1 * 8);
        if(delay < 50){
            delay = 50;
        }else if(delay > 2000){
            delay = 2000;
        }
        *portA = 0x01;
        for(counter = 0; counter < delay; counter++);
        *portA = 0x03;
        for(counter = 0; counter < delay; counter++);
        *portA = 0x02;
        for(counter = 0; counter < delay; counter++);
        *portA = 0x06;
        for(counter = 0; counter < delay; counter++);
        *portA = 0x04;
        for(counter = 0; counter < delay; counter++);
        *portA = 0x0C;
        for(counter = 0; counter < delay; counter++);
        *portA = 0x08;
        for(counter = 0; counter < delay; counter++);
        *portA = 0x09;
        for(counter = 0; counter < delay; counter++);
    }
}


// ### Helper Functions ###
int validateHexArgs(Command *command, char* input, unsigned char** args,int partsCount, int expectedArgs)
/* Author Haydn Gynn
Company: Staffordshire University
Created: 04/12/2020
Functions used: sscanf(), printf()
Purpose: a universal function for parsing the hex command parameters,
        Expects memory from the args pointer to already be allocated with enough room to store the expectedArgs.
        Validates input is correct Hex, and ensures its within the correct range.
Version: 1.0
*/{
    int i;

    if (partsCount <= expectedArgs || partsCount > expectedArgs + 1){
        printf("\nIncorrect usage. Please use %s", command->usage);
        return 0; // Stop command execution, ensure correct usage.
    }

    // Currently only accommodates a maximum of 3 args inputs, can be expanded upon
    if (sscanf(input, "%x %x %x", args, args + 1, args + 2) != expectedArgs){
        printf("\nAddress must be in hex i.e 0-9 A-F");
        return 0;
    }

    for(i = 0;i < expectedArgs; i++){
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