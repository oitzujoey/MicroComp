
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include <unistd.h>
#include <sys/types.h>

// #define DEBUG
#define MICROASM_VERSION "0.1"

typedef struct {
    char **bits;
    bool *invert;
} Rom;

typedef struct {
    int romnum;
    int depth;
    int width;
    Rom *rom;
} Roms;

typedef struct {
    int opnum;
    int ucodeWidth;
    int fillLength;
    char **fill;
    char **fillVals;
    int *opLength;
    int **sigLength;
    char ****signals;
    char ****values;
} Asm;

void showHelp(void);
char *readFile(char *);
void parseUasm(char *, Roms *, Asm *);
int strSplit(char ***, int *, char *, const char *);
void removeCommentsAndWhitespace(char **, const int);
int removeEmptyLines(char ***, int *);
// int max(int, int);
void generateRoms(FILE **, Roms *, Asm *, char *);

const char *fBIN = "BIN";
const char *fHEX = "HEX";
const char *fIHEX = "IHEX";

int main(int argc, char **argv) {
    
    int c;
    enum Flags {optf, opto, optm, opth};
    int *options = NULL;
    char *filename = NULL;
    char *outFilename = NULL;
    char *format = NULL;
    char *uasm = NULL;
    char *romFilename;
    FILE **romFiles;

    char ext[] = ".txt";

    Roms roms;
    Asm _asm;
    
    // const char extTxt[] = ".txt";
    // const char extHex[] = ".hex";
    char *tempPtrChar = NULL;
    
    //Get the command line options.
    options = calloc(opth + 1, sizeof(int));
    
    if (NULL == options) {
        perror("ERROR");
        abort();
    }
    
    while ((c = getopt(argc, argv, "f:o:m:h")) != -1) {
        switch (c) {
            
            case 'f':
                options[optf] = c;
                filename = optarg;
                break;
            
            case 'o':
                options[opto] = c;
                outFilename = optarg;
                break;
            
            case 'm':
                options[optm] = c;
                format = optarg;
                break;
            
            case 'h':
                options[opth] = c;
                break;
            
            case '?':
                options[optm] = c;
                if (('m' == optopt) || ('f' == optopt) || ('o' == optopt)) {
                    fprintf(stderr, "ERROR: Option -%c requires an argument.\n", optopt);
                } else if (isprint(optopt)) {
                    fprintf(stderr, "ERROR: Unknown option -%c.\n", optopt);
                } else {
                    fprintf(stderr, "ERROR: Unknown option character 0x%X.\n", optopt);
                }
                return 1;
            
            default:
                fprintf(stderr, "ERROR: Unknown option. Can't happen.\n");
                abort();
        }
    }
    
    if (options[opth]) {
        showHelp();
        exit(0);
    }
    
    if (options[optf]) {
        // printf("Filename: %s\n", filename);
    } else {
        fprintf(stderr, "ERROR: No source filename given. Use -h flag for help.\n");
        exit(1);
    }
    
    if (options[optm]) {
        // printf("Output type: %s\n", format);
    } else {
        fprintf(stderr, "Warning: No output format choosen. Assuming output is binary.\n");

        format = calloc(strlen(fBIN) + 1, sizeof(char));

        if (NULL == format) {
            perror("ERROR");
            abort();
        }

        strcpy(format, fBIN);
    }
    
    if (options[opto]) {
        // printf("Output filename: %s\n", outFilename);
    } else {
        outFilename = calloc(strlen(filename) + 1, sizeof(char));

        if (NULL == outFilename) {
            perror("ERROR");
            abort();
        }

        strcpy(outFilename, filename);

        outFilename = realloc(outFilename, (strlen(filename) + 20) * sizeof(char));

        if (NULL == outFilename) {
            perror("ERROR");
            abort();
        }

        tempPtrChar = strrchr(outFilename, '.');
        
        if (NULL == tempPtrChar) {
            tempPtrChar = tempPtrChar + strlen(tempPtrChar);
            
            fprintf(stderr, "ERROR: No output file specified and no extention on input file. "
                "The programmer was lazy and did not account for this.\n");
            exit(1);

        }
        
        if (0 == strcmp(format, fBIN)) {
            strcpy(tempPtrChar, "-*.txt");
        } else if (0 == strcmp(format, fHEX)) {
            strcpy(tempPtrChar, "-*.txt");
        } else if (0 == strcmp(format, fIHEX)) {
            strcpy(tempPtrChar, "-*.hex");
            strcpy(ext, ".hex");
        }

        fprintf(stderr, "Warning: No output file provided. Using output file \"%s\"\n", outFilename);
    }

    //Get source file.
    uasm = readFile(filename);

    // Parse the microassembly. uasm will be freed.
    parseUasm(uasm, &roms, &_asm);

    // Create ROMs and write to file.
    romFiles = calloc(roms.romnum, sizeof(FILE *));
    romFilename = calloc(strlen(outFilename) + 20, sizeof(char));

    for (int i = 0; i < roms.romnum; i++) {

        strcpy(romFilename, outFilename);

        tempPtrChar = strrchr(romFilename, '*');

        if (NULL == tempPtrChar) {
            perror("ERROR");
            abort();
        }

        sprintf(tempPtrChar, "%i%s", i + 1, ext);

        romFiles[i] = fopen(romFilename, "w");
    }

    if (!options[opto]) {
        free(outFilename);
    }
    free(romFilename);

    generateRoms(romFiles, &roms, &_asm, format);

    if (!options[optm]) {
        free(format);
    }

    free(options);

    for (int i = 0; i < roms.romnum; i++) {
        fclose(romFiles[i]);
    }

    free(romFiles);

    return 0;
}

