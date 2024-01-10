#include "Router.h"

Router::Router(char *input, char *output){
    input_file = input;
    output_file = output;
    cout <<"parse input " << endl;
    InputParser();
}

Router::~Router(){

}

void Router::InputParser(){
    ifstream fin(input_file);
    string line;
    while(getline(fin, line)){
        stringstream ss(line);
        if(line[0] == 'T'){ // parser for top boundary
            int left, right;
            ss >> line >> left >> right;
            top_boundary.track[stoi(line.substr(1))].push_back(pair<int,int>(left, right));
        }
        else if(line[0] == 'B'){ // parser for bottom boundary
            int left, right;
            ss >> line >> left >> right;
            bottom_boundary.track[stoi(line.substr(1))].push_back(pair<int,int>(left, right));
        }
        else{ // parser for net
            int id;
            Nets_size = 0;
            int pin_idx = 0;
            while(ss >> id){ // top nets
                if(id != 0){
                    Nets[id].left = min(Nets[id].left, pin_idx);
                    Nets[id].right = max(Nets[id].right, pin_idx); 
                }
                top_pin[pin_idx] = id;
                pin_idx++;
                Nets_size = max(Nets_size, id);
            }
            pin_size = pin_idx;

            // bottom nets
            pin_idx = 0;
            getline(fin, line);
            ss.clear();
            ss.str(line);
            while(ss >> id){ // bottom nets
                if(id != 0){
                    Nets[id].left = min(Nets[id].left, pin_idx);
                    Nets[id].right = max(Nets[id].right, pin_idx); 
                }
                bottom_pin[pin_idx] = id;
                pin_idx++;
                Nets_size = max(Nets_size, id);
            }
        }
    }
}

bool comparePairs(const std::pair<int, int>& lhs, const std::pair<int, int>& rhs) {
    return lhs.first < rhs.first;
}


void Router::Build_boundary(){
    // top boundary
    vector<pair<int, int>> tmp;
    top_boundary.size = top_boundary.track.size();
    top_boundary.left = vector<int>(top_boundary.size);
    top_boundary.right = vector<int>(top_boundary.size);
    top_boundary.next_idx = vector<int>(top_boundary.size);
    for(auto& t : top_boundary.track){
        if(t.first == 0){
            tmp = t.second;
        }
        else{
            t.second.insert(t.second.end(), tmp.begin(), tmp.end());
            tmp = t.second;
            sort(t.second.begin(), t.second.end(), comparePairs);
        }

        // calculate start point;
        int cur_left = 0;
        int cur_right = 0;
        int i=0;
        for(auto& p : t.second){
            if(cur_left == p.first){
                cur_left = p.second;
            }
            else{
                cur_right = p.first;
                break;
            }
            i++;
        }
        top_boundary.next_idx[t.first] = i;
        top_boundary.left[t.first] = (cur_left == 0)?0:cur_left+1;
        top_boundary.right[t.first] = (cur_right == 0)?pin_size:cur_right;
    }

    // bottom boundary
    bottom_boundary.size = bottom_boundary.track.size();
    bottom_boundary.left = vector<int>(bottom_boundary.size);
    bottom_boundary.right = vector<int>(bottom_boundary.size);
    bottom_boundary.next_idx = vector<int>(bottom_boundary.size);
    for(auto& t : bottom_boundary.track){
        if(t.first == 0){
            tmp = t.second;
        }
        else{
            t.second.insert(t.second.end(), tmp.begin(), tmp.end());
            tmp = t.second;
            sort(t.second.begin(), t.second.end(), comparePairs);
        }
        
        // calculate start point;
        int cur_left = 0;
        int cur_right = 0;
        int i=0;
        for(auto& p : t.second){
            if(cur_left == p.first){
                cur_left = p.second;
            }
            else{
                cur_right = p.first;
                break;
            }
            i++;
        }
        bottom_boundary.next_idx[t.first] = i;
        bottom_boundary.left[t.first] = (cur_left == 0)?0:cur_left+1;
        bottom_boundary.right[t.first] = (cur_right == 0)?pin_size:cur_right;
    }
}

