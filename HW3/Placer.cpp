#include "Placer.h"

template <typename T>
void swapElements(T& a, T& b) {
    T temp = a;
    a = b;
    b = temp;
}

string to_lower(string& a){
    string tmp = a;
    for (char& c : tmp) {
        if(isalpha(c))
            c = std::tolower(c);
    }
    return tmp;
}

Placer::Placer(char *inputfile, char *outputfile){
    this->input_file = inputfile;
    this->output_file = outputfile;
    read_spice();
}

Placer::~Placer(){

}

void Placer::read_spice(){
    ifstream fin(input_file);
    string line;
    getline(fin, line); // circuit name and input/output
    double nmos_w, pmos_w;
    while(getline(fin, line)){
        stringstream ss(line);
        string name, drain, gate, source, body, type, width, length, nfin;
        ss >> name;
        string name_lower = to_lower(name);
        if(name_lower == ".ends" || name_lower == ".subckt")
            break;
        ss >> drain >> gate >> source >> body >> type >> width >> length >> nfin;

        MOS *cur_mos = new MOS;
        cur_mos->name = name.substr(1); // remove M begin
        cur_mos->drain = drain;
        cur_mos->gate = gate;
        cur_mos->source = source;
        cur_mos->nMos = (type == "nmos_rvt");
        
        //parser width and length
        ss.clear();
        ss.str(width.substr(2));
        ss >> cur_mos->w;
        if(cur_mos->nMos)
            nmos_w = cur_mos->w;
        else
            pmos_w = cur_mos->w;

        ss.clear();
        ss.str(length.substr(2));
        ss >> cur_mos->l;

        cur_mos->nfin = stoi(nfin.substr(5));

        mos_list.push_back(cur_mos);
        if(cur_mos->nMos)   
            nmos_list.push_back(cur_mos);
        else                
            pmos_list.push_back(cur_mos);
    }
    // calculate cross cost
    cross_cost = 27.0 + nmos_w/2.0 + pmos_w/2.0;
}

void Placer::generate_legal(){
    // build gate lib
    for(const auto& mos_ptr : mos_list){
        gate_lib[mos_ptr->gate].push_back(mos_ptr);
        if(!net_lib.count(mos_ptr->drain)){ // create drain net
            Net *new_net = new Net;
            new_net->min_x = INT_MAX;
            new_net->max_x = INT_MIN;
            new_net->name = mos_ptr->drain;
            new_net->prev_nmos = mos_ptr->nMos;
            new_net->cross = false;
            net_lib[mos_ptr->drain] = new_net;
        }
        else{
            net_lib[mos_ptr->drain]->cross |= (net_lib[mos_ptr->drain]->prev_nmos != mos_ptr->nMos);
        }
        if(!net_lib.count(mos_ptr->source)){ // create source net
            Net *new_net = new Net;
            new_net->min_x = INT_MAX;
            new_net->max_x = INT_MIN;
            new_net->name = mos_ptr->source;
            new_net->prev_nmos = mos_ptr->nMos;
            new_net->cross = false;
            net_lib[mos_ptr->source] = new_net;
        }
        else{
            net_lib[mos_ptr->source]->cross |= (net_lib[mos_ptr->source]->prev_nmos != mos_ptr->nMos);
        }
        
    }

    // make cmos pair
    // cout << "making cmos pair" << endl;
    for(const auto& g : gate_lib){
        int size = g.second.size();
        const vector<MOS*>& mos = g.second;
        // cout << "for gate : " << g.first << endl;
        for(int i=0;i<size;i++){
            // cout << "pair : " << mos[i]->name << endl;
            if(mos[i]->cpair){ // already found its girlfriend 
                continue;
            }
            for(int j=i+1;j<size;j++){ // loser we need to help to find one girl friend
                if(mos[j]->cpair){ // is sin, she has boy fried
                    continue;
                }
                else if(mos[i]->nMos != mos[j]->nMos){ // another loser
                    mos[i]->cpair = mos[j];
                    mos[j]->cpair = mos[i];
                    break;
                }
            }
        }
    }
    // make cmos sequence;
    // cout << "making cmos sequence" << endl;
    mos_head = nmos_list[0];
    MOS *cur_nmos = mos_head;
    for(int i=1;i<nmos_list.size();i++){
        MOS *next_nmos = nmos_list[i];
        MOS *next_pmos = next_nmos->cpair;
        if(cur_nmos->is_dummy){
            cur_nmos->next = next_nmos;
            next_nmos->prev = cur_nmos;
            next_pmos->prev = cur_nmos;
            cur_nmos = next_nmos;
        }
        else{
            MOS *cur_pmos = cur_nmos->cpair;
            if(cur_nmos->source == next_nmos->drain && cur_pmos->source == next_pmos->drain){ // match diffusion
                cur_nmos->next = next_nmos;
                next_nmos->prev = cur_nmos;
                cur_pmos->next = next_pmos;
                next_pmos->prev = cur_pmos;
                cur_nmos = next_nmos;
            }
            else{ // insert dummpy
                MOS *dummy = getDummy();
                cur_nmos->next = dummy;
                cur_pmos->next = dummy;
                dummy->prev = cur_nmos;
                cur_nmos = dummy;
                i--;
            }
        }
    }
}

