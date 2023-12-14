#ifndef PLACER_H
#define PLACER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <queue>
#include <climits>
#include <cmath>
#include <algorithm>
#include <cctype>
using namespace std;


struct MOS
{
    string name;
    string drain, gate, source; // drain == left, source == right
    bool nMos;
    bool is_dummy;
    double w, l;
    int nfin;

    MOS *next, *prev, *cpair; // The mos next, prev, complementary pair (nMOS and pMOS) in floorplan
};

struct Net
{
    string name;
    double min_x, max_x;
    bool prev_nmos;
    bool cross; // Net cross nmos and pmos
};

class Placer{
    private:
        double cross_cost;
        double best_cost;
        char *input_file, *output_file;
        MOS* mos_head; // the first nmos in the floorplan (left down) 
        vector<MOS*> mos_list;
        vector<MOS*> nmos_list;
        vector<MOS*> pmos_list;
        queue<MOS*> dummy_recycle;
        unordered_map<string, vector<MOS*>> gate_lib; // map for MOS with same gate
        unordered_map<string, Net*> net_lib;

        // SA
        double temperature;
    public:
        Placer(char *, char *);
        ~Placer();
        void place();
        void output();
        void read_spice();
        void show_mos();
        void generate_legal(); // generate a legal solution


        // SA
        void SA();
        void random_insert();
        void random_swap_nmos();
        void random_swap_cmos();
        void random_rotate_nmos();
        void random_rotate_pmos();
        void remove_prev_dummy();
        void check_prev_dummy(MOS*);
        void check_next_dummy(MOS*);
        double getCost();
        // for dummy mos
        MOS* getDummy();
};


#endif