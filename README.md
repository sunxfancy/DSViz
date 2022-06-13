# DSViz

DSViz is a header-only C++11 library for visualization node based data sturcture for generating GraphViz dot files. 

You can inherit the interface and implement the dsviz_show method. 

```c++
virtual void dsviz_show(DSViz::IViz& viz) {
    DSViz::TableNode node(viz, 2);
    viz.setName(this, node.name);
    node.addPointer("parent", parent, "", "", "", "[constraint=false]");
    node.add("name", name);
    node.add("sum", sum);
    node.addEdge(left,  "left");
    node.addEdge(right, "right");
}
```

`TableNode` is the class used for printing table node in GraphViz. 
- `addPointer` can set pointer which is another node
- `add` can add a new field
- `addEdge` links nodes 

![](./a.png)