MOS* Placer::getDummy(){
    MOS *out;
    if(!dummy_recycle.empty()){
        out = dummy_recycle.front();
        dummy_recycle.pop();
    }
    else{
        out = new MOS;
        out->is_dummy = true;
        out->name = "Dummy";
    }
    return out;
}

void Placer::output(){
    ofstream fout(output_file);

    // HPWL
    fout << best_cost << endl;

    MOS *cur_nmos = mos_head;
    MOS *cur_pmos = cur_nmos->cpair;

    // out pmos
    while(cur_pmos){
        fout << cur_pmos->name << " ";
        if(cur_pmos->is_dummy)
            cur_pmos = cur_pmos->next->cpair;  // cpair because dummy is point to next nmos
        else
            cur_pmos = cur_pmos->next;
        
    }
    fout << endl;

    // out pmos pins
    bool prev_is_dummy = true;
    cur_pmos = cur_nmos->cpair;
    while(cur_pmos){
        // out for dummy
        if(cur_pmos->is_dummy){
            fout << "Dummy ";
            prev_is_dummy = true;
            cur_pmos = cur_pmos->next->cpair;  // cpair because dummy is point to next nmos
            continue;    
        }
        // out for pmos
        if(prev_is_dummy){
            fout << cur_pmos->drain << " ";
        }
        fout << cur_pmos->gate << " " << cur_pmos->source << " ";
        cur_pmos = cur_pmos->next;
        prev_is_dummy = false;
    }
    fout << endl;

    // out nmos 
    while(cur_nmos){
        fout << cur_nmos->name << " ";
        cur_nmos = cur_nmos->next;
    }
    fout << endl;
    // out pmos pins
    prev_is_dummy = true;
    cur_nmos = mos_head;
    while(cur_nmos){
        // out for dummy
        if(cur_nmos->is_dummy){
            fout << "Dummy ";
            prev_is_dummy = true;
            cur_nmos = cur_nmos->next;
            continue;    
        }
        // out for pmos
        if(prev_is_dummy){
            fout << cur_nmos->drain << " ";
        }
        fout << cur_nmos->gate << " " << cur_nmos->source << " ";
        cur_nmos = cur_nmos->next;
        prev_is_dummy = false;
    }
    fout << endl;
}

void Placer::show_mos(){
    for(auto& mos_ptr : mos_list){
        cout << "MOS name : " << mos_ptr->name << endl;
        cout << "\tDrain  : " << mos_ptr->drain << endl;
        cout << "\tGate   : " << mos_ptr->gate << endl;
        cout << "\tSource : " << mos_ptr->source << endl;
        cout << "\tType   : " << ((mos_ptr->nMos)?"NMOS":"PMOS") << endl;
        cout << "\tWidth  : " << mos_ptr->w << endl;
        cout << "\tLength : " << mos_ptr->l << endl;
        cout << "\tnfin   : " << mos_ptr->nfin << endl;
        cout << "\tCMOS   : " << mos_ptr->cpair->name << endl;
    }
}

void Placer::place(){
    // cout <<"generate legal"<<endl;
    generate_legal();
    // cout << "show mos " << endl;
    // show_mos();
    best_cost = getCost();
    SA();
}

void Placer::SA(){
    temperature = 10;
    double prev_cost = best_cost;
    int count = 0;
    double r = 0.85;
    double stop_temperature = 10e-9;
    int op_time = max(int(mos_list.size() * 100), 1000);
    while(temperature > stop_temperature){
        for(int i=0;i<op_time;i++){
            int op = random()/double(RAND_MAX) * 100;
            if(op < 25 && 1)
                random_swap_nmos();
            else if(op < 50 && 1)
                random_swap_cmos();
            else if(op < 75 && 1)
                random_rotate_nmos();
            else if(op < 100 && 1)
                random_rotate_pmos();
        }
        // cout << "current temperature : " << temperature << endl;
        // cout << "best cost : " << best_cost << endl;
        temperature *= r;

        if(prev_cost == best_cost){
            count++;
            if(count == 10)
                break;
        }
        else{
            count = 0;
            prev_cost = best_cost;
        }
    }
}

