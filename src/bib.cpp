#include <bits/stdc++.h>
#include "graph.hpp"
#include "branch.hpp"
#include "highs.hpp"
#include "util/timer.hpp"
using namespace std;

//  ./bib <graph_type> <graph_path> <algorithm>
int main(int argc, char** argv) {
    ifstream ifs(argv[2]);
    ugraph g = read_graph(ifs, argv[1]);
    timer t;
    mds_solution_t sol;
    size_t nodes;
    if (argc >= 4 && argv[3] == "biblp"s) {
        mds_branch_t<flg_red | flg_lplb, mds_highs_lb_t, mds_reduce_t> solver(g);
        solver.solve();
        sol = solver.best;
        nodes = solver.nodes;
    }
    else if (argc >= 4 && argv[3] == "bibco"s) {
        mds_branch_t<flg_red | flg_ddlb | flg_mcslb, mds_highs_lb_t, mds_reduce_t> solver(g);
        solver.solve();
        sol = solver.best;
        nodes = solver.nodes;
    }
    else if (argc >= 4 && argv[3] == "biblp-if"s) {
        mds_branch_t<flg_red | flg_lplb, mds_highs_lb_t, ijcai20_inf> solver(g);
        solver.solve();
        sol = solver.best;
        nodes = solver.nodes;
    }
    
    else if (argc >= 4 && argv[3] == "bibco-if"s) {
        mds_branch_t<flg_red | flg_ddlb | flg_mcslb, mds_highs_lb_t, ijcai20_inf> solver(g);
        solver.solve();
        sol = solver.best;
        nodes = solver.nodes;
    }
    else if (argc >= 4 && argv[3] == "ddlb"s) {
        mds_branch_t<flg_red | flg_ddlb, mds_highs_lb_t, mds_reduce_t> solver(g);
        solver.solve();
        sol = solver.best;
        nodes = solver.nodes;
    }
    else if (argc >= 4 && argv[3] == "mcslb"s) {
        mds_branch_t<flg_red | flg_mcslb, mds_highs_lb_t, mds_reduce_t> solver(g);
        solver.solve();
        sol = solver.best;
        nodes = solver.nodes;
    }
    cout << t.count() << "," << sol.size() << "," << nodes;
    return 0;
}