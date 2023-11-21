#include "B_tree.h"


B_tree::B_tree(char *file_name){
    ifstream fin(file_name);
    string line;
    getline(fin, line);
    stringstream ss(line);
    ss >> R_low >> R_upper;
    ss.clear();
    while(getline(fin, line)){
        ss.str(line);
        int w, h;
        string block_name;
        ss >> block_name >> w >> h;
        Block *new_block = new Block;
        new_block->w = w;
        new_block->h = h;
        new_block->name = block_name;
        new_block->rotated = false;
        new_block->left = nullptr;
        new_block->right = nullptr;
        new_block->parent = nullptr;
        block_list.push_back(new_block);
        ss.clear();
    }
}

B_tree::~B_tree(){
    for(auto& b : block_list)
        delete b;
}

void B_tree::solve(){
    // cout<<"build skyline"<<endl;
    sky = skyline();
    // cout<<"build tree"<<endl;
    build_tree();
    // cout<<"packing"<<endl;
    packing();
    // cout << "SA" << endl;
    FastSA();
    // classicalSA();
}

void B_tree::build_tree(){
    block_size = block_list.size();
    root = block_list[0];
    for(int i=0;i<block_size;i++){
        int left = 2*i + 1;
        int right = left + 1;
        block_list[i]->left = nullptr;
        block_list[i]->right = nullptr;
        if(left < block_size){
            block_list[i]->left = block_list[left];
            block_list[left]->parent = block_list[i];
            block_list[left]->is_left = true;
        }
        if(right < block_size){
            block_list[i]->right = block_list[right];
            block_list[right]->parent = block_list[i];
            block_list[right]->is_left = false;
        }
    }
}

void B_tree::packing(){
    // pre-order traverse the tree to get total width and height;
    // width = 0; height = 0; // width and height for all floorplan
    // show_tree();
    root->x = 0;
    root->y = 0;
    sky.initial();
    sky.insert(0, root->w, root->h);
    width = root->w; height = root->h;
    stack<Block*> s;
    s.push(root->right);
    s.push(root->left);
    int size = 0;

    while(!s.empty()){
        Block *cur_block = s.top();
        s.pop();
        if(!cur_block)
            continue;
        int w = cur_block->w;
        int h = cur_block->h;

        // cout<<cur_block->name<<" "<<w<<" "<<h <<" rotated : " << cur_block->rotated<< " is left : " << cur_block->is_left << endl;
        if(cur_block->is_left){
            cur_block->x = cur_block->parent->x + cur_block->parent->w;
            cur_block->y = sky.insert(cur_block->x, cur_block->x + w, h);
        }
        else{
            cur_block->x = cur_block->parent->x;
            cur_block->y = sky.insert(cur_block->x, cur_block->x + w, h);
        }

        // update width and height for floorplan
        width = max(width, cur_block->x + w);
        height = max(height, cur_block->y + h);

        // sky.show();
        if(cur_block->right)
            s.push(cur_block->right);
        if(cur_block->left)
            s.push(cur_block->left);
    }

    area = width * height;
}

void B_tree::update_best_area(){
    double ar = width / double(height);
    bool meet_ratio = (ar < R_upper) && (ar > R_low);
    if(cur_area < best_area && meet_ratio || (force_legal && meet_ratio && !accept_best)){ 
        accept_best = true;
        best_area = cur_area;
        best_width = width;
        best_height = height;
        // cout<<"update best area"<<endl;
        for(auto& b : block_list){
            b->best_x = b->x;
            b->best_y = b->y;
            b->best_rotated = b->rotated;
        }
    }
}

void B_tree::getFastSA_parameter(){
    for(int i=0;i<100;i++){
        int op = rand()%3;
        if(op == 0){
            random_rotate(1);
        }
        else if(op == 1){
            random_swap(1);
        }
        else{
            random_delete_insert(1);
        }
    }
    delta_avg /= 100;
    delta_avg = abs(delta_avg);
    ave_area /= 100;
}