void Placer::random_swap_nmos(){
    int nmos_idx1 = random() % nmos_list.size();
    int nmos_idx2 = random() % nmos_list.size();
    MOS *nmos1 = nmos_list[nmos_idx1];
    MOS *nmos2 = nmos_list[nmos_idx2];
    if(nmos1->gate != nmos2->gate || nmos1 == nmos2)
        return ;

    // swap nmos
    swapElements(nmos1->name, nmos2->name);
    swapElements(nmos1->drain, nmos2->drain);
    swapElements(nmos1->source, nmos2->source);

    check_prev_dummy(nmos1);
    check_next_dummy(nmos1);
    check_prev_dummy(nmos2);
    check_next_dummy(nmos2);

    double new_cost = getCost();

    if(new_cost <= best_cost){
        best_cost = new_cost;
    }
    else{
        // swap back
        swapElements(nmos1->name, nmos2->name);
        swapElements(nmos1->drain, nmos2->drain);
        swapElements(nmos1->source, nmos2->source);

        check_prev_dummy(nmos1);
        check_next_dummy(nmos1);
        check_prev_dummy(nmos2);
        check_next_dummy(nmos2);
    }
}

void Placer::random_swap_cmos(){
    int nmos_idx1 = random() % nmos_list.size();
    int nmos_idx2 = random() % nmos_list.size();
    MOS *nmos1 = nmos_list[nmos_idx1];
    MOS *nmos2 = nmos_list[nmos_idx2];
    MOS *pmos1 = nmos1->cpair;
    MOS *pmos2 = nmos2->cpair;
    if(nmos1 == nmos2)
        return ;

    // swap nmos
    swapElements(nmos1->name, nmos2->name);
    swapElements(nmos1->gate, nmos2->gate);
    swapElements(nmos1->drain, nmos2->drain);
    swapElements(nmos1->source, nmos2->source);

    swapElements(pmos1->name, pmos2->name);
    swapElements(pmos1->gate, pmos2->gate);
    swapElements(pmos1->drain, pmos2->drain);
    swapElements(pmos1->source, pmos2->source);

    // swap pmos

    check_prev_dummy(nmos1);
    check_next_dummy(nmos1);
    check_prev_dummy(nmos2);
    check_next_dummy(nmos2);

    double new_cost = getCost();

    if(new_cost <= best_cost){
        best_cost = new_cost;
    }
    else{
        // swap back
        swapElements(nmos1->name, nmos2->name);
        swapElements(nmos1->gate, nmos2->gate);
        swapElements(nmos1->drain, nmos2->drain);
        swapElements(nmos1->source, nmos2->source);

        swapElements(pmos1->name, pmos2->name);
        swapElements(pmos1->gate, pmos2->gate);
        swapElements(pmos1->drain, pmos2->drain);
        swapElements(pmos1->source, pmos2->source);

        check_prev_dummy(nmos1);
        check_next_dummy(nmos1);
        check_prev_dummy(nmos2);
        check_next_dummy(nmos2);
    }
}

void Placer::random_insert(){

}

void Placer::random_rotate_nmos(){
    int nmos_idx1 = random() % nmos_list.size();
    MOS *nmos = nmos_list[nmos_idx1];

    swapElements(nmos->source, nmos->drain);
    check_prev_dummy(nmos);
    check_next_dummy(nmos);

    double new_cost = getCost();

    bool accept = false;
    double delta_c = new_cost - best_cost;
    if(delta_c <= 0)
        accept = true;
    else{
        double p = rand() % 100 / 100.0;
        if(p < exp(-delta_c / temperature))
            accept = true;
    }

    if(accept){
        best_cost = new_cost;
    }
    else{
        // swap back
        swapElements(nmos->source, nmos->drain);
        check_prev_dummy(nmos);
        check_next_dummy(nmos);
    }
}

void Placer::random_rotate_pmos(){
    int pmos_idx1 = random() % pmos_list.size();
    MOS *pmos = pmos_list[pmos_idx1];
    MOS *nmos = pmos->cpair;

    swapElements(pmos->source, pmos->drain);
    check_prev_dummy(nmos);
    check_next_dummy(nmos);

    double new_cost = getCost();
    bool accept = false;
    double delta_c = new_cost - best_cost;
    if(delta_c <= 0)
        accept = true;
    else{
        double p = rand() % 100 / 100.0;
        if(p < exp(-delta_c / temperature))
            accept = true;
    }
    if(accept){
        best_cost = new_cost;
    }
    else{
        // swap back
        swapElements(pmos->source, pmos->drain);
        check_prev_dummy(nmos);
        check_next_dummy(nmos);
    }
}

