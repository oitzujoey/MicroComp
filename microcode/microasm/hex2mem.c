
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int strSplit(char ***, int *, char *, const char *);
char *readFile(char *);

int main(int argc, char **argv) {
    
    char *src;
    char **tokens;
    int tokenLen;
    FILE *destFile;
    
    if (1 < argc) {
        src = readFile(argv[1]);
    } else {
        fprintf(stderr, "ERROR: Source filename required.\n");
        exit(1);
    }
    
    if (0 < strSplit(&tokens, &tokenLen, src, "\n ")) {
        perror("ERROR");
        abort();
    }
    
    if (2 < argc) {
        destFile = fopen(argv[2], "w");
    } else {
        fprintf(stderr, "ERROR: Destination filename required.\n");
    }
    
    for (int i = 0; i < tokenLen; i++) {
        fprintf(destFile, "%s\n", tokens[i]);
    }
    
    free(src);
    free(tokens);
    
    fclose(destFile);
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
