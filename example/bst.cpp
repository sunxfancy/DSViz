
#include <iostream>
#include <cstring>

using namespace std;

struct bintree_node {
    bintree_node *left;
    bintree_node *right;
    int           data;
};

class bst {
    bintree_node *root;

  public:
    bst() { root = NULL; }
    int  isempty() { return (root == NULL); }
    bintree_node *getRoot() { return root; }
    void insert(int item);
    void displayBinTree();
    void printBinTree(bintree_node *);
};

void
bst::insert(int item) {
    bintree_node *p = new bintree_node;
    bintree_node *parent;
    p->data  = item;
    p->left  = NULL;
    p->right = NULL;
    parent   = NULL;
    if (isempty())
        root = p;
    else {
        bintree_node *ptr;
        ptr = root;
        while (ptr != NULL) {
            parent = ptr;
            if (item > ptr->data)
                ptr = ptr->right;
            else
                ptr = ptr->left;
        }
        if (item < parent->data)
            parent->left = p;
        else
            parent->right = p;
    }
}

void
bst::displayBinTree() {
    printBinTree(root);
}

void
bst::printBinTree(bintree_node *ptr) {
    if (ptr != NULL) {
        printBinTree(ptr->left);
        cout << ptr->data << " ";
        printBinTree(ptr->right);
    }
}

#ifndef DISABLE_DSVIZ
#include "dsv.hpp"
void dsviz_show(bintree_node* P, DSViz::IViz &viz);
typedef DSViz::Mock<bintree_node, dsviz_show> mock;

void dsviz_show(bintree_node* P, DSViz::IViz &viz) {
    DSViz::TableNode node(viz);
    viz.setName(mock::get(P), node.name);
    node.add("data", P->data);
    if (P->left) node.addEdge(mock::get(P->left), "left");
    if (P->right) node.addEdge(mock::get(P->right), "right");
}

const char* _dotToDebugger(bst& b) {
    if (!b.getRoot()) return "";
    
    auto* root = mock::get(b.getRoot());
    DSViz::Dot dot;
    dot.load_ds(root);
    string str = dot.print();

    static char cstr[1024 * 16];
    strcpy(cstr, str.c_str());
    return cstr;
}
#endif

int
main() {
    bst b;
    int array[] = {20, 10, 5, 15, 40, 45, 30};
    for (int i = 0; i < 7; i++) {
        b.insert(array[i]);
    }

    cout << "sorted number: " << endl;
    b.displayBinTree();
    cout << endl;
    return 0;
}