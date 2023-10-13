#include "FM.h"

Partitioner::Partitioner(){

}

Partitioner::~Partitioner(){

}

void Partitioner::load(char *file_name){
    ifstream fin(file_name);
    string line;

    // get balance factor
    getline(fin, line);
    stringstream ss(line);
    ss >> this->balance_factor;
    ss.clear();

    // Read Nets
    map<int, int> cell_dic;
    nets = 0;
    cells = 0;
    P_MAX = 0;
    vector<int> tmp_net_list;
    unordered_set<int> same_cell;
    while(getline(fin, line)){
        ss.str(line);

        int in;
        char _;

        while(ss >> line){
            if(line == "NET" || line[0] == 'n')
                continue;
            if(line[0] == ';'){
                net_list.push_back(tmp_net_list);
                nets++;
                tmp_net_list.clear();
                same_cell.clear();
                continue;
            }
            stringstream zz(line);
            zz >> _ >> in; // remove prefix c

            if(same_cell.count(in))
                continue;
            same_cell.insert(in);
            // get cell index
            int cell_idx;
            if(cell_dic.count(in)){ // cell already load and encode
                cell_idx = cell_dic[in];
            }
            else{ // encode cell name
                cell_idx = cells;
                cells++;
                cell_dic[in] = cell_idx;
                cell_name.push_back(in);
            }

            // make net list
            tmp_net_list.push_back(cell_idx);
        }
        ss.clear();
    }
    fin.close();

    // make cell list and cell neighbor
    int net_idx = 0;
    cell_list = vector<vector<int>>(cells);
    fin.open(file_name);
    getline(fin, line);
    same_cell.clear();
    while(getline(fin, line)){
        ss.str(line);
        int in;
        char _;

        while(ss >> line){
            if(line == "NET" || line[0] == 'n')
                continue;
            if(line[0] == ';'){
                same_cell.clear();
                net_idx++;
                continue;
            }
            stringstream zz(line);
            zz >> _ >> in; // remove prefix c
            if(same_cell.count(in))
                continue;
            same_cell.insert(in);
            int cell_idx = cell_dic[in];
            cell_list[cell_idx].push_back(net_idx);
            if(cell_list[cell_idx].size() > P_MAX)
                P_MAX = cell_list[cell_idx].size();
        }

        ss.clear();
    }

}

void Partitioner::solve(){
    cout<<"start solve"<<endl;
    // declare space

    set_A = vector<unordered_set<int>>(2*P_MAX+1);
    set_B = vector<unordered_set<int>>(2*P_MAX+1);

    step_gain = vector<int>(cells);
    step_move = vector<int>(cells);
    min = ceil((1-balance_factor)/2.0 * cells);
    max = floor((1+balance_factor)/2.0 * cells);
    cout<<"min = "<<min<<endl;
    cout<<"max = "<<max<<endl;
    net_A_size = vector<int>(nets);
    net_B_size = vector<int>(nets);

    locked = vector<bool>(cells);
    cell_gain = vector<int>(cells);
    cell_side = vector<bool>(cells);
    cell_old_side = vector<bool>(cells);

    // start solve
    int best_sol = nets;
    int prev = nets;
    int count = 0;
    vector<int> best_side(cells);

    while(1){
        size_A = 0;
        size_B = 0;
        int initial_net = random()%(nets);
        initial_partition(initial_net);
        for(int i=0;i<cells;i++){
            cell_old_side[i] = cell_side[i];
        }
        cout<<"min = "<<min<<endl;
        cout<<"max = "<<max<<endl;
        cout<<"size_A = "<<size_A<<endl;
        cout<<"size_B = "<<size_B<<endl;

        // start FM algorithm
        stop_FM = false;
        while(!stop_FM){

            for(int i=0;i<nets;i++){
                net_A_size[i] = 0;
                net_B_size[i] = 0;
                for(const auto& cell:net_list[i]){
                    if(cell_side[cell])
                        net_B_size[i]++;
                    else   
                        net_A_size[i]++;
                }
            }
            cout<<"get gain"<<endl;
            get_all_gain();
            cout<<"make bucket"<<endl;
            make_bucket();
            cout<<"FM"<<endl;
            FM();
        }

        // compare to best solution
        getCuts();
        cout<<"Cuts = "<<cut<<endl;
        bool balance = (size_A >= min && size_A <= max);
        if(cut < best_sol && balance){
            best_sol = cut;
            for(int i=0;i<cells;i++){
                best_side[i] = cell_side[i];
            }
        }

        if(prev == best_sol)
            count++;
        if(count > 10)
            break;
        prev = best_sol;
    }

    size_A = 0;
    for(int i=0;i<cells;i++){
        if(!best_side[i])
            size_A++;
        cell_side[i] = best_side[i];
    }
}

