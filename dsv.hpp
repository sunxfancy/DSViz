#pragma once
#include <string>
#include <ostream>
#include <sstream>
#include <map>
#include <vector>

namespace DSViz
{
    
class IViz;
class Node;

class IDataStructure {
public:
    virtual std::string dsviz_show(IViz& viz) = 0;
    
};


//template <class T>
//struct dsviz_show {
//    std::string operator()(T*, IViz&);
//}; class DSVizShow = dsviz_show<T>

class IViz {
public:
    virtual std::string print() const = 0;

    virtual std::string load_ds(IDataStructure* ds) {
        if (!hasNode(ds)) {
            names[ds] = "";
            return names[ds] = ds->dsviz_show(*this);
        } else
            return names[ds];
    }

    template<class T >
    std::string load_ds_c(T* ds) {
        if (!hasNode(ds)) {
            names[ds] = "";
            return names[ds] = dsviz_show(ds, *this);
        } else 
            return names[ds];
    }


    virtual void addEdge(std::string from, void* to, std::string edge) {
        edges[std::make_pair(from, to)] = edge;
    }

    
    virtual void addNode(std::string name, std::string node) {
        nodes[name] = node;
    }

    virtual bool hasNode(void* ds) {
        return (names.find(ds) != names.end());
    }


    virtual std::string genNodeName() {
        return "_node" + std::to_string(count0++);
    }
    virtual std::string genEdgeName() {
        return "_edge" + std::to_string(count1++);
    }
    virtual std::string genPortName() {
        return "_port" + std::to_string(count2++);
    }