void B_tree::FastSA(){
    accept_best = false;
    // initialize
    best_area = area; cur_area = area;
    best_width = width; cur_width = width;
    best_height = height; cur_height = height;
    update_best_area();
    getFastSA_parameter();
    if(block_size < 20)
        build_tree();
    initial_temp = delta_avg / log(1.001);
    temperature = initial_temp;
    SA();
    if(!accept_best){ // use classical SA to guarantee a legal solution
        build_tree();
        temperature = 10000;
        classicalSA();
    }
}
void B_tree::SA(){
    // temperature = 100000;
    int count = 0; // early stop count
    int prev_best = best_area;
    double lambda = 0.8;
    double stop_temperature = 1e-5;
    double cost_constant = 1e-10;
    int iter_time = max(block_size * 10, 1500); // iteration per temperature
    bool stop = false;
    n=1;
    while(!stop){
        delta_cost = 0;
        for(int i=0;i<iter_time;i++){
            // cout<<"iter : " << i << " Cost : "<< cur_area << endl;
            int op = rand()%3;
            if(op == 0){
                random_rotate(false);
            }
            else if(op == 1){
                random_swap(false);
            }
            else{
                random_delete_insert(false);
            }
        }
        delta_cost /= iter_time;
        delta_cost /= ave_area;
        delta_cost = abs(delta_cost) + cost_constant;
        
        // cout << "n == " << n << endl;
        // cout<<"temperature : " << temperature << " Cost : "<< cur_area << endl;

        temperature = initial_temp * delta_cost;
        if(n <= k){
            temperature /= (n * c);
        }
        else{
            force_legal = true;
            temperature /= (n);
        }
        if(prev_best == best_area){
            count++;
            if(count == 10)
                break;
        }
        else{
            prev_best = best_area;
            count = 0;
        }
        if(temperature < stop_temperature && n >= 10)
            stop = true;
        n++;
    }
}


void B_tree::classicalSA(){
    accept_best = false;
    // initialize
    best_area = area; cur_area = area;
    best_width = width; cur_width = width;
    best_height = height; cur_height = height;
    update_best_area();

    temperature = 100000;
    double lambda = 0.85;
    double stop_temperature = 1e-10;
    int iter_time = max(block_size * 10, 1000); // iteration per temperature
    bool stop = false;
    while(!stop){
        for(int i=0;i<iter_time;i++){
            // cout<<"iter : " << i << " Cost : "<< cur_area << endl;
            int op = rand()%3;
            if(op == 0){
                random_rotate(0);
            }
            else if(op == 1){
                random_swap(0);
            }
            else{
                random_delete_insert(0);
            }
        }
        // cout<<"temperature : " << temperature << " Cost : "<< cur_area << endl;
        temperature *= lambda;
        if(temperature < 1000)
            force_legal = true;
        if(temperature < stop_temperature)
            stop = true;
    }
}

void B_tree::random_rotate(bool force_accept){
    int block_idx = rand() % block_size;
    Block *cur_block = block_list[block_idx];
    // cout << "rotate : " << cur_block->name << endl;
    // rotate
    cur_block->rotated = !cur_block->rotated;
    // swap w h
    int tmp = cur_block->w;
    cur_block->w = cur_block->h;
    cur_block->h = tmp;
    packing(); // get new cost
    int new_cost = area;
    int old_cost = cur_area;

    // sa process
    bool accept = false; // accept new solution
    int delta_c = new_cost - old_cost;
    if(delta_c <= 0){ // down-hill move
        accept = true;
    }
    else{ // up-hill move
        double p = rand() % 100 / 100.0;
        if(p < exp(-delta_c / temperature))
            accept = true;
    }

    // FastSA
    if(force_accept){
        ave_area += area;
        delta_avg += delta_c;
    }

    double ar = width / double(height);
    bool meet_ratio = ((ar < R_upper) && (ar > R_low)) || !accept_best;

    if(accept && meet_ratio || force_accept || (force_legal && meet_ratio && !accept_best)){
        delta_cost += delta_c;
        // cout << "accept solution " << area << endl;
        cur_area = area; 
        update_best_area();
    }
    else{
        // rotate back
        cur_block->rotated = !cur_block->rotated;
        int tmp = cur_block->w;
        cur_block->w = cur_block->h;
        cur_block->h = tmp;
    }
}

