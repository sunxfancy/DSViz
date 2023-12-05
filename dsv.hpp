/**
    MIT License

    Copyright (c) 2020 西风逍遥游

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to
   deal in the Software without restriction, including without limitation the
   rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
   sell copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
   IN THE SOFTWARE.
*/

#pragma once
#include <cassert>
#include <map>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

namespace DSViz {

class IViz;
class Node;


/**
 * @brief A interface for data structure to print graphviz dot node
 */
class IDataStructure {
  public:
    virtual void dsviz_show(IViz &viz) = 0;
};

/**
 * @brief A mock class which provides a non-invasive way to print graphviz dot
 * @param T The type of data structure you want to mock
 * @param F The function to print the data structure
 */
template <typename T, void (*F)(T*, IViz &)>
class Mock : public IDataStructure {
  public:
    virtual void dsviz_show(IViz &viz) override {
        F(ds, viz);
    }

    /**
     * @brief Get the mock object pointer from the original pointer
     * @param ds The original pointer
     */
    static Mock<T,F>* get(T *ds) {
        auto& c = container();
        if (c.find(ds) == c.end()) {
            c[ds] = new Mock(ds);
        }
        return c[ds]; 
    }
  private:
    static std::map<T*, Mock<T,F>*>& container() {
        static std::map<T*, Mock<T,F>*> inst;
        return inst;
    }

    Mock(T *ds) : ds(ds) {}
    T *ds;
};

/**
 * @brief An abstract interface to print graphviz dot graph
 */
class IViz {
  public:
    /**
     * @brief Print the graphviz dot file
     * @return The graphviz dot file in std::string
     */
    virtual std::string print() const = 0;

    /**
     * @brief Generate a unique port name
     * @return The port name `_portN`
     */
    virtual std::string genPortName() = 0;

    /**
     * @brief Generate a unique edge name
     * @return The edge name `_edgeN`
     */
    virtual std::string genEdgeName() = 0;

    /**
     * @brief Generate a unique node name
     * @return The node name `_nodeN`
     */
    virtual std::string genNodeName() = 0;


    /**
     * @brief Add an edge to the graph
     * @param from The name of the source node
     * @param to The name of the target node
     * @param edge The edge label
     */
    virtual void addEdge(std::string from, std::string to,
                         std::string edge = "") = 0;

    /**
     * @brief Add an edge to the graph
     * @param from The name of the source node
     * @param to The name of the target node
     * @param edge The edge label
     */
    virtual void addEdge(std::string from, void *to, std::string edge = "") {
        addEdge(from, getName(to), edge);
    }

    /**
     * @brief Add an edge to the graph
     * @param from The name of the source node
     * @param to The name of the target node
     * @param edge The edge label
     */
    virtual void addEdge(void *from, std::string to, std::string edge = "") {
        addEdge(getName(from), to, edge);
    }

    /**
     * @brief Add an edge to the graph
     * @param from The name of the source node
     * @param to The name of the target node
     * @param edge The edge label
     */
    virtual void addEdge(void *from, void *to, std::string edge = "") {
        addEdge(getName(from), getName(to), edge);
    }

    /**
     * @brief Check if a pointer is already mapped to a node in dot file
     */
    virtual bool hasNode(void *ds) const                     = 0;

    /**
     * @brief Add a node to the graph
     * @param name The name of the node
     * @param node The node content
     */
    virtual void addNode(std::string name, std::string node) = 0;


    /**
     * @brief Add a subgraph to the graph
     * @param name The content of the subgraph in graphviz dot format
     */
    virtual void addSubGraph(std::string sg) = 0;

    /**
     * @brief Set the name of a node
     * @param ds The pointer to the node
     * @param name The name of the node
     */
    virtual void        setName(void *ds, std::string name) = 0;

    /**
     * @brief Get the name of a node
     * @param ds The pointer to the node
     * @return The name of the node
     */
    virtual std::string getName(void *ds) const             = 0;

