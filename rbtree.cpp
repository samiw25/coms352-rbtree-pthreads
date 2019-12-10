/**
 * rbtree.cpp contains the implementations of the functions of a multithreaded red black tree
 * using a readers/writers format in which readers have priority.
 * 
 * Author: Samantha Williams
 * November 12, 2019
 **/

#include "rbtree.h"
#include <pthread.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <queue>
#include <mutex>
#include <chrono>

using namespace std;
pthread_t tid;
mutex x;
mutex wsem;
mutex rsem;
bool done = false;
queue<int> search_queue;
queue<string> modify_queue;
chrono::high_resolution_clock::time_point start, complete;
vector<string> thread_results;
RBtree *tree;

/**
 * search returns true if the input integer is present in the tree
 * and false if it is not.
 * */
bool RBtree::search(int num) {
    Node *tmp = root;
    do {
        if (tmp->key == num) {
            return true;
        //if node's key is greater, check it's left children
        } else if (tmp->key > num) {
            tmp = tmp->left;
        //if node's key is less than, check it's right children
        } else {
            tmp = tmp->right;
        }
    } while (tmp != nil);
    return false;
}

/**
 * get_minimum is a helper search function that returns the node with the smallest key value that belongs
 * to the subtree starting at node n.
 * */
Node * RBtree::get_minimum(Node *n) {
    Node *tmp = n;
    while (tmp->left != nil) {
        tmp = tmp->left;
    }
    return tmp;
}

/**
 * delete_node removes the node with the inputted key value from the tree
 * if it exists. It calls helper functions to maintain the red black characteristics.
 * */
void RBtree::delete_node(int num) {
    Node *n = root;
    Node *x = NULL;
    //find the node with this integer value
    while (n->key != num) {
        if (n->key > num) {
            n = n->left;
        } else {
            n = n->right;
        }
    }
    Node *tmp = n;
    bool orig_color = tmp->color;
    
    if (n->left == nil) {
        x = n->right;
        transplant(n, n->right);
    } else if (n->right == nil) {
        x = n->left;
        transplant(n, n->left);
    } else {
        tmp = get_minimum(n->right);
        orig_color = tmp->color;
        x = tmp->right;
        if (tmp->parent == n) {
            x->parent = tmp;
        } else {
            transplant(tmp, tmp->right);
            tmp->right = n->right;
            tmp->right->parent = tmp;
        }
        transplant(n, tmp);
        tmp->left = n->left;
        tmp->left->parent = tmp;
        tmp->color = n->color;
    }
    if (!orig_color) {
        delete_node_fixup(x);
    }
}

/**
 * insert inserts a new node with the given key value into the tree. It calls helper
 * functions to maintain the red black characteristics.
 * */
void RBtree::insert(int num) {
    Node *n = new Node();
    n->key = num;
    //assume color is red at start
    n->color = true;
    Node *x = root;
    Node *y = nil;
    //figure out where n should be placed based on normal BST
    while (x != nil) {
        //y saves the what should be the parent node of n
        y = x;
        if (n->key < x->key) {
            x = x->left;
        } else {
            x = x->right;
        }
    }
    n->parent = y;
    //is n root, right child, or left child?
    if (y == nil) {
        root = n;
    } else if (n->key < y->key) {
        y->left = n;
    } else {
        y->right = n;
    }
    n->left = nil;
    n->right = nil;
    //fix any rb properties that were violated during insertion
    insert_fixup(n);
}

/**
 * transplant is a helper method for deletions that changes the relationships of the nodes during deletion.
 * */
void RBtree::transplant(Node *n1, Node *n2) {
    if (n1->parent == nil) {
        root = n2;
    } else if (n1 == n1->parent->left) {
        n1->parent->left = n2;
    } else {
        n1->parent->right = n2;
    }
    n2->parent = n1->parent;
}

/**
 * delete_node_fixup is a helper method that ensures the properties of the red black tree are maintained during deletion.
 * */
