#include <stdlib.h>
#include "cachelab.h"
#include <stdio.h>
#include <string.h>
#include <limits.h>

typedef struct {
    int tag, lastusage, valid;
} lineinfo;//Here we declared a struct in order to gather what we will store inside each of the addresses

int main(int argc, char *argv[])
{
    int noofsetindexbits = atoi(argv[2]);
    int associativity = atoi(argv[4]);
    int noofblockbits = atoi(argv[6]);//Here we take the input for the three parameters from the command line

    int nooftagbits = 64 - noofblockbits - noofsetindexbits; //we calculate the number of tagbits through knowing the number of bits for block and index
    int sizeofblock = 2 << noofblockbits;
    int sizeofset = 2 << noofsetindexbits;

    //first we need to dynamically allocate memory inside our cache simulator in order to ensure that we have
    //a valid amount of space in each line of the cache
    lineinfo *ourcache[sizeofset][associativity];
    int i, j;
    for ( i = 0; i < sizeofset; i++) {
        for ( j = 0; j < associativity; j++) {
            lineinfo *line = (lineinfo*) malloc(12);
            ourcache[i][j] = line;
        }
    }
    FILE* f;
    f = fopen(argv[8], "r");//we open the inputted file
    int timecount, hits, misses, evictions;
    hits = 0;
    misses = 0;
    evictions = 0;
    timecount = 0;
    char curline[1048];
    while(fgets(curline, 1048, f)) {
        timecount = timecount + 1;
        char instruction = curline[1];
        char *hexaddress = malloc(sizeof(char) * 20);
        char *address = malloc(sizeof(char) * 80);
        int addressno = -1;
        char* token = strtok(curline + 3, ",");
        if (curline[0] == 'I') {
            NULL;
        }
        else{
            while (token != NULL) {
                if (addressno == -1) {
                    addressno = atoi(token);
                    char* str = malloc(sizeof(char) * 100);//100 chars will be enough for any of the lines, there is no line with 25 hex decimals
                    int i;
                    i = 0;
                    while (token[i] != '\0') {
                        switch (token[i]) { 
                        case '0': 
                            strcat(str, "0000"); 
                            break; 
                        case '1': 
                            strcat(str, "0001"); 
                            break; 
                        case '2': 
                            strcat(str, "0010"); 
                            break; 
                        case '3': 
                            strcat(str, "0011"); 
                            break; 
                        case '4': 
                            strcat(str, "0100"); 
                            break; 
                        case '5': 
                            strcat(str, "0101"); 
                            break; 
                        case '6': 
                            strcat(str, "0110"); 
                            break; 
                        case '7': 
                            strcat(str, "0111"); 
                            break; 
                        case '8': 
                            strcat(str, "1000"); 
                            break; 
                        case '9': 
                            strcat(str, "1001"); 
                            break; 
                        case 'a': 
                            strcat(str, "1010"); 
                            break; 
                        case 'b': 
                            strcat(str, "1011"); 
                            break; 
                        case 'c': 
                            strcat(str, "1100"); 
                            break; 
                        case 'd': 
                            strcat(str, "1101"); 
                            break; 
                        case 'e': 
                            strcat(str, "1110"); 
                            break; 
                        case 'f': 
                            strcat(str, "1111"); 
                            break; 
                        } 
                        i = i + 1; 
                    } 
                    int zeroesaddition = 64 - strlen(token) * 4;
                    for ( i = 0; i < zeroesaddition; i++) {
                        int len = 1;
                        memmove(str + len, str, strlen(str) + 1);
                    }
                    address = str;
                }
                token = strtok(NULL, ",");
            }
            char tagdiv[nooftagbits + 1];
            strncpy(tagdiv, address, nooftagbits);
            int tag = strtol(tagdiv, NULL, 2);
            char initnumbersetup[noofsetindexbits + 1];
            strncpy(initnumbersetup, address + nooftagbits, noofsetindexbits);
            int finalnumbersetup = strtol(initnumbersetup, NULL, 2);
            char blockoffset[noofblockbits + 1];
            strncpy(blockoffset, address + nooftagbits + noofsetindexbits, noofblockbits);
            int realblockoffset = strtol(blockoffset, NULL, 2);
            int hitflag = 0;
            for ( i = 0; i < associativity; i++) {
                lineinfo *line = ourcache[finalnumbersetup][i];
                if (line->tag == tag && line->valid == 1 && (realblockoffset < sizeofblock)) {
                    line->lastusage = timecount;
                    hits++;
                    hitflag = 1;
                    break;
                }
            }
            if (hitflag) {
                if (instruction == 'M') {
                    hits++;
                }
                continue;
            }
            misses++;
            int flagadd = 0;
            for ( i = 0; i < associativity; i++) {
                lineinfo *line = ourcache[finalnumbersetup][i];
                if (line->valid == 0) {
                    line->valid = 1;
                    line->tag = tag;
                    line->lastusage = timecount;
                    flagadd = 1;
                    break;
                }
            }
            if (flagadd == 1) {
                if (instruction == 'M') {
                    hits++;
                }
                continue;
            }  
            evictions++;
            long lasttime = LONG_MAX;
            int lastlineindex;
            for ( i = 0; i < associativity; i++) {
                lineinfo *line = ourcache[finalnumbersetup][i];
                if (line->lastusage < lasttime) {
                    lasttime = line->lastusage;
                    lastlineindex = i;
                }
            }
            lineinfo *line = ourcache[finalnumbersetup][lastlineindex];
            line->valid = 1;
            line->tag = tag;
            line->lastusage = timecount;
            if (instruction == 'M') {
                hits++;
            }
        }
    }
    fclose(f);
    printSummary(hits, misses, evictions);
}