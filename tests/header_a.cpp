#include "dsv.hpp"
std::string otherTranslationUnit() {
    DSViz::Dot d;
    { DSViz::TableNode n(d); n.add("text",std::string("header-only")); }
    return d.print();
}
