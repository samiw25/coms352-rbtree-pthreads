/**
 * rbtree.h defines two classes: Node and RBtree. It defines teh functions and variables wihtin these two classes
 * as well as some functions needed for parsing and printing to files 
 * Author: Samantha Williams
 * November 12, 2019
 **/
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <queue>

#ifndef RBTREE_H_
#define RBTREE_H_

using namespace std;
/**
 * The Node class represents a red black tree node object.
 * */
class Node {
    public:
    //the integer key value of the node.
    int key;
    //the node's parent node.
    Node *parent;
    //the node's left child node.
    Node *left;
    //the node's right child node.
    Node *right;
    //the color of the node; true = red, false = black.
    bool color;
    //constructor for the Node class
    Node() {
        parent = NULL;
        left = NULL;
        right = NULL;
        color = false;
    }
};

/**
 * The RBtree class defines the functions available for searching and modifying the tree.
 * It also has a pointer to the root node of the tree so that the user can access other nodes.
 * */
class RBtree {
    public:
    /**
     * root is the root of the red black tree.
     * */
    Node *root;
    /**
     * NIL is the sentinel node of the rb tree.
     * */
    Node *nil;
    /**
    * search returns true if the input integer is present in the tree
    * and false if it is not.
    * */

    bool search(int num);
    /**
     * get_minimum is a helper search function that returns the node with the smallest key value that belongs
     * to the subtree starting at node n.
     * */
    Node * get_minimum(Node *n);
    /**
    * delete_node removes the node with the inputted key value from the tree
    * if it exists. It calls helper functions to maintain the red black characteristics.
    * */
    void delete_node(int num);
    /**
    * insert inserts a new node with the given key value into the tree. It calls helper
    * functions to maintain the red black characteristics.
    * */
    void insert(int num);
    /**
     * transplant is a helper method for deletions that changes the relationships of the nodes during deletion.
     * */
    void transplant(Node *n1, Node *n2);
    /**
     * delete_node_fixup is a helper method that ensures the properties of the red black tree are maintained during deletion.
     * */
    void delete_node_fixup(Node *n);
    /**
     * insert_fixup is a helper method that ensures the properties of the red black tree are maintained during insertion.
     * */
    void insert_fixup(Node *n);
    /**
     * left_rotate performs a left rotation on the node n and its children
     * */
    void left_rotate(Node *n);
    /**
     * right_rotate performs a right rotation on the node n and its children
     * */
    void right_rotate(Node *n);
    /**
     * in_order_print is a recursive function to print the tree in order from the input node n
     * */
    void preorder_print(Node *n, ofstream& out);
	/**
	 * create_threads creates the search and modify threads specified by the input integers search_threads and modify_threads
	 * it then parses commands and inputs for the search and modify commands and adds them to their respective queues in order
	 * to be executed by the appropriate threads.
	 * */
    void create_threads(int search_threads, int modify_threads, vector<string> commands, vector<int> inputs);



    /**
     * Constructor for the RBtree class
     * */
    RBtree() {
        root = new Node();
        nil = new Node();
        root = nil;
        root->parent = nil;
    }

};

/**
 * reader is the function that the search threads run in. It takes the thread_id as input and runs the function at the beginning
 * of the search_queue once signaled that it can begin. Search threads have priority over modify threads.
 * */
static void * reader(void *arg);

/**
 * writer is the function that the modify threads run in. It takes the thread_id as input and runs the function at the beginning
 * of the modify_queue once signaled that it can begin
 * */
static void * writer(void *arg);

/**
 * read_file parses the input file for the tree nodes, thread information, and function calls.
 * Returns 1 if an error occurs while opening or reading the file.
 * */
int read_file(char *filename);

void write_file(RBtree *t);

/**
 * create_tree_helper is a recursive function that adds the nodes to the tree in preorder as they're listed.  It takes in a pointer to 
 * the tree, a vector containing the keys in preorder, a vector containing the colors in preorder, and a pointer to the index sharer
 * by the recursive calls of the function. It returns a pointer to the root of the tree.
 * */
Node * create_tree_helper(RBtree *t, vector<int> keys, vector<char> cols, int *index_ptr);

/**
 * create_tree takes in a vector of strings that contains the nodes of the tree as inputted by the input file.
 * After separating the keys and the colors, it calls create_tree_helper to add the nodes to the tree.
 * Returns a pointer to the tree;
 * */
RBtree * create_tree(vector<string> nodes);

#endif