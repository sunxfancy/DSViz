#include "dsv.hpp"
#include "check.hpp"
#include <fstream>
#include <type_traits>

struct Item { int value; };
void show(Item *, DSViz::IViz &);
using Mock = DSViz::Mock<Item, show>;
void show(Item *p, DSViz::IViz &v) {
    DSViz::TableNode node(v);
    v.setName(Mock::get(p), node.name);
    node.add("value", p->value);
}
void dsviz_show(Item *p, DSViz::IViz &v) {
    DSViz::TableNode node(v); v.setName(p, node.name); node.add("value", p->value);
}
struct Cycle : DSViz::IDataStructure {
    Cycle *next = nullptr;
    void dsviz_show(DSViz::IViz &v) override {
        DSViz::TableNode node(v);
        v.setName(this, node.name);
        node.add("text", "<hello> & world");
        node.addEdge(next, "say \"hello\" \\ end\nnext");
    }
};
int main(int argc, char **argv) {
    CHECK(argc == 2);
    static_assert(std::has_virtual_destructor<DSViz::IViz>::value, "virtual destructor");
    static_assert(std::has_virtual_destructor<DSViz::IDataStructure>::value, "virtual destructor");
    DSViz::Dot d;
    d.load_ds(nullptr);
    d.load_ds(Mock::get(nullptr));
    d.load_ds_c(static_cast<Item *>(nullptr));
    CHECK(!d.hasNode(nullptr));
    CHECK(d.getDS("missing") == nullptr);
    bool threw = false;
    try { d.getName(nullptr); } catch (const std::out_of_range &) { threw = true; }
    CHECK(threw);
    int a, b;
    d.setName(&a, "old"); d.setName(&a, "new");
    CHECK(d.getDS("old") == nullptr && d.getDS("new") == &a);
    threw = false;
    try { d.setName(&b, "new"); } catch (const std::invalid_argument &) { threw = true; }
    CHECK(threw && !d.hasNode(&b));
    d.clear();
    CHECK(!d.hasNode(&a));
    {
        DSViz::TableNode n(d);
        n.add("value", 42, "bgcolor='red'");
        n.Done(); n.Done();
        CHECK(d.print().find("42") != std::string::npos);
    }
    {
        DSViz::Node n(d);
        n.label = "say \"hello\" \\ literal\nnext";
    }
    d.addNode("a", "[]"); d.addNode("b", "[]");
    d.addEdge(std::string("a"), std::string("b"), "[label=left]");
    d.addEdge(std::string("a"), std::string("b"), "[label=right]");
    CHECK(d.print().find("label=left") != std::string::npos);
    CHECK(d.print().find("label=right") != std::string::npos);
    Cycle c; c.next = &c; d.load_ds(&c);
    Item item{7}; d.load_ds(Mock::get(&item));
    {
        DSViz::SubGraph sub(d, "example", "quoted \"label\"");
        { DSViz::Node n(sub); n.label = "subgraph"; }
        CHECK(sub.print().find("subgraph\"") != std::string::npos);
        CHECK(sub.print() == sub.print());
        sub.addEdge(std::string("a"), std::string("b"), "[label=subleft]");
        sub.addEdge(std::string("a"), std::string("b"), "[label=subright]");
    }
    std::ostringstream out; d.write(out); CHECK(out.str() == d.print());
    std::ofstream file(argv[1]); d.write(file);
    d.reset(); CHECK(d.print().find("42") == std::string::npos);
    Mock::clearCache();
}
