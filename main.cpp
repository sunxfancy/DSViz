
#include "dsv.hpp"
#include <iostream>
using std::string;

struct Node : public DSViz::IDataStructure {
    string name;
    float sum;
    struct Node* left;
    struct Node* right;

    Node(string name, float sum, Node* left, Node* right) : name(name), sum(sum), left(left), right(right) {} 

    virtual string dsviz_show(DSViz::IViz& viz) {
        DSViz::TableNode node(viz, 2);
        // node.genLabel();
        node.add("sum", sum);
        // node.addPointer("left", left);
        // node.addPointer("right", right);
        node.addLeftRightSubTree("child", left, right);
        return node.name;
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