    /**
     * @brief Get the pointer from the name
     * @param name The name of the pointer
     * @return The pointer
     */
    virtual void       *getDS(std::string name) const       = 0;

    /**
     * @brief Load a data structure to the graph
     * @details This function will start a depth-first search from the root node.
     *          Then all the nodes are able to reached will be added to the graph.
     * @param ds The pointer to the data structure
     */
    virtual void load_ds(IDataStructure *ds) {
        if (!hasNode(ds)) {
            ds->dsviz_show(*this);
        }
    }

    /**
     * @brief Load a data structure to the graph
     * @details This function will start a depth-first search from the root node.
     *          Then all the nodes are able to reached will be added to the graph.
     * @param ds The pointer to the data structure
     */
    template <class T> void load_ds_c(T *ds) {
        if (!hasNode(ds)) {
            dsviz_show(ds, *this);
        }
    }

    inline static std::string encode(std::string data) {
        std::string ans;
        for (auto c : data) {
            if (c == '<') {
                ans += "&lt;";
                continue;
            }
            if (c == '>') {
                ans += "&gt;";
                continue;
            }
            if (c == '=' || c == '?' || c == ':' || c == '&' || c == '^' ||
                c == '~' || c == '*' || c == '%' || c == '/' || c == '(' ||
                c == ')' || c == ';' || c == '[' || c == ']' || c == '{' ||
                c == '}') {
                ans += "&#";
                int ascii = (int)c;
                ans += std::to_string(ascii);
                ans += ';';
                continue;
            }
            ans += c;
        }
        return ans;
    }
};

/**
 * @brief A class representing a node in graphviz dot file
 */
class Node {
  public:
    Node(IViz &viz, std::string name = "", std::string shape = "",
         std::string style = "")
        : viz(viz), shape(shape), style(style) {
        if (name == "")
            this->name = viz.genNodeName();
        else
            this->name = name;
        ss << "[";
    }
    virtual ~Node() { Done(); }

    virtual void Done() {
        if (isDone) return;
        this->genShape();
        this->genLabel();
        this->genStyle();
        for (auto &p : other_attrs)
            genAttr(p.first, p.second);
        ss << "]";
        viz.addNode(this->name, ss.str());
        isDone = true;
    }

    virtual void addEdge(IDataStructure *ds, std::string edge_label = "",
                         std::string edge = "") {
        if (ds != nullptr) {
            viz.load_ds(ds);
            if (edge_label != "") edge += " label=\"" + edge_label + "\"";
            viz.addEdge(this->name, ds, "[" + edge + "]");
        }
    }

    virtual void addAttr(std::string attr, std::string value) {
        other_attrs[attr] = value;
    }

    std::string name, label, shape, style;

  protected:
    virtual void genAttr(std::string name, const std::string &attr) {
        if (!attr.empty()) ss << " " << name << "=\"" << attr << "\"";
    }

    virtual void genLabel() { genAttr("label", label); }
    virtual void genShape() { genAttr("shape", shape); }
    virtual void genStyle() { genAttr("style", style); }

    IViz &viz;
    bool  isDone = false;

    std::stringstream                  ss;
    std::map<std::string, std::string> other_attrs;
};

/**
 * @brief A class representing a table node in graphviz dot file
 *        such as:
 *          node [lable=< <table border='0' cellborder='1' cellspacing='0'>...</table> >]
 */
class TableNode : public Node {
  public:
    TableNode(IViz &viz, int span = 1, std::string name = "",
              std::string shape = "", std::string style = "")
        : Node(viz, name, shape, style), span(span) {
        tss << "<table border='0' cellborder='1' cellspacing='0' "
               "cellpadding='2'>";
    }
    virtual ~TableNode() {
        tss << "</table>";
        label = tss.str();
        Done();
    }