void B_tree::swap_deletion(Block *first, Block *second){
    swapElements(first->w, second->w);
    swapElements(first->h, second->h);
    swapElements(first->best_x, second->best_x);
    swapElements(first->best_y, second->best_y);
    swapElements(first->name, second->name);
    swapElements(first->rotated, second->rotated);
    swapElements(first->best_rotated, second->best_rotated);
}

Block* B_tree::deletion(Block *cur_block, int& insert_state){
    Block *return_block = nullptr;
    Block *cur = cur_block;
    while(cur->left){ // default replace by left child
        swap_deletion(cur, cur->left);
        cur = cur->left;
    }

    if(cur->right){
        if(cur == root){
            root = cur->right;
            cur->right->parent = nullptr;
        }
        else{
            cur->right->parent = cur->parent;
            if(cur->is_left){
                cur->parent->left = cur->right;
                cur->right->is_left = true;
            }
            else{
                cur->parent->right = cur->right;
                cur->right->is_left = false;
            }
        }
        return_block = cur->right;
        cur->right = nullptr;
        insert_state = 1;
    }
    else{ // no more child
        if(cur->is_left){
            cur->parent->left = nullptr;
            insert_state = 0;
        }
        else{
            cur->parent->right = nullptr;
            insert_state = 2;
        }
        return_block = cur->parent;
        cur->parent = nullptr;
    }
    deleted_block = cur;
    return return_block;
}

void B_tree::insertion(Block *cur_block, Block *target_block, int& insert_state){
    // default insert cur to target position
    // target become left child
    insert_state = rand() % 4;
    // 0 replace target block, insert target_block to cur_block->rihgt
    // 1 raplace target block, insert target_block to cur_block->left
    // 2 replace cur_block to target_block's right child
    // 3 replace cur_block to target_block's left child
    if(insert_state == 0){
        cur_block->left = nullptr;
        if(target_block == root){
            root = cur_block;
            cur_block->right = target_block;
            target_block->is_left = false;
            target_block->parent = cur_block;
        }
        else{
            // update cur block
            cur_block->parent = target_block->parent;
            cur_block->is_left = target_block->is_left;
            
            // update target's parent
            if(cur_block->is_left)
                cur_block->parent->left = cur_block;
            else
                cur_block->parent->right = cur_block;
            // update target_block
            cur_block->right = target_block;
            target_block->is_left = false;
            target_block->parent = cur_block;
        }
    }
    else if(insert_state == 1){
        cur_block->right = nullptr;
        if(target_block == root){
            root = cur_block;
            cur_block->left = target_block;
            target_block->is_left = true;
            target_block->parent = cur_block;
        }
        else{
            // update cur block
            cur_block->parent = target_block->parent;
            cur_block->is_left = target_block->is_left;
            
            // update target's parent
            if(cur_block->is_left)
                cur_block->parent->left = cur_block;
            else
                cur_block->parent->right = cur_block;
            // update target_block
            cur_block->left = target_block;
            target_block->is_left = true;
            target_block->parent = cur_block;
        }
    }
    else if(insert_state == 2){
        cur_block->parent = target_block;
        cur_block->right = nullptr;
        cur_block->left = nullptr;
        if(target_block->right){
            cur_block->right = target_block->right;
            cur_block->right->parent = cur_block;
            target_block->right = cur_block;
            cur_block->is_left = false;
        }
        else{
            target_block->right = cur_block;
            cur_block->is_left = false;
        }
    }
    else if(insert_state == 3){
        cur_block->parent = target_block;
        cur_block->right = nullptr;
        cur_block->is_left = true;
        if(target_block->left){
            cur_block->left = target_block->left;
            cur_block->left->parent = cur_block;
            target_block->left = cur_block;
        }
        else{
            cur_block->left = nullptr;
            target_block->left = cur_block;
        }
    }
}

