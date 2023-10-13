#include "FM.h"
int main(int argc, char **argv){
    Partitioner tmp;
    tmp.load(argv[1]);
    tmp.solve();
    tmp.output(argv[2]);
    return 0;
}