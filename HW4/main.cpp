#include "Placer.h"
int main(int argc, char **argv){
    Placer P(argv[1], argv[2]);
    P.place();
    P.output();
    return 0;
}