#include <queue>
#include <climits>
#include <iostream>
using namespace std;

struct node{
    int x1, x2; // start x-coordinate and end x-coordinate
    int y; // height
    node *next, *prev;
};


class skyline{
    private:
        node *root;
        node *tail;
        queue<node*> recycle; // recycle bin for not use node;
    public:
        skyline();
        ~skyline();
        void initial();
        int insert(const int&, const int&, const int&); // start x-coordinate, end x-coordinate, height of input block
        node* getNode();
        void show();
};