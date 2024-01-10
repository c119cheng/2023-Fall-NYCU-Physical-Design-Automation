#ifndef ROUTER_H
#define ROUTER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <utility>
#include <climits>
#include <algorithm>
using namespace std;

struct boundary{
    map<int, vector<pair<int,int>>> track; // track name -> track constraint
    vector<int> left; // left most free space for track
    vector<int> right; // next end point, (left, right) is first free space for route
    int size; // number of track
    vector<int> next_idx; // next obstacle idx
};

struct Net{
    int name;
    int left=INT_MAX, right=0; // left most and right most index
    int in_degree;
    int out_degree;
    string track;
    vector<pair<string,pair<int,int>>> route; // router track name -> left index and right index (not use no dogleg)
};

class Router{
    private:
        char *input_file, *output_file;
        boundary top_boundary, bottom_boundary;
        int pin_size;
        int top_pin[100];
        int bottom_pin[100];
        int Nets_size;
        Net Nets[100];

        bool VCG[100][100] = {0};
        vector<pair<int, int>> route_order;
        int new_track;
    public:
        Router(char *, char *);
        ~Router();
        void InputParser();
        void route();
        bool route_bottom(); // route bottom boundary
        void output();

        void Build_boundary();
        void Build_VCG();

        // for debug
        void show_boundary();
        void show_nets();
        void show_route_order();
};


#endif