void Placer::check_prev_dummy(MOS* nmos){
    MOS* pmos = nmos->cpair;
    if(nmos->prev == nullptr){ // is head
        return;
    }
    else if(nmos->prev->is_dummy){ // check whether remove dummy by share diffusion
        MOS *prev_nmos = nmos->prev->prev;
        MOS *prev_pmos = prev_nmos->cpair;

        if(prev_nmos->source == nmos->drain && prev_pmos->source == pmos->drain){
            // remove dummy
            dummy_recycle.push(prev_nmos->next);
            prev_nmos->next = nmos;
            nmos->prev = prev_nmos;
            prev_pmos->next = pmos;
            pmos->prev = prev_pmos;
        }
    }
    else{ // check wether insert dummy
        MOS *prev_nmos = nmos->prev;
        MOS *prev_pmos = pmos->prev;

        if(prev_nmos->source != nmos->drain || prev_pmos->source != pmos->drain){
            // insert dummy
            MOS *dummy = getDummy();
            // nmos and dummy
            prev_nmos->next = dummy;
            dummy->next = nmos;
            nmos->prev = dummy;
            dummy->prev = prev_nmos;

            // pmos
            prev_pmos->next = dummy;
            pmos->prev = dummy;
        }
    }
}

void Placer::check_next_dummy(MOS* nmos){
    MOS* pmos = nmos->cpair;
    if(nmos->next == nullptr){ // is tail
        return;
    }
    else if(nmos->next->is_dummy){ // check whether remove dummy by share diffusion
        MOS *next_nmos = nmos->next->next;
        MOS *next_pmos = next_nmos->cpair;
        if(next_nmos->drain == nmos->source && next_pmos->drain == pmos->source){
            // remove dummy
            dummy_recycle.push(nmos->next);
            nmos->next = next_nmos;
            next_nmos->prev = nmos;
            pmos->next = next_pmos;
            next_pmos->prev = pmos;
        }
    }
    else{ // check wether insert dummy
        MOS *next_nmos = nmos->next;
        MOS *next_pmos = next_nmos->cpair;
        if(next_nmos->drain != nmos->source || next_pmos->drain != pmos->source){
            // insert dummy
            MOS *dummy = getDummy();
            // nmos and dummy
            nmos->next = dummy;
            dummy->next = next_nmos;
            next_nmos->prev = dummy;
            dummy->prev = nmos;

            // pmos
            pmos->next = dummy;
            next_pmos->prev = dummy;
        }
    }
}

double Placer::getCost(){
    double cur_x = 12.5;
    MOS *cur_mos = mos_head;
    bool prev_dummy = true;
    while(cur_mos){
        if(cur_mos->is_dummy){
            cur_x += 108; // by spec
            prev_dummy = true;
        }
        else{
            if(prev_dummy){
                // update x for drain
                net_lib[cur_mos->drain]->min_x = min(net_lib[cur_mos->drain]->min_x, cur_x);
                net_lib[cur_mos->drain]->max_x = max(net_lib[cur_mos->drain]->max_x, cur_x);
                net_lib[cur_mos->cpair->drain]->min_x = min(net_lib[cur_mos->cpair->drain]->min_x, cur_x);
                net_lib[cur_mos->cpair->drain]->max_x = max(net_lib[cur_mos->cpair->drain]->max_x, cur_x);
            }
            // update x for source
            if(cur_mos == mos_head){
                cur_x += 49.5;
            }
            else if(cur_mos->next){
                cur_x += 54;
            }
            else{
                cur_x += 49.5;
            }

            net_lib[cur_mos->source]->min_x = min(net_lib[cur_mos->source]->min_x, cur_x);
            net_lib[cur_mos->source]->max_x = max(net_lib[cur_mos->source]->max_x, cur_x);
            net_lib[cur_mos->cpair->source]->min_x = min(net_lib[cur_mos->cpair->source]->min_x, cur_x);
            net_lib[cur_mos->cpair->source]->max_x = max(net_lib[cur_mos->cpair->source]->max_x, cur_x);
        }
        cur_mos = cur_mos->next;
    }
    double total_cost = 0;
    for(const auto& net : net_lib){
        total_cost += net.second->max_x - net.second->min_x;
        net.second->max_x = 0;
        net.second->min_x = INT_MAX;
        if(net.second->cross)
            total_cost += cross_cost;
    }
    return total_cost;
}