void generateRoms(FILE **romFiles, Roms *roms, Asm *_asm, char *format) {
    
    // bool romBits[roms->romnum][_asm->opnum][roms->depth][roms->width];
    bool ****romBits = malloc(roms->romnum * sizeof(bool ***));

    bool invert;

    int hexChar;

    for (int i = 0; i < roms->romnum; i++) {

        romBits[i] = malloc(_asm->opnum * sizeof(bool **));

        if (NULL == romBits[i]) {
            perror("ERROR");
            abort();
        }

        #ifdef DEBUG
        
        printf("ROM %i\n", i + 1);
        
        #endif

        for (int j = 0; j < _asm->opnum; j++) {

            romBits[i][j] = malloc(roms->depth * sizeof(bool *));

            if (NULL == romBits[i][j]) {
                perror("ERROR");
                abort();
            }

            #ifdef DEBUG
            
            printf("\tOP %i\n", j);
            
            #endif

            for (int k = 0; k < roms->depth; k++) {

                romBits[i][j][k] = calloc(roms->width, sizeof(bool));

                if (NULL == romBits[i][j][k]) {
                    perror("ERROR");
                    abort();
                }

                // Set all signals to the inactive state.
                for (int l = 0; l < roms->width; l++) {
                    invert = roms->rom[i].invert[l];

                    romBits[i][j][k][l] = invert;
                }

                // Put the microcode into the ROMs.
                for (int l = 0; l < _asm->sigLength[j][k]; l++) {
                    if (_asm->values[j][k][l][0] == i) {

                        invert = roms->rom[i].invert[_asm->values[j][k][l][1]];

                        romBits[i][j][k][_asm->values[j][k][l][1]] = !invert;
                    }
                }

                // Fill the ROMs with fill if the byte is not a part of the microcode.
                for (int l = 0; l < _asm->fillLength; l++) {
                    if (k >= _asm->opLength[j]) {

                        invert = roms->rom[i].invert[_asm->fillVals[l][1]];

                        if (_asm->fillVals[l][0] == i) {
                            romBits[i][j][k][_asm->fillVals[l][1]] = !invert;
                        }
                    }
                }

                #ifdef DEBUG
                
                printf("\t\t");

                for (int l = 0; l < roms->width; l++) {
                    printf("%i", romBits[i][j][k][l]);
                }

                printf("\n");
                
                #endif

                // Write to file.
                if (0 == strcmp(format, fBIN)) {
                    for (int l = roms->width - 1; l >= 0; l--) {
                        fprintf(romFiles[i], "%i", romBits[i][j][k][l]);
                    }

                } else if (0 == strcmp(format, fHEX)) {

                    hexChar = 0;

                    for (int l = roms->width - 1; l >= 0; l--) {
                        hexChar += romBits[i][j][k][l] << l;
                    }
                    
                    fprintf(romFiles[i], "%02X", hexChar);

                } else if (0 == strcmp(format, fIHEX)) {
                    for (int l = roms->width - 1; l >= 0; l--) {
                        fprintf(romFiles[i], "%i", romBits[i][j][k][l]);
                    }

                } else {
                    fprintf(stderr, "ERROR: Can't happen.\n");
                    exit(1);
                }
                
                fprintf(romFiles[i], " ");
            }
            fprintf(romFiles[i], "\n");
        }
    }

    if (0 == strcmp(fIHEX, format)) {
        pid_t p = fork();

        if (p < 0) {
            fprintf(stderr, "Warning: Fork failed.\n");
        }

        if (p == 0) {
            char *args[] = {"./intel_hex_generator", NULL};
            execvp(args[0], args);
        }
    }

    // Free unneeded memory.

    for (int i = 0; i < roms->romnum; i++) {
        for (int j = 0; j < _asm->opnum; j++) {
            for (int k = 0; k < roms->depth; k++) {
                free(romBits[i][j][k]);
            }
            free(romBits[i][j]);
        }
        free(romBits[i]);
    }
    free(romBits);

    for (int i = 0; i < roms->romnum; i++) {
        free(roms->rom[i].bits);
        free(roms->rom[i].invert);
    }

    free(roms->rom);

    for (int i = 0; i < _asm->opnum; i++) {
        for (int j = 0; j < _asm->opLength[i]; j++) {
            for (int k = 0; k < _asm->sigLength[i][j]; k++) {
                free(_asm->values[i][j][k]);
            }

            free(_asm->values[i][j]);
        }

        free(_asm->values[i]);
        free(_asm->sigLength[i]);
    }
    
    free(_asm->values);
    free(_asm->sigLength);
    
    for (int i = 0; i < _asm->fillLength; i++) {
        free(_asm->fillVals[i]);
    }

    free(_asm->fillVals);

    free(_asm->opLength);

    // Create file text.
}