    inline void attr_name(std::string name, std::string attr) {
        tss << "<td " << IViz::encode(attr) << ">" << IViz::encode(name)
            << "</td>";
    }

    inline void attr_value(std::string value, std::string attr,
                           std::string pt_name = "") {
        tss << "<td";
        if (span != 1) tss << " colspan='" << span << "'";
        if (!pt_name.empty()) tss << " PORT='" << pt_name << "'";
        tss << " " << IViz::encode(attr) << ">" << IViz::encode(value)
            << "</td>";
    }

    inline void attr_value_nospan(std::string value, std::string attr,
                                  std::string pt_name = "") {
        tss << "<td";
        if (!pt_name.empty()) tss << " PORT='" << pt_name << "'";
        tss << " " << IViz::encode(attr) << ">" << IViz::encode(value)
            << "</td>";
    }

    template <typename T>
    inline void add(std::string name, T number, std::string attr = "",
                    std::string attr2 = "") {
        tss << "<tr>";
        attr_name(name, attr);
        attr_value(std::to_string(number), attr2.empty() ? attr : attr2);
        tss << "</tr>";
    }

    inline void addPointer(std::string name, IDataStructure *ds,
                           std::string content = "", std::string attr = "",
                           std::string attr2 = "", std::string edge = "") {
        if (ds == nullptr) return;
        std::string pt_name = viz.genPortName();

        tss << "<tr>";
        attr_name(name, attr);
        attr_value(content, attr2.empty() ? attr : attr2, pt_name);
        tss << "</tr>";
        if (ds != nullptr) {
            viz.load_ds(ds);
            viz.addEdge(this->name + ":" + pt_name, ds, edge);
        }
    }

    inline void addLeftRightSubTree(std::string name, IDataStructure *left,
                                    IDataStructure *right,
                                    std::string     content_left  = "",
                                    std::string     content_right = "",
                                    std::string     attr          = "",
                                    std::string     attr2         = "") {
        if (left == nullptr && right == nullptr) return;
        std::string pt_name_l = viz.genPortName();
        std::string pt_name_r = viz.genPortName();
        tss << "<tr>";
        attr_name(name, attr);
        attr_value_nospan(content_left, attr2.empty() ? attr : attr2,
                          pt_name_l);
        attr_value_nospan(content_right, attr2.empty() ? attr : attr2,
                          pt_name_r);
        tss << "</tr>";
        if (left != nullptr) {
            viz.load_ds(left);
            viz.addEdge(this->name + ":" + pt_name_l, left);
        }
        if (right != nullptr) {
            viz.load_ds(right);
            viz.addEdge(this->name + ":" + pt_name_r, right);
        }
    }

    inline void
    addChildren(std::string name, IDataStructure **children, size_t size,
                std::vector<std::string> content = std::vector<std::string>(0),
                std::string attr = "", std::string attr2 = "") {
        tss << "<tr>";
        attr_name(name, attr);
        for (size_t i = 0; i < size; ++i) {
            std::string pt_name   = viz.genPortName();
            std::string content_i = content.size() > i ? content[i] : "";
            attr_value_nospan(content_i, attr2.empty() ? attr : attr2, pt_name);

            if (children[i] != nullptr) {
                viz.load_ds(children[i]);
                viz.addEdge(this->name + ":" + pt_name, children[i]);
            }
        }
        tss << "</tr>";
    }

    template <class T>
    inline void addPointerC(std::string name, T *ds, std::string content = "",
                            std::string attr = "", std::string attr2 = "") {
        if (ds == nullptr) return;
        std::string pt_name = viz.genPortName();

        tss << "<tr>";
        attr_name(name, attr);
        attr_value(content, attr2.empty() ? attr : attr2, pt_name);
        tss << "</tr>";
        if (ds != nullptr) {
            viz.load_ds_c(ds);
            viz.addEdge(this->name + ":" + pt_name, ds);
        }
    }

