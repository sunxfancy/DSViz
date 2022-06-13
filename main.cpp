
#include "dsv.hpp"
#include <iostream>
using std::string;

struct Node : public DSViz::IDataStructure {
    string name;
    float sum;

    Node* parent;
    Node* left;
    Node* right;

    Node(string name, float sum, Node* left, Node* right) 
        : name(name), sum(sum), parent(nullptr), left(left), right(right) {
        if (left)  left->parent  = this;
        if (right) right->parent = this;
    } 

    virtual void dsviz_show(DSViz::IViz& viz) {
        DSViz::TableNode node(viz, 2);
        viz.setName(this, node.name);
        node.addPointer("parent", parent, "", "", "", "[constraint=false]");
        node.add("name", name);
        node.add("sum", sum);
        node.addEdge(left,  "left");
        node.addEdge(right, "right");
    }
};

int main() {
    Node A {string("A"), 10.0f, nullptr, nullptr};
    Node B {string("B"), 12.0f, nullptr, nullptr};
    Node hello {string("hello"), 11.5f, &A, &B};

    DSViz::Dot dot;
    dot.load_ds(&hello);
    std::cout << dot.print();

    return 0;
}