void parseUasm(char *uasmStr, Roms *roms, Asm *_asm) {
    
    const char ROM[] = "ROM ";
    const char ROMS[] = "ROMS ";
    const char ASM[] = "ASM ";
    const char FILL[] = "FILL ";
    const char WIDTH[] = "WIDTH ";
    const char DEPTH[] = "DEPTH ";
    const char VERSION[] = "VERSION ";

    char **lines;
    int numberOfLines = 1;

    enum MODE {mNONE, mROM, mASM};
    int mode = mNONE;

    int romIndex = 0;
    int bitIndex = 0;
    int opIndex = 0;
    int uopIndex = 0;

    int tempInt;
    char *tempPtrChar = NULL;
    
    //Split string into one array per line. Empty lines are removed.
    if (strSplit(&lines, &numberOfLines, uasmStr, "\r\n")) {
        perror("ERROR");
        abort();
    }
    
    // Remove comments and unnecessary whitespace.
    // It also converts all tabs to spaces.
    removeCommentsAndWhitespace(lines, numberOfLines);

    if(removeEmptyLines(&lines, &numberOfLines)) {
        perror("ERROR");
        abort();
    }

    // First pass: Collect size parameters.
    for (int i = 0; i < numberOfLines; i++) {

        //Get number of opcodes.
        if (0 == strncmp(lines[i], ASM, strlen(ASM))) {
            _asm->opnum = (int) strtoul(lines[i] + strlen(ASM), NULL, 0);
            // *strchr(lines[i], ' ') = '\0';

            if (0 >= _asm->opnum) {
                printf("On line %i: Number of opcodes must be a positive integer greater than zero.\n", i);
                perror("ERROR");
                exit(1);
            }

        //Get the number of ROMS
        } else if (0 == strncmp(lines[i], ROMS, strlen(ROMS))) {
            roms->romnum = (int) strtoul(lines[i] + strlen(ROMS), NULL, 0);
            // *strchr(lines[i], ' ') = '\0';

            if (0 >= roms->romnum) {
                printf("On line %i: Number of ROMs must be a positive integer greater than zero.\n", i);
                perror("ERROR");
                exit(1);
            }

        //Get the width of the ROM.
        } else if (0 == strncmp(lines[i], WIDTH, strlen(WIDTH))) {
            lines[i] += strlen(WIDTH);
            roms->width = (int) strtoul(lines[i], NULL, 0);
            lines[i][0] = '\0';

            if (0 >= roms->width) {
                printf("On line %i: Data width must be an positive integer greater than zero.\n", i);
                perror("ERROR");
                exit(1);
            }

        //Get the depth of the ROM.
        } else if (0 == strncmp(lines[i], DEPTH, strlen(DEPTH))) {
            lines[i] += strlen(DEPTH);
            roms->depth = (int) strtoul(lines[i], NULL, 0);
            lines[i][0] = '\0';

            if (0 >= roms->depth) {
                printf("On line %i: ROM depth must be an positive integer greater than zero.\n", i);
                perror("ERROR");
                exit(1);
            }

        } else if (0 == strncmp(lines[i], VERSION, strlen(VERSION))) {
            lines[i] += strlen(VERSION);
            // @FIXME Do a numeric comparison.
            if (strncmp(lines[i], MICROASM_VERSION, strlen(MICROASM_VERSION)+1)) {
                printf("Warning: Microassembler and microassembly versions do not match.\n"
                       "\tMicroassembler version: %s\n"
                       "\tMicroassembly version:  %s\n",
                       MICROASM_VERSION,
                       lines[i]);
            }
        }
    }

    #ifdef DEBUG

    printf("\nROM number is %i chips.\n", roms->romnum);
    printf("ROM width is %i bits each.\n", roms->width);
    printf("ROM depth is %i words.\n", roms->depth);
    printf("Number of opcodes is %i.\n", _asm->opnum);

    #endif

    // Allocate memory and complete the syntax structures.
    _asm->ucodeWidth = roms->romnum * roms->width;

    roms->rom = malloc(roms->romnum * sizeof(Rom));

    if (NULL == roms->rom) {
        perror("ERROR");
        abort();
    }
    
    for (int i = 0; i < roms->romnum; i++) {
        roms->rom[i].bits = calloc(roms->width, sizeof(char *));
        roms->rom[i].invert = calloc(roms->width, sizeof(bool));

        if (NULL == roms->rom[i].bits) {
            perror("ERROR");
            abort();
        }
        
        if (NULL == roms->rom[i].invert) {
            perror("ERROR");
            abort();
        }
    }

    _asm->opLength = calloc(_asm->opnum, sizeof(int));
    _asm->signals = calloc(_asm->opnum, sizeof(char ***));
    _asm->values = calloc(_asm->opnum, sizeof(int ***));
    _asm->sigLength = calloc(_asm->opnum, sizeof(int *));

    if (NULL == _asm->opLength) {
        perror("ERROR");
        abort();
    }

    if (NULL == _asm->signals) {
        perror("ERROR");
        abort();
    }

    if (NULL == _asm->values) {
        perror("ERROR");
        abort();
    }

    if (NULL == _asm->sigLength) {
        perror("ERROR");
        abort();
    }

    for (int i = 0; i < _asm->opnum; i++) {
        _asm->opLength[i] = 0;
        _asm->signals[i] = calloc(roms->depth, sizeof(char **));
        _asm->values[i] = calloc(roms->depth, sizeof(int **));
        _asm->sigLength[i] = calloc(roms->depth, sizeof(int));

        if (NULL == _asm->signals[i]) {
            perror("ERROR");
            abort();
        }

        if (NULL == _asm->values[i]) {
            perror("ERROR");
            abort();
        }

        if (NULL == _asm->sigLength[i]) {
            perror("ERROR");
            abort();
        }
    }

    // Second pass: Collect the rest of the data.
    for (int i = 0; i < numberOfLines; i++) {

        // Set mode to ROMs.
        if (0 == strncmp(lines[i], ROMS, strlen(ROMS))) {
            mode = mROM;

        // Set mode to ASM.
        } else if (0 == strncmp(lines[i], ASM, strlen(ASM))) {
            mode = mASM;

        // Fill fill with the signals to fill unused ROM with.
        } else if (0 == strncmp(lines[i], FILL, strlen(FILL))) {

            lines[i] += strlen(FILL);
            
            if (strSplit(&_asm->fill, &_asm->fillLength, lines[i], " ")) {
                perror("ERROR");
                abort();
            }

            _asm->fillVals = malloc(_asm->fillLength * sizeof(int *));

            if (NULL == _asm->fillVals) {
                perror("ERROR");
                abort();
            }

            for (int j = 0; j < _asm->fillLength; j++) {
                _asm->fillVals[j] = malloc(2 * sizeof(int));

                if (NULL == _asm->fillVals[j]) {
                    perror("ERROR");
                    abort();
                }
            }

        // Parse ROM section.
        } else if (mROM == mode) {

            if (0 == strncmp(lines[i], ROM, strlen(ROM))) {
                romIndex = (int) strtoul(lines[i] + strlen(ASM), NULL, 0) - 1;

            } else if (isdigit(lines[i][0])) {
                
                bitIndex = (int) strtoul(lines[i], &lines[i], 0);

                //Remove whitespace.
                while (' ' == lines[i][0]) {
                    lines[i]++;
                }

                // Detect inverted signals.
                if ('!' == lines[i][0]) {
                    roms->rom[romIndex].invert[bitIndex] = true;
                    lines[i]++;
                } else {
                    roms->rom[romIndex].invert[bitIndex] = false;
                }

                //Remove any junk at the end of the line.
                tempPtrChar = strchr(lines[i], ' ');

                if (NULL != tempPtrChar) {
                    *tempPtrChar = '\0';
                }

                // Put each signal into the tree.
                roms->rom[romIndex].bits[bitIndex] = lines[i];
            }

        // Parse ASM section.
        } else if (mASM == mode) {

            if (isdigit(lines[i][0])) {
                
                opIndex = (int) strtoul(lines[i], NULL, 0);
                uopIndex = 0;

            } else {
                if (strSplit(&_asm->signals[opIndex][uopIndex], &_asm->sigLength[opIndex][uopIndex], lines[i], " ")) {
                    perror("ERROR");
                    abort();
                }

                _asm->values[opIndex][uopIndex] = malloc(_asm->sigLength[opIndex][uopIndex] * sizeof(int *));

                if (NULL == _asm->values[opIndex][uopIndex]) {
                    perror("ERROR");
                    abort();
                }

                for (int j = 0; j < _asm->sigLength[opIndex][uopIndex]; j++) {
                    _asm->values[opIndex][uopIndex][j] = malloc(2 * sizeof(int));

                    if (NULL == _asm->values[opIndex][uopIndex][j]) {
                        perror("ERROR");
                        abort();
                    }
                }
                
                _asm->opLength[opIndex]++;
                uopIndex++;
            }
        }
    }
    
    // Convert the names of the signals to the positions of the signals.
    
    // Fill:
    for (int i = 0; i < _asm->fillLength; i++) {

        for (int j = 0; j < roms->romnum; j++) {
            for (int k = 0; k < roms->width; k++) {
                
                if (0 == strcmp(roms->rom[j].bits[k], _asm->fill[i])) {
                    _asm->fillVals[i][0] = j;
                    _asm->fillVals[i][1] = k;
                }
            }
        }
    }

    // Signals:
    for (int i = 0; i < _asm->opnum; i++) {
        for (int j = 0; j < roms->depth; j++) {
            for (int k = 0; k < _asm->sigLength[i][j]; k++) {

                // Don't you love this variable?
                for (int l = 0; l < roms->romnum; l++) {
                    for (int m = 0; m < roms->width; m++) {
                        
                        if (0 == strcmp(roms->rom[l].bits[m], _asm->signals[i][j][k])) {
                            _asm->values[i][j][k][0] = l;
                            _asm->values[i][j][k][1] = m;
                        }
                    }
                }
            }
        }
    }

    // Display tree.
    #ifdef DEBUG

    printf("\nROMS:\n");

    for (int i = 0; i < roms->romnum; i++) {
        printf("\tROM %i\n", i + 1);

        for (int j = 0; j < roms->width; j++) {
            printf("\t\t%s%s\n", roms->rom[i].invert[j] ? "/" : "", roms->rom[i].bits[j]);
        }
    }

    printf("\nFILL:\n\t");

    for (int i = 0; i < _asm->fillLength; i++) {
        printf("%s ", _asm->fill[i]);
    }

    printf("\n\t");

    for (int i = 0; i < _asm->fillLength; i++) {
        printf("{%i, %i} ", _asm->fillVals[i][0] + 1, _asm->fillVals[i][1]);
    }

    printf("\n\nASM:\n");

    for (int i = 0; i < _asm->opnum; i++) {
        printf("\tOP %i\n", i);

        for (int j = 0; (j < roms->depth) && (NULL != _asm->signals[i][j]); j++) {
            printf("\t\t%i ", j);

            for (int k = 0; k < _asm->sigLength[i][j]; k++) {
                printf("%s ", _asm->signals[i][j][k]);
            }
            
            printf("\n\t\t%i ", j);

            for (int k = 0; k < _asm->sigLength[i][j]; k++) {
                printf("{%i, %i} ", _asm->values[i][j][k][0] + 1, _asm->values[i][j][k][1]);
            }
            putchar('\n');
        }
    }
    putchar('\n');

    #endif
    
    // Free unneeded memory.

    free(uasmStr);
    free(lines);

    free(_asm->fill);

    for (int i = 0; i < _asm->opnum; i++) {
        for (int j = 0; j < roms->depth; j++) {
            free(_asm->signals[i][j]);
        }

        free(_asm->signals[i]);
    }

    free(_asm->signals);
}