    template <class T>
    inline void
    addChildrenC(std::string name, T **children, size_t size,
                 std::vector<std::string> content = std::vector<std::string>(0),
                 std::string attr = "", std::string attr2 = "") {
        tss << "<tr>";
        attr_name(name, attr);
        for (size_t i = 0; i < size; ++i) {
            std::string pt_name = viz.genPortName();
            attr_value_nospan(i < content.size() ? content[i] : " ",
                              attr2.empty() ? attr : attr2, pt_name);

            if (children[i] != nullptr) {
                viz.load_ds_c(children[i]);
                viz.addEdge(this->name + ":" + pt_name, children[i]);
            }
        }
        tss << "</tr>";
    }

    template <class T>
    inline void addArray(std::string name, T *numbers, size_t size,
                         std::string attr = "", std::string attr2 = "") {
        tss << "<tr>";
        attr_name(name, attr);
        for (size_t i = 0; i < size; ++i) {
            attr_value_nospan(std::to_string(numbers[i]),
                              attr2.empty() ? attr : attr2);
        }
        tss << "</tr>";
    }

    virtual void genArrowAttr(std::string name, const std::string &attr) {
        if (!attr.empty()) ss << " " << name << "=<" << attr << ">";
    }

    virtual void genLabel() override { genArrowAttr("label", label); }

  private:
    int               span;
    std::stringstream tss;
};

template <>
inline void
TableNode::add<std::string>(std::string name, std::string str, std::string attr,
                            std::string attr2) {
    tss << "<tr>";
    attr_name(name, attr);
    attr_value(str, attr2.empty() ? attr : attr2);
    tss << "</tr>";
}

template <>
inline void
TableNode::add<bool>(std::string name, bool b, std::string attr,
                     std::string attr2) {
    tss << "<tr>";
    attr_name(name, attr);
    attr_value(b ? "true" : "false", attr2.empty() ? attr : attr2);
    tss << "</tr>";
}

template <>
inline void
TableNode::add<const char *>(std::string name, const char *str,
                             std::string attr, std::string attr2) {
    tss << "<tr>";
    attr_name(name, attr);
    attr_value(std::string(str), attr2.empty() ? attr : attr2);
    tss << "</tr>";
}


/**
 * @brief The configuration of the system
 */
struct Config {
    std::string node_style{"shape=plaintext"};
    std::string edge_style{""};
    std::string graph_style{""};
    std::string other{""};

    std::string genGraphStyle() const {
        std::stringstream ss;
        if (!node_style.empty())
            ss << "node [" << node_style << "];" << std::endl;
        if (!edge_style.empty())
            ss << "edge [" << edge_style << "];" << std::endl;
        if (!graph_style.empty())
            ss << "graph [" << graph_style << "];" << std::endl;
        ss << other << std::endl;
        return ss.str();
    }
};

/**
 * @brief A class representing an edge in graphviz dot file
 */
class Edge {
  public:
    Edge(std::string from, std::string to) : from(from), to(to) {}
    std::string from, to;

    bool operator<(const Edge &rhs) const {
        if (from < rhs.from) return true;
        if (rhs.from < from) return false;
        return to < rhs.to;
    }
};

/**
 * @brief A class representing a subgraph in graphviz dot file
 */
class SubGraph : public IViz {
  public:
    SubGraph(IViz &viz, std::string name = "", std::string label = "",
             Config config = {})
        : viz(viz) {
        ss << "subgraph cluster_" << name << " {" << std::endl;
        if (!label.empty()) ss << "label = \"" << label << "\";" << std::endl;
        ss << config.genGraphStyle() << std::endl;
    }
    virtual ~SubGraph() {
        for (auto subgraph : subgraphs) {
            ss << subgraph << std::endl;
        }
        for (auto node : nodes) {
            ss << node.first << " " << node.second << ";" << std::endl;
        }
        for (auto edge : edges) {
            ss << edge.first.from << " -> " << edge.first.to << " "
               << edge.second << ";" << std::endl;
        }
        ss << "}" << std::endl;
        viz.addSubGraph(print());
    }