void Router::Build_VCG(){
    // VCG[i][j] = 1 if i->j exist an edge 
    for(int i=0;i<pin_size;i++){
        // cout << top_pin[i] << "->" << bottom_pin[i] << endl;
        VCG[top_pin[i]][bottom_pin[i]] = true;
    }
}

void Router::route(){
    Build_boundary();
    Build_VCG();

    // find out all net with in-degree=0 as route order
    route_order.reserve(Nets_size);
    for(int i=1;i<=Nets_size;i++){
        int in_degree = 0;
        int out_degree = 0;
        for(int j=1;j<=Nets_size;j++){
            if(VCG[j][i]){
                in_degree++;
            }
            if(VCG[i][j])
                out_degree++;
        }
        Nets[i].in_degree = in_degree;
        Nets[i].out_degree = out_degree;
        if(in_degree==0){
            route_order.push_back(pair<int,int>(Nets[i].left, i));
        }
    }
    sort(route_order.begin(), route_order.end(), comparePairs);
    cout << "Initial route order" << endl;
    show_route_order();

    // route top boundary
    for(int i=top_boundary.size-1;i>=0;i--){
        int route_size = route_order.size();
        for(int j=0;j<route_size;j++){
            int net_idx = route_order[j].second;
            if(Nets[net_idx].left >= top_boundary.left[i] && Nets[net_idx].right < top_boundary.right[i]){ // fit in space
                Nets[net_idx].track = "T" + to_string(i);
                cout << "route net" << net_idx << " on " << Nets[net_idx].track << endl;
                top_boundary.left[i] = Nets[net_idx].right + 1;
                route_order.erase(route_order.begin()+j);
                j--; route_size--;
                // update 0 indegree nets
                for(int k=1;k<=Nets_size;k++){
                    if(VCG[net_idx][k]){
                        Nets[k].in_degree--;
                        if(Nets[k].in_degree == 0)
                            route_order.push_back(pair<int,int>(Nets[k].left, k));
                    }
                }
                // sort(route_order.begin(), route_order.end(), comparePairs);
                // j=-1; // start with first net
                // show_route_order();
            }
            else if(Nets[net_idx].right >= top_boundary.right[i]){ // next free space
                int& next_idx = top_boundary.next_idx[i];
                int cur_left = top_boundary.right[i];
                int cur_right = 0;
                for(;;){
                    pair<int, int>& p = top_boundary.track[i][next_idx];
                    if(cur_left == p.first){
                        cur_left = p.second;
                    }
                    else{
                        cur_right = p.first;
                        break;
                    }
                    next_idx++;
                }
                top_boundary.left[i] = cur_left;
                top_boundary.right[i] = (cur_right == 0)?pin_size:cur_right;
                // cout << "next obstacle for track : " << i << endl;
            }
        }
        sort(route_order.begin(), route_order.end(), comparePairs);
        // cout << "next track" << endl;
    }


    // route on new track and bottom boundary
    new_track = 0;
    while(route_order.size() != 0){
        int size = route_order.size();
        bool success = route_bottom();
        if(success)
            continue;
        else{ // route on new track
            new_track++;
            int left = 0;
            for(int i=0;i<size;i++){
                int net_idx = route_order[i].second;
                if(Nets[net_idx].left >= left){ // fit
                    Nets[net_idx].track = "C" + to_string(new_track);
                    cout << "route net" << net_idx << " on " << Nets[net_idx].track << endl;
                    left = Nets[net_idx].right + 1;
                    route_order.erase(route_order.begin()+i);
                    i--; size--;
                    
                    // update 0 indegree nets
                    for(int k=1;k<=Nets_size;k++){
                        if(VCG[net_idx][k]){
                            Nets[k].in_degree--;
                            if(Nets[k].in_degree == 0)
                                route_order.push_back(pair<int,int>(Nets[k].left, k));
                        }
                    }
                }
            }

            sort(route_order.begin(), route_order.end(), comparePairs);
        }
    }

}