void RBtree::delete_node_fixup(Node *n) {
    Node *tmp = NULL;
    while (n != root && n->color == false) {
        if (n == n->parent->left) {
            tmp = n->parent->right;
            if (tmp->color) {
                tmp->color = false;
                n->parent->color = true;
                left_rotate(n->parent);
                tmp = n->parent->right;
            }
            if (tmp->left->color == false && tmp->right->color == false) {
                tmp->color = true;
                n = n->parent;
            } else {
                if (!tmp->right->color) {
                    tmp->left->color = false;
                    tmp->color = true;
                    right_rotate(tmp);
                    tmp = n->parent->right;
                }
                tmp->color = n->parent->color;
                n->parent->color = false;
                tmp->right->color = false;
                left_rotate(n->parent);
                n = root;
            }
        } else {
            tmp = n->parent->left;
            if (tmp->color) {
                tmp->color = false;
                n->parent->color = true;
                right_rotate(n->parent);
                tmp = n->parent->left;
            }
            if (tmp->right->color == false && tmp->left->color == false) {
                tmp->color = true;
                n = n->parent;
            } else {
                if (!tmp->left->color) {
                    tmp->right->color = false;
                    tmp->color = true;
                    left_rotate(tmp);
                    tmp = n->parent->left;
                }
                tmp->color = n->parent->color;
                n->parent->color = false;
                tmp->left->color = false;
                right_rotate(n->parent);
                n = root;
            }
        }
        n->color = false;
    }
}

/**
 * insert_fixup is a helper method that ensures the properties of the red black tree are maintained during insertion.
 * */
void RBtree::insert_fixup(Node *n) {
    Node *tmp = NULL;
    while (n->parent->color) {
        //n's parent is a left child
        if (n->parent == n->parent->parent->left) {
            tmp = n->parent->parent->right;
            //case 1: n's parent's parent's right child is red; n's parent is red, but does not have 2 black children
            if (tmp->color) {
                n->parent->color = false;
                tmp->color = false;
                n->parent->parent->color = true;
                n = n->parent->parent;
                //n's parent's parent's right child is black
            } else {
                //case 2: n is a right child
                if (n == n->parent->right) {
                    n = n->parent;
                    left_rotate(n);
                }
                //case 3: red node with a red child
                n->parent->color = false;
                n->parent->parent->color = true;
                right_rotate(n->parent->parent);
            }
            //n's parent is a right child
        } else {
            tmp = n->parent->parent->left;
            //case 1: n's parent's parent's left child is red; n's parent is red, but does not have 2 black children
            if (tmp->color) {
                n->parent->color = false;
                tmp->color = false;
                n->parent->parent->color = true;
                n = n->parent->parent;
                ////n's parent's parent's left child is black
            } else {
                //case 2: n is a left child
                if (n == n->parent->left) {
                    n = n->parent;
                    right_rotate(n);
                }
                //case 3: red node with a red child
                n->parent->color = false;
                n->parent->parent->color = true;
                left_rotate(n->parent->parent);
            }
        }
    }
    //make the root black
    root->color = false;
}

/**
 * left_rotate performs a left rotation on the node n and its children
 * */
void RBtree::left_rotate(Node *n) {
    Node *tmp = n->right;
    n->right = tmp->left;
    if (tmp->left != nil) {
        tmp->left->parent = n;
    }
    tmp->parent = n->parent;
    if (n->parent == nil) {
        root = tmp;
    } else if (n == n->parent->left) {
        n->parent->left = tmp;
    } else {
        n->parent->right = tmp;
    }
    tmp->left = n;
    n->parent = tmp;
}

/**
 * right_rotate performs a right rotation on the node n and its children
 * */
void RBtree::right_rotate(Node *n) {
    Node *tmp = n->left;
    n->left = tmp->right;
    if (tmp->right != nil) {
        tmp->right->parent = n;
    }
    tmp->parent = n->parent;
    if (n->parent == nil) {
        root = tmp;
    } else if (n == n->parent->right) {
        n->parent->right = tmp;
    } else {
        n->parent->left = tmp;
    }
    tmp->right = n;
    n->parent = tmp;
}

/**
 * in_order_print is a recursive function to print the tree in order from the input node n
 * */
