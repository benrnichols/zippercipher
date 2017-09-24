#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
//#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
//all the sboxes
const int s1[16] = {0xD, 0x3, 0x1, 0xF, 0xD, 0xB, 0x2, 0x9, 0xC, 0xA, 0x1, 0x6, 0x3, 0x0, 0x6, 0xE};
const int s2[16] = {0xE, 0x5, 0xE, 0xA, 0x9, 0x5, 0xD, 0xA, 0x7, 0xE, 0x5, 0x1, 0xb, 0x4, 0xD, 0x0};
const int s3[16] = {0x1, 0x0, 0xB, 0x9, 0x2, 0x8, 0x4, 0x6, 0x3, 0x4, 0xc, 0x9, 0xd, 0x3, 0x5, 0x8};
const int s4[16] = {0xF, 0x4, 0xD, 0x0, 0x4, 0x2, 0xA, 0x7, 0xD, 0xb, 0x3, 0xb, 0x8, 0x9, 0x6, 0xB};
const int s5[16] = {0x6, 0xC, 0x7, 0xB, 0xF, 0x3, 0x1, 0x9, 0x0, 0x5, 0x6, 0x4, 0x2, 0xf, 0x0, 0xE};
const int s6[16] = {0x8, 0x5, 0x2, 0x2, 0xD, 0xE, 0x9, 0xB, 0x3, 0xf, 0x2, 0x7, 0xc, 0x6, 0xF, 0x1};
const int s7[16] = {0x6, 0xA, 0xC, 0xD, 0x5, 0x0, 0x6, 0x3, 0xA, 0xc, 0x7, 0x2, 0xf, 0xc, 0x5, 0x9};
const int s8[16] = {0x5, 0x8, 0x1, 0x1, 0xA, 0xB, 0xE, 0x4, 0x4, 0x8, 0x7, 0x0, 0x4, 0x9, 0xA, 0x5};
const int* s[8] = {s1,s2,s3,s4,s5,s6,s7,s8};

typedef struct passstruct blockinfo;
struct passstruct {
    char* key;
    char* block;
    bool encrypt;
};

int arrSize(char* arr) {
    int i =0;
    while(*arr != '\0') {
        arr  = arr +1;
        i++;
    }
    return i;
}
////just going to store each hex digit as a character, not storing as a nyble bc bitshifting is a pain
char* toBytes(char* start, int size) {
    char* byteArr = malloc((sizeof(char))*size);
    char* temp = byteArr;
    for(int i =0; i<size; i++) {
        if(*start >='0' && *start<='9') {
            *temp = *start -'0';
        } else {
            *temp = toupper(*start) - 'A' + 0xA;
        }
        temp++;
        start++;
    }
    return byteArr;
}

void roundFunc(char** RE,char ** LE, char* roundKey) {
    char* temp = malloc(8*(sizeof(char)));
    //XOR step
    for(int i =0; i<8; i++) {
        *(temp+i) = *((*RE)+i) ^ *(roundKey+i);
    }
    //sbox step
    for(int j=0; j<8; j++) {
        *(temp+j) = s[j][*(temp+j)];
    }
    //create another temp to enable shifting
    char* temp2 = malloc(8*(sizeof(char)));
    for(int k =0; k<8; k++) {
        *(temp2+k) = *(temp+k);
    }
    //circularly shift left by 3 bits
    for(int m=0; m<8; m++) {
        *(temp+m) = ((*(temp2+m))%2) * 0x8 + (*(temp2+(m+1 %8)))/2;
    }
    free(temp2);
    //f has been finished
    //XOR the output with LE
    for(int n=0; n<8; n++) {
        *((*LE)+n) = *((*LE)+n) ^ *(temp+n);
    }
    //Switch Left and right
    char* temp3 = *LE;
    *LE = *RE;
    *RE = temp3;
    //cleanup
    free(temp);
}