// int max(int a, int b) {
//     return (a > b) ? a : b;
// }

int removeEmptyLines(char ***linesPtr, int *numberOfLinesPtr) {

    int replaceLine = 0;
    int sourceLine = 0;
    char *tempPtrChar = NULL;
    int newLength = 0;

    char **lines = *linesPtr;
    int numberOfLines = *numberOfLinesPtr;

    // Squish the lines together.
    while (replaceLine < numberOfLines) {
        if ('\0' == lines[replaceLine][0]) {

            sourceLine = replaceLine + 1;
            
            while (sourceLine < numberOfLines) {
                if ('\0' != lines[sourceLine][0]) {
                    
                    tempPtrChar = lines[sourceLine];
                    lines[sourceLine] = lines[replaceLine];
                    lines[replaceLine] = tempPtrChar;
                    break;
                }
                sourceLine++;
            }
        }
        replaceLine++;
    }

    // Calculate the new length of the array.
    for (newLength = 0; '\0' != lines[newLength][0]; newLength++);

    //Reduce the size of the array.
    *linesPtr = realloc(lines, newLength * sizeof(char *));
    *numberOfLinesPtr = newLength;

    if (NULL == *linesPtr) {
        return 1;
    }

    return 0;
}

void removeCommentsAndWhitespace(char **lines, int numberOfLines) {
    
    char *tempPtrChar = NULL;
    char *tempPtrChar1 = NULL;
    bool blockComment = false;
    
    for (int i = 0; i < numberOfLines; i++) {
        
        //Remove ';' comments
        tempPtrChar = strchr(lines[i], ';');
        
        if (NULL != tempPtrChar) {
            *tempPtrChar = '\0';
        }
        
        //Remove '//' comments
        tempPtrChar = strstr(lines[i], "//");
        
        if (NULL != tempPtrChar) {
            *tempPtrChar = '\0';
        }
        
        //Remove block comments.
        if (!blockComment) {
            tempPtrChar = strstr(lines[i], "/*");
            
            if (NULL != tempPtrChar) {
                *tempPtrChar = '\0';
                blockComment = true;
            }
        } else {
            tempPtrChar = strstr(lines[i], "*/");
            
            if (NULL != tempPtrChar) {
                lines[i] = tempPtrChar + strlen("*/");
                blockComment = false;
            } else {
                lines[i][0] = '\0';
            }
        }

        //Remove leading whitespace.
        while (isspace(*lines[i])) {
            lines[i]++;
        }

        //Remove trailing whitespace.
        tempPtrChar = lines[i] + strlen(lines[i]) - 1;
        
        while ((tempPtrChar >= lines[i]) && isspace(*tempPtrChar)) {
            tempPtrChar--;
        }

        tempPtrChar[1] = '\0';

        //Leave only one space in between words.
        tempPtrChar = lines[i];

        while ('\0' != *tempPtrChar) {
            if (isspace(*tempPtrChar)) {

                *tempPtrChar = ' ';

                tempPtrChar1 = tempPtrChar;

                while (('\0' != *tempPtrChar1) && isspace(*tempPtrChar1)) {
                    tempPtrChar1++;
                }

                memmove(tempPtrChar + 1, tempPtrChar1, (strlen(tempPtrChar1) + 1) * sizeof(char));
            }
            tempPtrChar++;
        }
    }
}