void B_tree::deletion_back(Block *cur_block, int& insert_state){
    // 0 replace target block, insert target_block to cur_block->right
    // 1 raplace target block, insert target_block to cur_block->left
    // 2 insert cur_block to target_block's right child
    // 3 insert cur_block to target_block's left child
    if(insert_state == 1){
        if(cur_block == root){
            root = cur_block->left;
            cur_block->left->parent = nullptr;
        }
        else{
            // update left child
            cur_block->left->parent = cur_block->parent;
            cur_block->left->is_left = cur_block->is_left;
            // update parent
            if(cur_block->is_left)
                cur_block->parent->left = cur_block->left;
            else
                cur_block->parent->right = cur_block->left;
        }
    }
    else if(insert_state == 0){
        if(cur_block == root){
            root = cur_block->right;
            cur_block->right->parent = nullptr;
        }
        else{
            // update left child
            cur_block->right->parent = cur_block->parent;
            cur_block->right->is_left = cur_block->is_left;
            // update parent
            if(cur_block->is_left)
                cur_block->parent->left = cur_block->right;
            else
                cur_block->parent->right = cur_block->right;
        }
    }
    else if(insert_state == 2){
        if(cur_block->right){
            cur_block->parent->right = cur_block->right;
            cur_block->right->parent = cur_block->parent;
        }
        else{
            cur_block->parent->right = nullptr;
        }
    }
    else{
        if(cur_block->left){
            cur_block->parent->left = cur_block->left;
            cur_block->left->parent = cur_block->parent;
        }
        else{
            cur_block->parent->left = nullptr;
        }
    }

    // update cur
    cur_block->parent = nullptr;
    cur_block->right = nullptr;
    cur_block->left = nullptr;
}

void B_tree::insertion_back(Block *cur_block, Block *old_position, Block *target_block, int insert_state){
    // for insert back  0->insert to old_position left, 
    // 2->insert to old_position right
    // 1->replace old_position, then old_position block become right child
    // finally up swapping until block == cur_block;
    if(insert_state == 0){
        old_position->left = cur_block;
        cur_block->parent = old_position;
        cur_block->is_left = true;
    }
    else if(insert_state == 2){
        old_position->right = cur_block;
        cur_block->parent = old_position;
        cur_block->is_left = false;
    }
    else{
        // update cur_block
        cur_block->parent = old_position->parent;
        cur_block->is_left = old_position->is_left;
        cur_block->right = old_position;
        // update parent
        if(old_position == root){
            root = cur_block;
        }
        else{
            if(cur_block->is_left)
                cur_block->parent->left = cur_block;
            else
                cur_block->parent->right = cur_block;
        }
        // update old_postion
        old_position->parent = cur_block;
        old_position->is_left = false;
    }

    while(cur_block != target_block){
        swap_deletion(cur_block, cur_block->parent);
        cur_block = cur_block->parent;
    }
}

void B_tree::random_delete_insert(bool force_accept){
    // show_tree();
    int block_idx = rand() % block_size;
    int target_idx = rand() % block_size;
    if(block_idx == target_idx)
        return ;
    Block *cur_block = block_list[block_idx];
    // cout << "move : " << cur_block->name << endl;

    int insert_back_state = 0;       // for insert back  0->insert to old_position left, 
                                // 1->replace old_position, then old_position block become right child
                                // 2->insert to old_position right
                                // finally up swapping until block == cur_block;
    Block *old_position = deletion(cur_block, insert_back_state);
    // cout << "Deleted block name : " << deleted_block->name << endl;
    if(deleted_block == block_list[target_idx]){
        insertion_back(deleted_block, old_position, cur_block, insert_back_state);
        // show_tree();
        return ;
    }
    // cout << "after deletion " << endl;
    // show_tree();
    Block *target_block = block_list[target_idx];
    int insert_state;
    // cout << "insert to : " << target_block->name << endl;
    insertion(deleted_block, target_block, insert_state);
    // cout << "after insertion " << endl;
    // show_tree();
    // cout<< " packing " << endl;
    packing(); // get new cost
    int new_cost = area;
    int old_cost = cur_area; 
    
    // sa process
    bool accept = false; // accept new solution
    int delta_c = new_cost - old_cost;
    if(delta_c <= 0){ // down-hill move
        accept = true;
    }
    else{ // up-hill move
        double p = rand() % 100 / 100.0;
        // cout << p << " " << exp(-delta_c / temperature) << endl;
        if(p < exp(-delta_c / temperature))
            accept = true;
    }

    // FastSA
    if(force_accept){
        ave_area += area;
        delta_avg += delta_c;
    }

    double ar = width / double(height);
    bool meet_ratio = ((ar < R_upper) && (ar > R_low)) || !accept_best;

    if(accept && meet_ratio || force_accept || (force_legal && meet_ratio && !accept_best)){
        delta_cost += delta_c;
        // cout << "accept solution " << area << endl;
        cur_area = area; 
        update_best_area();
    }
    else{
        // insert back
        // cout << "after deletion back" << endl;
        deletion_back(deleted_block, insert_state);
        // show_tree();
        // cout << "after insertion back" << endl;
        insertion_back(deleted_block, old_position, cur_block, insert_back_state);
        // show_tree();
    }
}