void Partitioner::initial_partition(int in=0){
    cout<<"start initial partition"<<endl;
    vector<bool> tmp(cells, false); // cell already pattitioned
    for(int i=in;i<nets;i++){
        bool side_A = false;
        bool side_B = false;
        for(const auto& cell : net_list[i])
            if(tmp[cell]){
                if(cell_side[cell])
                    side_B = true;
                else
                    side_A = true;
            }

        bool side;
        if(side_A && side_B){
            continue;
        }
        else if(side_A){
            side = 0;
        }
        else if(side_B){
            side = 1;
        }
        else{
            if(size_A > size_B)
                side = 1;
            else
                side = 0;
        }

        for(const auto& cell : net_list[i]){
            if(!tmp[cell]){
                if(side && size_B <= max || size_A >= max){
                    cell_side[cell] = 1;
                    size_B++;
                }
                else{
                    cell_side[cell] = 0;
                    size_A++;
                }
                tmp[cell] = 1;
            }
        }
    }

    for(int i=0;i<in;i++){
        bool side_A = false;
        bool side_B = false;
        for(const auto& cell : net_list[i])
            if(tmp[cell]){
                if(cell_side[cell])
                    side_B = true;
                else
                    side_A = true;
            }

        bool side;
        if(side_A && side_B){
            continue;
        }
        else if(side_A){
            side = 0;
        }
        else if(side_B){
            side = 1;
        }
        else{
            if(size_A > size_B)
                side = 1;
            else
                side = 0;
        }

        for(const auto& cell : net_list[i]){
            if(!tmp[cell]){
                if(side && size_B <= max || size_A >= max){
                    cell_side[cell] = 1;
                    size_B++; 
                }
                else{
                    cell_side[cell] = 0;
                    size_A++;
                }
                tmp[cell] = 1;
            }
        }
    }

    // partition for those not partitioned
    for(int i=0;i<cells;i++){
        if(!tmp[i]){
            int side_t = random()%2;
            if(side_t){
                cell_side[i] = 0;
                size_A++;
            }
            else{
                cell_side[i] = 1;
                size_B++;
            }
        }
    }

}

void Partitioner::get_all_gain(){
    for(int i=0;i<cells;i++)
        cell_gain[i] = 0;

    for(int i=0;i<nets;i++){
        if(net_A_size[i] > 1 && net_B_size[i] > 1)
            continue;
        if(net_A_size[i] == 1){
            for(const auto&cell : net_list[i]){
                if(cell_side[cell] == 0){
                    cell_gain[cell]++;
                    break;
                }
            }
        }
        else if(net_A_size[i] == 0){
            for(auto const& cell : net_list[i])
                cell_gain[cell]--;
        }
        if(net_B_size[i] == 1){
            for(const auto& cell : net_list[i]){
                if(cell_side[cell] == 1){
                    cell_gain[cell]++;
                    break;
                }
            }
        }
        else if(net_B_size[i] == 0){
            for(auto const& cell : net_list[i])
                cell_gain[cell]--;
        }
    }
}

void Partitioner::make_bucket(){
    for(int i=0;i<=2*P_MAX;i++){
        set_A[i].clear();
        set_B[i].clear();
    }

    set_B_ptr = 0;
    set_A_ptr = 0;
    for(int i=0;i<cells;i++){
        if(cell_side[i]){
            int idx = cell_gain[i] + P_MAX;
            set_B[idx].insert(i);
            if(set_B_ptr < idx)
                set_B_ptr = idx;
        }
        else{
            int idx = cell_gain[i] + P_MAX;
            set_A[idx].insert(i);
            if(set_A_ptr < idx)
                set_A_ptr = idx;
        }
        locked[i] = 0;
    }
}

void Partitioner::FM(){
    int iter_time = cells;
    int total_gain = 0;
    int max_gain = 0;
    int max_idx = 0;
    for(int lock_num=0;lock_num<iter_time;lock_num++){
        while(set_A_ptr >= 0 &&set_A[set_A_ptr].empty()){
            set_A_ptr--;
        }
        while(set_B_ptr >= 0 &&set_B[set_B_ptr].empty()){
            set_B_ptr--;
        }
        int cell;

        if(set_A_ptr < 0 && set_B_ptr < 0){
            break;
        }
        else if(set_A_ptr < 0){
            // move from B to A
            cell = *set_B[set_B_ptr].begin();
            set_B[set_B_ptr].erase(cell);
            size_A++;
        }
        else if(set_B_ptr < 0){
            // move from A to B
            cell = *set_A[set_A_ptr].begin();
            set_A[set_A_ptr].erase(cell); 
            size_A--;     
        }
        else if((set_A_ptr > set_B_ptr && size_A > min) || size_A >= max){
            // move from A to B
            cell = *set_A[set_A_ptr].begin();
            set_A[set_A_ptr].erase(cell); 
            size_A--;
        }
        else{
            // move from B to A
            cell = *set_B[set_B_ptr].begin();
            set_B[set_B_ptr].erase(cell);
            size_A++;
        }

        locked[cell] = 1;
        cell_side[cell] = !cell_side[cell];
        step_move[lock_num] = cell;
        // update_neighbor(cell);
        update_gain(cell);

        total_gain += cell_gain[cell];
        if(total_gain >= max_gain){
            max_gain = total_gain;
            max_idx = lock_num;
        }
    }
    // cout<<"balance  "<<balance<<endl;
    if(max_gain <= 0){
        stop_FM = true;
    }
    
    for(int i=0;i<max_idx;i++)
        cell_old_side[step_move[i]] = !cell_old_side[step_move[i]];
    size_A = 0;
    for(int i=0;i<cells;i++){
        cell_side[i] = cell_old_side[i];
        if(!cell_side[i])
            size_A++;
    }
    size_B = cells - size_A;
}