bool Router::route_bottom(){
    // route top boundary
    bool success = false;
    for(int i=0;i<bottom_boundary.size;i++){
        if(bottom_boundary.left[i] >= bottom_boundary.right[i])
            continue;
        int route_size = route_order.size();
        for(int j=0;j<route_size;j++){
            int net_idx = route_order[j].second;
            if(Nets[net_idx].out_degree > 0)
                continue;
            if(Nets[net_idx].left >= bottom_boundary.left[i] && Nets[net_idx].right < bottom_boundary.right[i]){ // fit in space
                Nets[net_idx].track = "B" + to_string(i);
                cout << "route net" << net_idx << " on " << Nets[net_idx].track << endl;
                bottom_boundary.left[i] = Nets[net_idx].right + 1;
                route_order.erase(route_order.begin()+j);
                j--; route_size--;
                
                // update 0 indegree nets
                for(int k=1;k<=Nets_size;k++){
                    if(VCG[net_idx][k]){
                        Nets[k].in_degree--;
                        if(Nets[k].in_degree == 0)
                            route_order.push_back(pair<int,int>(Nets[k].left, k));
                    }
                }
                success = true;
                // sort(route_order.begin(), route_order.end(), comparePairs);
                // j=-1; // start with first net
                // show_route_order();
            }
            else if(Nets[net_idx].right >= bottom_boundary.right[i]){ // next free space
                int& next_idx = bottom_boundary.next_idx[i];
                int cur_left = bottom_boundary.right[i];
                int cur_right = 0;
                for(;;){
                    pair<int, int>& p = bottom_boundary.track[i][next_idx];
                    if(cur_left == p.first){
                        cur_left = p.second;
                    }
                    else{
                        cur_right = p.first;
                        break;
                    }
                    next_idx++;
                }
                bottom_boundary.left[i] = cur_left;
                bottom_boundary.right[i] = (cur_right == 0)?pin_size:cur_right;
                // cout << "next obstacle for track : " << i << endl;
            }
        }
        sort(route_order.begin(), route_order.end(), comparePairs);
        // cout << "next track" << endl;
    } 
    return success;
}

void Router::output(){
    show_boundary();
    show_nets();

    // convert track name
    map<string, string> m;
    int tmp = new_track;
    for(int i=1;i<=new_track;i++){
        m["C" + to_string(i)] = "C" + to_string(tmp);
        tmp--;
    }

    ofstream fout(output_file);
    fout << "Channel density: " << new_track << endl;
    for(int i=1;i<=Nets_size;i++){
        fout << "Net " << i << endl;
        Nets[i].track = (Nets[i].track[0] == 'C')?m[Nets[i].track] : Nets[i].track;
        fout << Nets[i].track << " " << Nets[i].left << " " << Nets[i].right << endl;
    }
}

void Router::show_boundary(){
    cout << "Top boundary" << endl;
    for(auto t : top_boundary.track){
        cout << "\tT" << t.first << " : ";
        for(auto p : t.second){
            cout << p.first << "->" << p.second << " ";
        }
        cout << " ,start point : " << top_boundary.left[t.first]; 
        cout << " ,end point : " << top_boundary.right[t.first];
        cout << " ,next obstacle index=" << top_boundary.next_idx[t.first];
        cout << endl;
    }
    cout << "Bottom boundary" << endl;
    for(auto t : bottom_boundary.track){
        cout << "\tB" << t.first << " : ";
        for(auto p : t.second){
            cout << p.first << "->" << p.second << " ";
        }
        cout << " ,start point : " << bottom_boundary.left[t.first]; 
        cout << " ,end point : " << bottom_boundary.right[t.first];
        cout << " ,next obstacle index=" << bottom_boundary.next_idx[t.first];
        cout << endl;
    }
}

void Router::show_nets(){
    cout << "show nets : " << endl;
    for(int i=1;i<=Nets_size;i++){
        cout << "\tNet" << i << " : ";
        cout << "left=" << Nets[i].left << " , right=" << Nets[i].right << endl;
    }
}

void Router::show_route_order(){
    cout << "Show route order" << endl;
    for(auto n : route_order){
        cout << "\tNet" << n.second << " : left=" << n.first << endl;
    }
}