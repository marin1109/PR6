#include "stdlib.h"
#include "stdio.h"
#include "arpa/inet.h"

void test_endianess1(){
    uint32_t a = 1234;
    uint32_t b = htonl(a);
    if(a == b){
        printf("Big Endian\n");
    }else{
        printf("Little Endian\n");
    }
}

void test_endianess2(){
    uint32_t witness = 0x01020304;
    char *p = (char *) &witness;

    printf("Représentation en mémoire : %02x %02x %02x %02x\n", p[0], p[1], p[2], p[3]);

    if(p[0] == 0x01){
        printf("Big Endian\n");
    }else{
        printf("Little Endian\n");
    }
}

int main(){
    printf("Test1 :\n");
    test_endianess1();

    printf("Test2 :\n");
    test_endianess2();

    return EXIT_SUCCESS;
}