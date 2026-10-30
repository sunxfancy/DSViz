#include "dsv.hpp"
#include "check.hpp"
#include <iostream>
#include <functional>
#include <map>
struct Leaf : DSViz::IDataStructure {
    int visits = 0;
    void dsviz_show(DSViz::IViz &v) override {
        ++visits; DSViz::TableNode n(v); v.setName(this, n.name); n.add("value", 7);
    }
};
struct Plain { int value; };
void dsviz_show(Plain *p, DSViz::IViz &v) {
    DSViz::TableNode n(v); v.setName(p,n.name); n.add("value",p->value);
}
void invalid() {
    DSViz::Dot d; int a;
    checkThrows<std::invalid_argument>([&]{ d.setName(nullptr,"x"); });
    checkThrows<std::invalid_argument>([&]{ d.setName(&a,""); });
    checkThrows<std::invalid_argument>([&]{ d.addNode("", "[]"); });
    checkThrows<std::invalid_argument>([&]{ d.addEdge(std::string(""),std::string("x")); });
    checkThrows<std::invalid_argument>([&]{ d.addEdge(std::string("x"),std::string("")); });
    checkThrows<std::out_of_range>([&]{ d.getName(&a); });
    CHECK(d.getDS("") == nullptr);
    { DSViz::SubGraph s(d,"invalid");
      checkThrows<std::invalid_argument>([&]{ s.addNode("", "[]"); });
      checkThrows<std::invalid_argument>([&]{ s.addEdge(std::string(""),std::string("x")); }); }
    std::cout << d.print();
}
void fields() {
    DSViz::Dot d;
    { DSViz::TableNode n(d,2);
      n.add("integer", -42); n.add("double",1.25); n.add("bool",true);
      n.add("false",false); n.add("string",std::string("<&> 中文")); n.add("literal","hello");
      int values[]={1,2,3}; n.addArray("array",values,3,"bgcolor='yellow'");
      n.addArray<int>("empty",nullptr,0);
      n.addAttr("color","red"); }
    auto text=d.print(); CHECK(text.find("-42")!=std::string::npos);
    CHECK(text.find("true")!=std::string::npos); CHECK(text.find("false")!=std::string::npos);
    CHECK(text.find("&lt;")!=std::string::npos); std::cout << text;
}
void children() {
    DSViz::Dot d; Leaf a,b; DSViz::IDataStructure *list[]={&a,nullptr,&b,&a};
    { DSViz::TableNode n(d,2);
      n.addPointer("null",nullptr); n.addPointer("pointer",&a,"A","","","[color=red]");
      n.addLeftRightSubTree("both",&a,&b); n.addLeftRightSubTree("left",&a,nullptr);
      n.addLeftRightSubTree("right",nullptr,&b); n.addLeftRightSubTree("empty",nullptr,nullptr);
      n.addChildren("children",list,4,{"one"});
      n.addChildren("empty",nullptr,0); }
    CHECK(a.visits==1 && b.visits==1); std::cout<<d.print();
}
void plain() {
    DSViz::Dot d; Plain a{1},b{2}; Plain *items[]={&a,nullptr,&b};
    { DSViz::TableNode n(d);
      n.addPointerC("pointer",&a,"","","","[color=blue]");
      n.addPointerC<Plain>("null",nullptr);
      n.addChildrenC("items",items,3,{"A"}); n.addChildrenC<Plain>("empty",nullptr,0); }
    CHECK(d.hasNode(&a) && d.hasNode(&b)); CHECK(d.print().find("color=blue")!=std::string::npos);
    std::cout<<d.print();
}
void config() {
    DSViz::Config c; c.graph_style="rankdir=LR"; c.edge_style="color=blue";
    c.other="// custom graph options"; DSViz::Dot d(c);
    { DSViz::Node n(d,"named","box","rounded"); n.label="hello"; }
    CHECK(d.print().find("rankdir=LR")!=std::string::npos);
    d.reset(); CHECK(d.print().find("named")==std::string::npos);
    CHECK(d.print().find("rankdir=LR")!=std::string::npos);
    CHECK(d.genNodeName()=="_node0"); CHECK(d.genPortName()=="_port0"); CHECK(d.genEdgeName()=="_edge0");
    std::ostringstream out; out<<d; CHECK(out.str()==d.print()+"\n"); std::cout<<d.print();
}
void subgraphs() {
    DSViz::Dot d; int a,b;
    { DSViz::SubGraph outer(d,"outer","a \"quoted\" label");
      { DSViz::SubGraph inner(outer,"inner");
        { DSViz::Node n(inner,"a"); n.label="A"; inner.setName(&a,n.name); }
        CHECK(inner.hasNode(&a)); CHECK(inner.getDS("a")==&a); CHECK(inner.getName(&a)=="a");
        CHECK(inner.print().find("label=\"A\"")!=std::string::npos); }
      { DSViz::Node n(outer,"b"); n.label="B"; outer.setName(&b,n.name); }
      outer.addEdge(static_cast<void*>(&a),static_cast<void*>(&b));
      outer.addEdge(std::string("a"),static_cast<void*>(&b),"[color=red]");
      outer.addEdge(static_cast<void*>(&a),std::string("b"),"[color=blue]"); }
    CHECK(d.print().find("cluster_inner")!=std::string::npos); std::cout<<d.print();
}
void stress() {
    DSViz::Dot d;
    for(int i=0;i<1000;++i) { DSViz::Node n(d); n.label="node "+std::to_string(i); }
    auto first=d.print(); CHECK(first==d.print()); CHECK(first.find("node 999")!=std::string::npos);
    std::cout<<first;
}
int main(int argc,char **argv) {
    const std::map<std::string,std::function<void()>> cases={{"invalid",invalid},{"fields",fields},
        {"children",children},{"plain",plain},{"config",config},{"subgraphs",subgraphs},{"stress",stress}};
    try { CHECK(argc==2); cases.at(argv[1])(); }
    catch(const std::exception &e) { std::cerr<<e.what()<<'\n'; return 1; }
}