char* keyGen (char* fullKey, int roundNumber) {
    char* roundKey = malloc(8 *(sizeof(char)));
    switch(roundNumber) {
        case 1:
            for(int i =0; i<8; i++) {
                *(roundKey+i) = *(fullKey+i);
            }
            break;
        case 2:
            for(int i =0; i<8; i++) {
                *(roundKey+i) = *(fullKey+i+1);
            }
            break;
        case 3:
            for(int i =0; i<8; i++) {
                *(roundKey+i) = *(fullKey+i+2);
                if(i%2==1){
                    *(roundKey+i) = *(roundKey+i) ^ 0xF;
                }
            }
            break;
        case 4:
            for(int i =0; i<8; i++) {
                *(roundKey+i) = *(fullKey+i+3);
                if(i%2==1){
                    *(roundKey+i) = *(roundKey+i) ^ 0xE;
                }
                else {
                    *(roundKey+i) = *(roundKey+i) ^ 0x1;
                }
            }
            break;
        case 5:
            for(int i =0; i<8; i++) {
                *(roundKey+i) = *(fullKey+i+4);
                if(i%2==1){
                    *(roundKey+i) = *(roundKey+i) ^ 0xC;
                }
                else {
                    *(roundKey+i) = *(roundKey+i) ^ 0x3;
                }
            }
            break;
        case 6:
            for(int i =0; i<8; i++) {
                *(roundKey+i) = *(fullKey+i+5);
                if(i%2==1){
                    *(roundKey+i) = *(roundKey+i) ^ 0x8;
                }
                else {
                    *(roundKey+i) = *(roundKey+i) ^ 0x7;
                }
            }
            break;
        case 7:
            for(int i =0; i<8; i++) {
                *(roundKey+i) = *(fullKey+i+6);
                if(i%2==1){
                    *(roundKey+i) = *(roundKey+i) ^ 0x0;
                }
                else {
                    *(roundKey+i) = *(roundKey+i) ^ 0xF;
                }
            }
            break;
        case 8:
            for(int i =0; i<8; i++) {
                *(roundKey+i) = *(fullKey+i+7);
            }
            break;
        case 9:
            for(int i =0; i<8; i++) {
                *(roundKey+i) = *(fullKey+i+8);
            }
            break;
    }
    return roundKey;
}


void blockFunc(char* key, char* block, bool encrypt) {
    char* LE = malloc(8*sizeof(char));
    char* RE = malloc(8*sizeof(char));
    for(int j = 0; j<8; j++) {
        *(LE+j) = *(block+j);
        *(RE+j) = *(block+8+j);
    }
    for(int i=0; i<9; i++) {
        char* roundKey = keyGen(key, i+1);
        roundFunc(&RE, &LE, roundKey);
        free(roundKey);
    }
    for(int k =0; k<8; k++) {
        *(block+k) = encrypt? *(LE+k):*(RE+k);
        *(block+8+k) = encrypt? *(RE+k):*(LE+k);
    }
    free(LE);
    free(RE);

}
//
//void* asyncBlock(void* info) {
//    blockinfo* params = (blockinfo*) info;
//    blockFunc(params->key, params->block, params->encrypt);
//    return(NULL);
//}

int main(int argc, char** argv) {
    printf("Encrypting");
    if(argc != 4) {
        printf("Wrong number of arguments, expect 4, received %d", argc);
    } else if (arrSize(*(argv+1))!=2){
        printf("Please enter either -d or -e as the first argument, found %s", *(argv+1));
    } else if (arrSize(*(argv+2))!= 18){
        printf("Please enter a 64-bit key in hex as the second argument, found %s", *(argv+2));
    }
    int digits = arrSize(*(argv+3))-2;
    int blocks = digits /16 +(digits%16==0?0:1);
    blockinfo* calls = malloc(blocks* sizeof(blockinfo));
//    //pthread_t* myThreads = malloc(blocks * sizeof(pthread_t));

    for(int i=0; i<blocks; i++) {
        calls[i].key = *(argv+2);
        calls[i].encrypt = *((*(argv+1))+1)=='e';
        if(i!=blocks-1) {
            calls[i].block = toBytes((*(argv + 3)) + 16 * i+2, 16);
        } else {
            calls[i].block = malloc(16* sizeof(char));
            for(int j = 0; j<16; j++) {
                if(i*16+j<digits) {
                    if(*(*(argv+3)+16*i+2+j) >='0' && *(*(argv+3)+16*i+2+j)<='9') {
                        *(calls[i].block+j) = *(*(argv+3)+16*i+2+j) -'0';
                    } else {
                        *(calls[i].block+j) = toupper(*(*(argv+3)+16*i+2+j)) - 'A' + 0xA;
                    }
                } else {
                    *(calls[i].block+j) = 0x0;
                }
            }
        }
       // pthread_create(myThreads+i, NULL, asyncBlock,(void*) calls+i);
       blockFunc(calls[i].key,calls[i].block,calls[i].encrypt);
    }
    printf("0x");
    for(int j=0; j<blocks; j++) {
        //pthread_join(myThreads[j],NULL);
        for(int k =0; k<16; k++) {
            char temp;
            if(*(calls[j].block+k)<0xA) {
                printf("%c", (*(calls[j].block+k) +'0'));
            } else {
                printf("%c", (*(calls[j].block+k) +'A'));
            }
        }
        free(calls[j].block);
    }
    free(calls);
   // free(myThreads);
    printf("end");
    return 0;

}