    virtual std::string print() const override { return ss.str(); }

    virtual void setName(void *ds, std::string name) override {
        viz.setName(ds, name);
    }
    virtual std::string getName(void *ds) const override {
        return viz.getName(ds);
    }
    virtual void *getDS(std::string name) const override {
        return viz.getDS(name);
    }

    using IViz::addEdge;
    virtual void addEdge(std::string from, std::string to,
                         std::string edge = "") override {
        assert(!from.empty());
        assert(!to.empty());
        edges[Edge(from, to)] = edge;
    }
    virtual void addNode(std::string name, std::string node) override {
        assert(!name.empty());
        nodes[name] = node;
    }

    virtual void addSubGraph(std::string subgraph) override {
        subgraphs.push_back(subgraph);
    }
    virtual bool hasNode(void *ds) const override { return viz.hasNode(ds); }

    virtual std::string genNodeName() override { return viz.genNodeName(); }
    virtual std::string genEdgeName() override { return viz.genEdgeName(); }
    virtual std::string genPortName() override { return viz.genPortName(); }

  private:
    IViz             &viz;
    std::stringstream ss;

    std::map<std::string, std::string> nodes;
    std::map<Edge, std::string>        edges;
    std::vector<std::string>           subgraphs;
};

inline std::ostream &
operator<<(std::ostream &out, const IViz &viz) {
    out << viz.print() << std::endl;
    return out;
}

/**
 * @brief A class representing the whole dot file in graphviz
 */
class Dot : public IViz {
  public:
    Dot(Config config = {}) : config(config) {}

    virtual std::string print() const override {
        std::stringstream ss;
        ss << "digraph structs {" << std::endl;
        ss << config.genGraphStyle() << std::endl;

        for (auto subgraph : subgraphs) {
            ss << subgraph << std::endl;
        }
        for (auto node : nodes) {
            ss << node.first << " " << node.second << ";" << std::endl;
        }
        for (auto edge : edges) {
            ss << edge.first.from << " -> " << edge.first.to << " "
               << edge.second << ";" << std::endl;
        }
        ss << "}" << std::endl;
        return ss.str();
    }

    virtual void setName(void *ds, std::string name) override {
        names[ds] = name;
        if (!name.empty()) DSs[name] = ds;
    }

    virtual std::string getName(void *ds) const override {
        assert(hasNode(ds));
        return names.find(ds)->second;
    }

    virtual void *getDS(std::string name) const override {
        assert(!name.empty());
        return DSs.find(name)->second;
    }

    using IViz::addEdge;

    virtual void addEdge(std::string from, std::string to,
                         std::string edge = "") override {
        assert(!from.empty());
        assert(!to.empty());
        edges[Edge(from, to)] = edge;
    }

    virtual void addNode(std::string name, std::string node) override {
        assert(!name.empty());
        nodes[name] = node;
    }

    virtual void addSubGraph(std::string subgraph) override {
        subgraphs.push_back(subgraph);
    }

    virtual bool hasNode(void *ds) const override {
        assert(ds);
        return (names.find(ds) != names.end());
    }

    virtual std::string genNodeName() override {
        return "_node" + std::to_string(count0++);
    }
    virtual std::string genEdgeName() override {
        return "_edge" + std::to_string(count1++);
    }
    virtual std::string genPortName() override {
        return "_port" + std::to_string(count2++);
    }

  protected:
    int count0 = 0, count1 = 0, count2 = 0;

    std::map<std::string, std::string> nodes;
    std::map<void *, std::string>      names;
    std::map<std::string, void *>      DSs;
    std::map<Edge, std::string>        edges;
    std::vector<std::string>           subgraphs;
    Config                             config;
    friend class Node;
};


} // namespace DSViz
