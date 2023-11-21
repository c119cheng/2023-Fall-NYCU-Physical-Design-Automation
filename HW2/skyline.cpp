#include "skyline.h"

skyline::skyline(){

}

void skyline::initial(){
    while(root){ // reset list
        recycle.push(root);
        root = root->next;
    }

    // initial list
    root = getNode();
    root->x1 = 0;
    root->x2 = INT_MAX;
    root->y = 0;
    tail = root;
    root->next = nullptr;
    root->prev = nullptr;
}

skyline::~skyline(){
    while(root){
        node* tmp = root->next;
        delete root;
        root = tmp;
    }
    while(!recycle.empty()){
        delete recycle.front();
        recycle.pop();
    }
}

node* skyline::getNode(){
    node *new_node;
    if(!recycle.empty()){
        new_node = recycle.front();
        recycle.pop();
    }
    else{
        new_node = new node;
    }
    return new_node;
}

int skyline::insert(const int& x1, const int& x2, const int& h){
    // start x-coordinate, end x-coordinate, height of input block
    // return the left down y-coordinate of block 
    // cout << "insert x1 and x2 : "<< x1 << " " << x2 << endl;
    node *left, *right;
    left = root;

    if(x1 == 0){ // spacial case when start at x=0
        int max_y = root->y;
        right = root;
        
        while(right->next && right->x2 < x2){
            node *tmp = right->next;
            max_y = max(max_y, tmp->y);
            recycle.push(right);
            right = tmp;
        }
        if(right->x2 == x2){
            root = right;
            right->x1 = 0;
            right->y = max_y + h;
        }
        else if(right == root){ // place upper root fragment

            node *new_node = getNode();
            // update new node
            new_node->x1 = x2;
            new_node->x2 = root->x2;
            new_node->y = root->y;
            new_node->prev = root;
            if(root->next)
                new_node->next = root->next;
            else{ // root == tail
                new_node->next = nullptr;
                tail = new_node;
            }
            // update right
            root->y = max_y + h;
            root->x2 = x2;
            root->next = new_node;

        }
        else{ 
            node *new_node = getNode();

            // update new node -> new root
            new_node->x1 = 0;
            new_node->x2 = x2;
            new_node->next = right;
            new_node->prev = nullptr;
            new_node->y = max_y + h;

            // update right
            right->x1 = x2;
            right->prev = new_node;

            root = new_node;
        }
        
        return max_y;
    }

    while(left && left->x2 < x1){ // find left fragment (left.x2 >= x1)
        left = left->next;       
    }
    int max_y;
    if(left->x2 == x1){ // ignore left->y for max_y
        right = left->next;
        max_y = right->y;
    }
    else{
        right = left->next;
        max_y = left->y;
    }

    while(right->next && right->x2 < x2){
        node *tmp = right->next;
        max_y = max(max_y, tmp->y);
        recycle.push(right);
        right = tmp;
    }
    
    if(x2 <=  left->x2){ // is on the same segment (left)
        node *new_node[2];
        for(int i=0;i<2;i++)
            new_node[i] = getNode();

        int old_x2 = left->x2;
        node *old_next = left->next;
        // update left part
        left->next = new_node[0];
        left->x2 = x1;

        // update new block information
        new_node[0]->prev = left;
        new_node[0]->next = new_node[1];
        new_node[0]->x1 = x1;
        new_node[0]->x2 = x2;
        new_node[0]->y = max_y + h;

        // update new tail information
        if(old_x2 != x2){
            new_node[1]->prev = new_node[0];
            new_node[1]->next = old_next;
            old_next->prev = new_node[1];
            new_node[1]->x1 = x2;
            new_node[1]->x2 = old_x2;
            new_node[1]->y = left->y;
        }
        else{
            new_node[0]->next = old_next;
            old_next->prev = new_node[0];
            recycle.push(new_node[1]);
        }
    }
    else if(x1 == right->x1 && x2 == right->x2){ // totally on right segment
        right->y = max_y + h;
        return max_y;
    }
    else if(right->x2 == x2){
        left->x2 = x1;
        left->next = right;
        right->prev = left;
        right->x1 = x1;
        right->y = max_y + h;
        return max_y;
    }
    else{ // new block is between left and right
        node *new_node = getNode();
        
        // update left
        left->next = new_node;
        left->x2 = x1;

        // update new node
        new_node->prev = left;
        new_node->next = right;
        new_node->x1 = x1;
        new_node->x2 = x2;
        new_node->y = max_y + h;

        // update right
        right->prev = new_node;
        right->x1 = x2;

        return max_y;
    }
    return -1;
}

void skyline::show(){
    node *tmp = root;
    bool flag = false;
    while(tmp){
        if(tmp->x1 == tmp->x2){
            cout << "x1 == x2" << endl;
            flag = true;
            break;
        } 
        tmp = tmp->next;
    }
    if(!flag)
        return ;
    cout << "---------- show skyline ----------"<<endl;;
    node *cur = root;
    while(cur){
        if(cur->next && cur->next->x1 != cur->x2){
            cout << cur->next->x1 << " " << cur->next->x2 << "  " << cur->next->y << endl;
            cout << "next x1 != x2" << endl;
            exit(0);
        }
        if(cur->x1 == cur->x2){
            cout << "x1 == x2" << endl;
            // exit(0);
        }
        cout << cur->x1 << "  " << cur->x2 << "  " << cur->y <<endl;
        cur = cur->next;
    }
    exit(0);
    cout << "-----------    end     ----------------"<<endl;
}