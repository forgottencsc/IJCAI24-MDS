#pragma once
#include "context.hpp"

struct mds_basic_lbs_t : virtual mds_context_t {
    
    mds_basic_lbs_t(const ugraph& g): mds_context_t(g) {}

    // Find a set of undominated vertices whose dominators are parewise disjoint.
    ui lb_disjoint_dominators() {
        vector<ui> us;
        for (ui u = 0; u < this->n; ++u)
            if (!dominated(u))
                us.push_back(u);
        sort(us.begin(), us.end(), [&](ui u, ui v) { return frequency(u) < frequency(v); });
        ui lb = selected();

        vector<char> occ(n, 0);
        for (ui u : us) {
            bool fail = 0;
            for (ui v : g[u]) {
                if (!undetermined(v)) continue;
                if (occ[v]) {
                    fail = 1;
                    break;
                }
            }
            if (!fail) {
                lb++;
                for (ui v : g[u]) {
                    if (!undetermined(v)) continue;
                    occ[v] = 1;
                }
            }
        }
        return lb;
    }

    ui lb_max_coverage_size() {
        vector<ui> cs;
        cs.reserve(undetermined());
        for (ui u = 0; u < n; ++u)
            if (undetermined(u))
                cs.push_back(coverage_size(u));
        sort(cs.begin(), cs.end());
        ui lb = selected();
        ui sum = 0;
        while (!cs.empty() && sum < n - dominated()) {
            sum += cs.back();
            cs.pop_back();
            lb++;
        }
        return lb;
    }
};