void Partitioner::update_gain(const int& in){
    // reserve cell gain
    unordered_set<int> visited;
    vector< std::pair<int, int> > pre_gain;

    for(const auto& i : cell_list[in]){
        if(cell_side[in]){
            // From A to B
            if(net_B_size[i] == 0){
                for(const auto& cell : net_list[i])
                    if(!locked[cell]){
                        if(!visited.count(cell)){
                            pre_gain.push_back({cell, cell_gain[cell]});
                            visited.insert(cell);
                        }
                        cell_gain[cell]++;
                    }
            }
            else if(net_B_size[i] == 1){
                for(const auto& cell : net_list[i])
                    if(!locked[cell] && cell_side[cell] == 1){
                        if(!visited.count(cell)){
                            pre_gain.push_back({cell, cell_gain[cell]});
                            visited.insert(cell);
                        }
                        cell_gain[cell]--;
                        break;
                    }
            }
            net_B_size[i]++;
            net_A_size[i]--;

            if(net_A_size[i] == 0){
                for(const auto& cell : net_list[i])
                    if(!locked[cell]){
                        if(!visited.count(cell)){
                            pre_gain.push_back({cell, cell_gain[cell]});
                            visited.insert(cell);
                        }
                        cell_gain[cell]--;
                    }
            }
            else if(net_A_size[i] == 1){
                for(const auto& cell : net_list[i])
                    if(!locked[cell] && cell_side[cell] == 0){
                        if(!visited.count(cell)){
                            pre_gain.push_back({cell, cell_gain[cell]});
                            visited.insert(cell);
                        }
                        cell_gain[cell]++;
                        break;
                    }
            }
        }       
        else{
            // From B to A
            if(net_A_size[i] == 0){
                for(const auto& cell : net_list[i])
                    if(!locked[cell]){
                        if(!visited.count(cell)){
                            pre_gain.push_back({cell, cell_gain[cell]});
                            visited.insert(cell);
                        }
                        cell_gain[cell]++;
                    }
            }
            else if(net_A_size[i] == 1){
                for(const auto& cell : net_list[i])
                    if(!locked[cell] && cell_side[cell] == 0){
                        if(!visited.count(cell)){
                            pre_gain.push_back({cell, cell_gain[cell]});
                            visited.insert(cell);
                        }
                        cell_gain[cell]--;
                        break;
                    }
            }
            net_A_size[i]++;
            net_B_size[i]--;

            if(net_B_size[i] == 0){
                for(const auto& cell : net_list[i])
                    if(!locked[cell]){
                        if(!visited.count(cell)){
                            pre_gain.push_back({cell, cell_gain[cell]});
                            visited.insert(cell);
                        }
                        cell_gain[cell]--;
                    }
            }
            else if(net_B_size[i] == 1){
                for(const auto& cell : net_list[i])
                    if(!locked[cell] && cell_side[cell] == 1){
                        if(!visited.count(cell)){
                            pre_gain.push_back({cell, cell_gain[cell]});
                            visited.insert(cell);
                        }
                        cell_gain[cell]++;
                        break;
                    }
            }
        }
    }

    // update bucket list
    for(const auto p : pre_gain){
        if(cell_gain[p.first] != p.second){
            if(cell_side[p.first]){
                set_B[p.second + P_MAX].erase(p.first);
                set_B[cell_gain[p.first] + P_MAX].insert(p.first);
                if(set_B_ptr < cell_gain[p.first] + P_MAX)
                    set_B_ptr = cell_gain[p.first] + P_MAX;
            }
            else{
                set_A[p.second + P_MAX].erase(p.first);
                set_A[cell_gain[p.first] + P_MAX].insert(p.first);
                if(set_A_ptr < cell_gain[p.first] + P_MAX)
                    set_A_ptr = cell_gain[p.first] + P_MAX;
            }
        }
    }
}

void Partitioner::output(char *file_name){
    ofstream fout(file_name);
    getCuts();
    fout << "Cutsize = " << cut << endl;
    cout << "Cutsize = " << cut << endl;
    fout << "G1 " << size_A << endl;
    for(int i=0;i<cells;i++)
        if(!cell_side[i])
            fout << "c" << cell_name[i] << " ";
    fout << ";" << endl;
    fout << "G2 " << cells - size_A << endl;
    for(int i=0;i<cells;i++)
        if(cell_side[i])
            fout << "c" << cell_name[i] << " ";
    fout << ";" << endl;

}

void Partitioner::getCuts(){
    cut = 0;
    for(int i=0;i<nets;i++){
        for(int j=1;j<net_list[i].size();j++){
            if(cell_side[net_list[i][j-1]] != cell_side[net_list[i][j]]){
                cut++;
                break;
            }
        }
    }
}