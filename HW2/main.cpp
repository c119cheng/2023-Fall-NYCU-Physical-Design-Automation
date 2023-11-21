#include "B_tree.h"
int main(int argc, char **argv){
    B_tree B(argv[1]);
    B.solve();
    B.output(argv[2]);
    return 0;
}