void RBtree::preorder_print(Node *n, ofstream& out) {
    if (n == nil) {
        out << ",f";
        return;
    }
    char col = 'b';
    if (n->color) {
        col = 'r';
    }
    if (n != root) {
        out << ",";
    }
    out << n->key << col;
    preorder_print(n->left, out);
    preorder_print(n->right, out);
}

/**
 * create_tree_helper is a recursive function that adds the nodes to the tree in preorder as they're listed.  It takes in a pointer to 
 * the tree, a vector containing the keys in preorder, a vector containing the colors in preorder, and a pointer to the index sharer
 * by the recursive calls of the function. It returns a pointer to the root of the tree.
 * */
Node * create_tree_helper(RBtree *t, vector<int> keys, vector<char> cols, int *index_ptr) {
    int index = *index_ptr;
    //check if the node is a leaf. If we haven't reached the end of the vectors, increment the pointer
    if (cols[index] == 'f') {
        if (index < keys.size()-1) {
            (*index_ptr)++;
        }
        return t->nil;
    }
    Node *temp = new Node();
    temp->key = keys[index];
    if (cols[index] == 'r') {
        temp->color = true;
    } else {
        temp->color = false;
    }
    (*index_ptr)++;
    //recursively create the node's children.
    temp->left = create_tree_helper(t, keys, cols, index_ptr);
    temp->left->parent = temp;
    temp->right = create_tree_helper(t, keys, cols, index_ptr);
    temp->right->parent = temp;
    return temp;
}

/**
 * create_tree takes in a vector of strings that contains the nodes of the tree as inputted by the input file.
 * After separating the keys and the colors, it calls create_tree_helper to add the nodes to the tree.
 * Returns a pointer to the tree.
 * */
RBtree * create_tree(vector<string> nodes) {
    RBtree *tree = new RBtree();
    //if the tree is null
    if (nodes[0] == "NULL" || nodes[0] == "null" || nodes[0] == "f") {
        return tree;
    }
    vector<int> values;
    vector<char> colors;
    //separate the node values from the colors
    for (int i=0; i<nodes.size(); i++) {
        //if the value is just 'f', push -1 so we know it's a nil node
        if (nodes[i].length() != 1) {
            values.push_back(stoi(nodes[i]));
        } else {
            values.push_back(-1);
        }
        colors.push_back(nodes[i].back());
    }
    //add the tree nodes
    int index = 0;
    tree->root = create_tree_helper(tree, values, colors, &index);
    tree->root->parent = tree->nil;
    return tree;
}
/**
 * reader is the function that the search threads run in. It takes the thread_id as input and runs the function at the beginning
 * of the search_queue once signaled that it can begin
 * */
void *(reader)(void *arg) {
	//wait for main thread to finish creating all threads
	while (!done) {;}
	if (!search_queue.empty()) {
		rsem.lock();	
		//search
		int num = search_queue.front();
        bool s = tree->search(num);
        string tf;
        if (s) {
            tf = "true";
        } else {
            tf = "false";
        }
        string result = "search(" + to_string(num) + ")->" + tf + ", performed by thread: " + to_string((long) arg);
        thread_results.push_back(result);
		search_queue.pop();
	}
	//check if there are any more readers, otherwise signal writers		
	if (!search_queue.empty()) {	
		rsem.unlock();
	}
	else {
		wsem.unlock();
	}
	return NULL;
}

/**
 * writer is the function that the modify threads run in. It takes the thread_id as input and runs the function at the beginning
 * of the modify_queue once signaled that it can begin
 * */
void *(writer)(void *arg) {
	//wait for main thread to finish creating all threads
	while (!done) {;}
	while (!search_queue.empty()) {;}
	wsem.lock();
	//modify
	if (!modify_queue.empty()) {
		string com = modify_queue.front();
		modify_queue.pop();
		string letter = com.substr(0, 1);
		com.erase(0, 1);
		int num = stoi(com);
		if (letter == "i") {
			tree->insert(num);
            string result = "insert(" + to_string(num) + "), performed by thread: " + to_string((long)arg);
            thread_results.push_back(result);
		}
		else if (letter == "d") {
			tree->delete_node(num);
            string result = "delete(" + to_string(num) + "), performed by thread: " + to_string((long)arg);
            thread_results.push_back(result);
		}
	}
    wsem.unlock();
	return NULL;
}