/*  Split the string into an array of tokens. 
    The original string may be modified. */
int strSplit(char ***tokens, int *length, char *str, const char *delimiters) {
    
    char **lines;
    char *lnPtr;
    int numberOfLines = 1;
    
    //Count the number of lines in the file.
    for (int i = 0; str[i] != '\0'; i++) {
        for (int j = 0; j < strlen(delimiters); j++) {
            if (delimiters[j] == str[i]) {
                numberOfLines++;
            }
        }
    }
    
    //Allocate that many elements for the array of lines.
    lines = malloc(numberOfLines * sizeof(char *));
    
    if (NULL == lines) {
        return 1;
    }
    
    /*  Initialize each element of lines to NULL since strtok
        will delete empty lines, which is good, I guess. */
    for (int i = 0; i < numberOfLines; i++) {
        lines[i] = NULL;
    }
    
    //Split the string.
    lnPtr = strtok(str, delimiters);
    
    for (int i = 0; lnPtr != NULL; i++) {
        
        lines[i] = lnPtr;
        lnPtr = strtok(NULL, delimiters);
    }
    
    //lines is probably too long, so lets reduce the size.
    if ((NULL == lines[numberOfLines - 1])) {
        for (int i = numberOfLines - 1; i >= 0; i--) {
            if (NULL == lines[i]) {
                numberOfLines--;
            }
        }
        
        lines = realloc(lines, numberOfLines * sizeof(char *));
        
        if (NULL == lines) {
            return 1;
        }
    }
    
    //Return the strings and the length of the array.
    *length = numberOfLines;
    *tokens = lines;
    
    return 0;
}

char *readFile(char *filename) {
    FILE *infile;
    char *buffer;
    long numbytes;
    
    //Open file.
    infile = fopen(filename, "r");
    
    if (NULL == infile) {
        perror("ERROR");
        abort();
    }
    
    //Get size of file and allocate memory for buffer.
    fseek(infile, 0L, SEEK_END);
    numbytes = ftell(infile);
    
    fseek(infile, 0L, SEEK_SET);
    
    buffer = calloc(numbytes + 1, sizeof(char));
    
    if (NULL == buffer) {
        perror("ERROR");
        abort();
    }
    
    //Read file into buffer.
    fread(buffer, sizeof(char), numbytes, infile);
    fclose(infile);
    
    return buffer;
}

void showHelp() {
    printf(
        "Options:\n"
        "    -f file     Microassembly file\n"
        "    -o file     Output file\n"
        "    -m format   Output format\n"
        "                Default format is raw binary.\n"
        "                    BIN     binary (default)\n"
        "                    HEX     hex\n"
        "                    IHEX    Intel HEX\n"
        "    -h          Help\n"
    );
}
