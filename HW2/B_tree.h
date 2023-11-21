#include "skyline.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <stack>
#include <cmath>
using namespace std;


template <typename T>
void swapElements(T& a, T& b) {
    T temp = a;
    a = b;
    b = temp;
}

struct Block{
    int w, h;
    int x, y;
    int best_x, best_y;
    bool rotated, best_rotated;
    string name;
    // info in tree
    bool is_left;
    Block *left, *right, *parent;
};

class B_tree{
    private:
        skyline sky;
        double R_low, R_upper; // aspect ratio for floorplan
        int width, height;
        int area, best_area, cur_area;
        int best_width, best_height;
        int cur_width, cur_height;
        Block *root;
        vector<Block *> block_list;
        int block_size; 
        unordered_map<string, int> block_lib; // block name and index library

        // for SA
        double temperature;
        bool accept_best;
        // for FastSA
        double initial_temp;
        const int c = 100;
        const int k = 7;
        bool force_legal;
        int n;
        double delta_cost;
        double delta_avg;
        double ave_area;
        // SA deletion
        Block *deleted_block;
    public:
        B_tree(char *);
        ~B_tree();
        void solve();
        void output(char *);
        void build_tree();
        void packing();
        void FastSA();
        void SA();
        void classicalSA();
        void getFastSA_parameter();
        void show_tree();
        void update_best_area();

        // operation 1 for SA : randomly rotate a block
        void random_rotate(bool);

        // operation 3 for SA : randomly swap 2 block
        void random_swap(bool);
        void swap_wh(Block*, Block*);
        void swap_all(Block*, Block*);

        // operation 2 for SA : randomly delete a block and insert
        void random_delete_insert(bool);
        void swap_deletion(Block*, Block*);
        Block* deletion(Block*, int&);
        void insertion(Block*, Block*, int&);
        void deletion_back(Block*, int&); // delete the node inserted
        void insertion_back(Block*, Block*, Block*, int); // insert the node to original position
};