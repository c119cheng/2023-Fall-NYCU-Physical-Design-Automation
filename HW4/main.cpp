#include "Router.h"
int main(int argc, char **argv){
    Router R(argv[1], argv[2]);
    R.route();
    R.output();
    return 0;
}