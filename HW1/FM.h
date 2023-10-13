#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <unordered_set>
#include <cmath>
using namespace std;

class Partitioner{
    private:
        double balance_factor;
        int nets; // net size
        int cells; // cell size
        int P_MAX;

        vector<int> cell_name;
        vector<vector<int>> net_list; // net[i] connect cells
        vector<vector<int>> cell_list; // cell[i] connect nets

        // partition state
        bool stop_FM;
        int min, max;
        int size_A, size_B;
        int set_A_ptr, set_B_ptr;
        int cut;
        vector<unordered_set<int>> set_A, set_B;
        vector<int> step_gain, step_move;
        vector<int> net_A_size, net_B_size;

        // cell state in FM
        vector<bool> locked;
        vector<int> cell_gain;
        vector<bool> cell_side;
        vector<bool> cell_old_side;


    public:
        Partitioner();
        ~Partitioner();

        void load(char *);
        void solve();
        void output(char *);

        void initial_partition(int);
        void get_all_gain();
        void make_bucket();
        void FM();
        void getCuts();
        void update_gain(const int&);
};