/**
 * create_threads creates the search and modify threads specified by the input integers search_threads and modify_threads
 * it then parses commands and inputs for the search and modify commands and adds them to their respective queues in order
 * to be executed by the appropriate threads.
 * */
void RBtree::create_threads(int search_threads, int modify_threads, vector<string> commands, vector<int> inputs) {
    //fill the queues from the commands
    for (int k=0; k<commands.size(); k++) {
        if (commands[k] == "s") {
            search_queue.push(inputs[k]);
        } else {
            modify_queue.push(commands[k]+" "+to_string(inputs[k]));
        }
    }
    int num_threads = search_threads + modify_threads;
    pthread_t threads[num_threads];
    //create the reader threads
    for (int i=0; i<search_threads; i++) {
		pthread_create(&threads[i], NULL, reader, (void *)i);
    }
    //create the writer threads
    for (int j=search_threads; j<num_threads; j++) {
		pthread_create(&threads[j], NULL, writer, (void *)j);
	}
	done = true;
}

/**
 * read_file parses the input file for the tree nodes, thread information, and function calls.
 * Returns 1 if an error occurs while opening or reading the file.
 * */
int read_file(char *filename) {
    ifstream file(filename);
    string line, line2, line3;
    //separate the line into the nodes' information
    vector<string> nodes;
    getline(file, line);
    string del = ",";
    string tok;
    size_t pos = 0;
    while ((pos = line.find(del)) != string::npos) {
        tok = line.substr(0, pos);
        nodes.push_back(tok);
        line.erase(0, pos + del.length());
    }
    tree = create_tree(nodes);
    int search, modify;
    //ignore blank space
    getline(file, line);
    //get num of search threads
    getline(file, line);
    del = ":";
    pos = line.find(del);
    search = stoi(line.substr(pos+1, line.length()-2));
    //get num of modify threads
    getline(file, line);
    pos = line.find(del);
    modify = stoi(line.substr(pos+1, line.length()-2));
    //ignore blank space
    getline(file, line);
    //get commands
    del = " || ";
    vector<string> com;
    vector<int> input;
	string last_line;
    int pos1, pos2;
    while (getline(file, line)) {
        while ((pos = line.find(del)) != string::npos) {
            tok = line.substr(0, pos);
            com.push_back(tok.substr(0,1));
            pos1 = tok.find("(");
            pos2 = tok.find(")");
			int n = stoi(line.substr(pos1+1, pos2));
            input.push_back(n);
            line.erase(0, pos + del.length());
        }
		last_line = line;
    } 
	com.push_back(last_line.substr(0, 1));
    pos1 = last_line.find("(");
    pos2 = last_line.find(")");
	int n = stoi(last_line.substr(pos1+1, pos2));
    input.push_back(n);

    //create the threads
    tree->create_threads(search, modify, com, input);
    write_file(tree);
    return 0;
}

/**
 * write_file prints the specified output to an output file "out.txt". It prints the execution time, the results of the
 * function calls and which thread ran them, as well as the final red black tree.
 * */
void write_file(RBtree *t) {
    ofstream out("out.txt");
	complete = chrono::high_resolution_clock::now();
	double time_span = chrono::duration_cast<chrono::microseconds>(complete - start).count();
    out << "Execution time: " << to_string(time_span) << " ms" << endl;
    out << endl;
    for (int i=0; i<thread_results.size(); i++) {
        out << thread_results[i] << endl;
    }
    out << endl;
    t->preorder_print(t->root, out);
	out << endl;
}

int main(int argc, char **argv) {
    if (argc == 1) {
        printf("Please enter a filename\n");
        return 1;
    }
    start = chrono::high_resolution_clock::now();
    char *filename = argv[1];
    if (read_file(filename)) {
        printf("Error Reading File\n");
    }
    return 0;
}
