#include "../graph.hpp"

void print_graph(const ugraph& g, ostream& os) {
    for (ui u = 0; u < g.n; ++u)
        for (ui v : g[u])
            if (v > u)
                os << u << ' ' << v << endl;
}

int main(void) {
    for (int i = 111; i <= 145; i += 2) {
        string istr = to_string(i);
        ifstream ifs("BD3/Grafo" + istr + ".txt");
        ofstream ofs("BD3/Grafo" + istr + ".edges");
        print_graph(read_graph_matrix(ifs), ofs);
    }

    for (int i = 450; i <= 500; i++) {
        string istr = to_string(i);
        ifstream ifs("BD6/Grafo" + istr + ".txt");
        ofstream ofs("BD6/Grafo" + istr + ".edges");
        print_graph(read_graph_matrix(ifs), ofs);
    }
}