    static std::string encode(std::string data) {
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
            if (c == '=' || c == '?' || c == ':' || c == '&' || c == '^' || c == '~' || c == '*' || c == '%' ||
                c == '/' || c == '(' || c == ')' || c == ';' || c == '[' || c == ']' || c == '{' || c == '}') {
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


protected:
    int count0 = 0, count1 = 0, count2 = 0;
    std::map<std::string, std::string> nodes;
    std::map<void*, std::string> names;
    std::map<std::pair<std::string, void*>, std::string> edges;
    
    friend class Node;
};

class Node {
public:
    Node(IViz& viz, std::string name = "", std::string shape = "") : viz(viz), shape(shape) {
        if (name == "") this->name = viz.genNodeName();
        else this->name = name;
        ss << "[";
    }
    virtual ~Node() {}

    virtual void Done() {
        this->genShape();
        this->genLabel();
        ss << "]";
        viz.addNode(this->name, ss.str()); 
    }

    virtual void genAttr(std::string name, const std::string& attr) {
        if (!attr.empty()) ss << " " << name << "=\"" << attr << "\""; 
    }
    
    virtual void genLabel() { genAttr("label", label); }
    virtual void genShape() { genAttr("shape", shape); }

    std::string name;
    std::string label;
    std::string shape;

protected:
    std::stringstream ss;
    IViz& viz;

};


class TableNode : public Node {
public:
    TableNode(IViz& viz, int span = 1, std::string name = "", std::string shape = "") : Node(viz, name, shape), span(span) {
        tss << "<table border='0' cellborder='1' cellspacing='0' cellpadding='2'>";
    }
    virtual ~TableNode() { 
        tss << "</table>";
        label = tss.str();
        Done();
    }

    inline void attr_name(std::string name, std::string attr) {
        tss << "<td " << IViz::encode(attr) << ">" << IViz::encode(name) << "</td>";
    }

    inline void attr_value(std::string value, std::string attr, std::string pt_name = "") {
        tss << "<td";
        if (span != 1) tss << " colspan='" << span << "'";
        if (!pt_name.empty()) tss << " PORT='" << pt_name << "'";
        tss << " " << IViz::encode(attr) << ">" << IViz::encode(value) << "</td>";
    }

    inline void attr_value_nospan(std::string value, std::string attr, std::string pt_name = "") {
        tss << "<td";
        if (!pt_name.empty()) tss << " PORT='" << pt_name << "'";
        tss << " " << IViz::encode(attr) << ">" << IViz::encode(value) << "</td>";
    }

    template<class T>
    inline void add(std::string name, T number, std::string attr = "", std::string attr2 = "") {
        tss << "<tr>";
        attr_name(name, attr);
        attr_value(std::to_string(number), attr2.empty() ? attr : attr2);
        tss << "</tr>";
    }

    
    inline void addPointer(std::string name, IDataStructure* ds, std::string content = "", std::string attr = "", std::string attr2 = "") {
        if (ds == nullptr) return;
        std::string pt_name = viz.genPortName();

        tss << "<tr>";
        attr_name(name, attr);
        attr_value(content, attr2.empty() ? attr : attr2, pt_name);
        tss << "</tr>";
        if (ds != nullptr) {
            viz.load_ds(ds);
            viz.addEdge(this->name + ":" + pt_name, ds, "");
        }
    }

    inline void addLeftRightSubTree(std::string name, IDataStructure* left, IDataStructure* right, std::string content_left = "", std::string content_right = "", std::string attr = "", std::string attr2 = "") {
        if (left == nullptr && right == nullptr) return;
        std::string pt_name_l = viz.genPortName();
        std::string pt_name_r = viz.genPortName();
        tss << "<tr>";
        attr_name(name, attr);
        attr_value_nospan(content_left, attr2.empty() ? attr : attr2, pt_name_l);
        attr_value_nospan(content_right, attr2.empty() ? attr : attr2, pt_name_r);
        tss << "</tr>";
        if (left != nullptr) {
            viz.load_ds(left);
            viz.addEdge(this->name + ":" + pt_name_l, left, "");
        }
        if (right != nullptr) {
            viz.load_ds(right);
            viz.addEdge(this->name + ":" + pt_name_r, right, "");
        }
    }

    inline void addChildren(std::string name, IDataStructure** children, size_t size, std::vector<std::string> content = std::vector<std::string>(0), std::string attr = "", std::string attr2 = "") {
        tss << "<tr>";
        attr_name(name, attr);
        for (size_t i = 0; i < size; ++i) {
            std::string pt_name = viz.genPortName();
            attr_value_nospan(content[i], attr2.empty() ? attr : attr2, pt_name);

            if (children[i] != nullptr) {
                viz.load_ds(children[i]);
                viz.addEdge(this->name + ":" + pt_name, children[i], "");
            }
        }
        tss << "</tr>";
    }

    template<class T>
    inline void addPointerC(std::string name, T* ds, std::string content = "", std::string attr = "", std::string attr2 = "") {
        if (ds == nullptr) return;
        std::string pt_name = viz.genPortName();

        tss << "<tr>";
        attr_name(name, attr);
        attr_value(content, attr2.empty() ? attr : attr2, pt_name);
        tss << "</tr>";
        if (ds != nullptr) {
            viz.load_ds_c(ds);
            viz.addEdge(this->name + ":" + pt_name, ds, "");
        }
    }


    template<class T>
    inline void addChildrenC(std::string name, T** children, size_t size, std::vector<std::string> content = std::vector<std::string>(0), std::string attr = "", std::string attr2 = "") {
        tss << "<tr>";
        attr_name(name, attr);
        for (size_t i = 0; i < size; ++i) {
            std::string pt_name = viz.genPortName();
            attr_value_nospan(i < content.size() ? content[i]:" ", attr2.empty() ? attr : attr2, pt_name);

            if (children[i] != nullptr) {
                viz.load_ds_c(children[i]);
                viz.addEdge(this->name + ":" + pt_name, children[i], "");
            }
        }
        tss << "</tr>";
    }


    template<class T>
    inline void addArray(std::string name, T* numbers, size_t size, std::string attr = "", std::string attr2 = "") {
        tss << "<tr>";
        attr_name(name, attr);
        for (size_t i = 0; i < size; ++i) {
            attr_value_nospan(std::to_string(numbers[i]), attr2.empty() ? attr : attr2);
        }
        tss << "</tr>";
    }

    virtual void genArrowAttr(std::string name, const std::string& attr) {
        if (!attr.empty()) ss << " " << name << "=<" << attr << ">"; 
    }

    virtual void genLabel() override { genArrowAttr("label", label); }
    
private:
    int span;
    std::stringstream tss;
};

template< >
inline void TableNode::add<std::string>(std::string name, std::string str, std::string attr, std::string attr2) {
    tss << "<tr>";
    attr_name(name, attr);
    attr_value(str, attr2.empty() ? attr : attr2);
    tss << "</tr>";
}



inline std::ostream& operator<<(std::ostream& out, const IViz& viz) {
    out << viz.print() << std::endl;
    return out;
}



class Dot : public IViz {
public:
    virtual std::string print() const {
        std::stringstream ss;
        ss << "digraph structs {" << std::endl;
        ss << "node [shape=plaintext]" << std::endl;
        for (auto node: nodes) {
            ss << node.first << " " << node.second << ";" << std::endl;
        }
        for (auto edge: edges) {
            auto i = names.find(edge.first.second);
            ss << edge.first.first << " -> " << i->second << " " << edge.second << ";" << std::endl;
        }
        ss << "}" << std::endl;
        return ss.str();
    }
};





} // namespace DSViz