void B_tree::swap_wh(Block *first, Block *second){
    // only swap wh and left flag for packing
    swapElements(first->w, second->w);
    swapElements(first->h, second->h);
    // swapElements(first->name, second->name);
}

void B_tree::swap_all(Block *first, Block *second){
    // accept solution -> swap all except wh are already swap
    swapElements(first->best_x, second->best_x);
    swapElements(first->best_y, second->best_y);
    swapElements(first->name, second->name);
    swapElements(first->rotated, second->rotated);
    swapElements(first->best_rotated, second->best_rotated);
}

void B_tree::random_swap(bool force_accept){
    int b1_idx = rand()%block_size; 
    int b2_idx = rand()%block_size; 
    Block *first = block_list[b1_idx];
    Block *second = block_list[b2_idx];
    // cout << "swap : " << first->name << " " << second->name<<endl;
    swap_wh(first, second);
    packing();

    int new_cost = area;
    int old_cost = cur_area;

    bool accept = false; // accept new solution
    int delta_c = new_cost - old_cost;
    if(delta_c <= 0){ // down-hill move
        accept = true;
    }
    else{ // up-hill move
        double p = rand() % 100 / 100.0;
        if(p < exp(-delta_c / temperature))
            accept = true;
    }

    // FastSA
    if(force_accept){
        ave_area += area;
        delta_avg += delta_c;
    }

    double ar = width / double(height);
    bool meet_ratio = ((ar < R_upper) && (ar > R_low)) || !accept_best;
    if(accept && meet_ratio || force_accept || (force_legal && meet_ratio && !accept_best)){
        delta_cost += delta_c;
        // cout << "accept solution " << area << endl;
        swap_all(first, second);
        cur_area = area; 
        update_best_area();
    }
    else{
        // swap back
        swap_wh(first, second);
    }
}

void B_tree::output(char *file_name){
    ofstream fout(file_name);
    fout << "A = " << best_area << endl;
    fout << "R = " << double(best_width) / double(best_height) << endl; 
    for(auto b : block_list){
        fout<<b->name<<" "<<b->best_x<<" "<<b->best_y;
        if(b->best_rotated)
            fout << " R";
        fout << endl;
    }
}

void B_tree::show_tree(){
    unordered_map<string, bool> m;
    queue<Block *> q;
    q.push(root);
    int size = 1;
    cout << "----------- show tree --------" << endl;
    while(!q.empty()){
        while(size--){
            Block *cur = q.front();
            q.pop();
            if(!cur){
                cout <<"- ";
                continue;
            }
            if(m[cur->name]){
                cout << "tree is cyclic " << endl;
                cout << "name : " << cur->name << endl;
                exit(0);
            }
            m[cur->name] = true;
            cout << cur->name << " ";
            q.push(cur->left);
            q.push(cur->right); 
        }
        cout << endl;
        size = q.size();
    }
    cout << "-----------  end  ------------" << endl;
}