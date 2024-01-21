#pragma once
#include "context.hpp"

bool contains(const vector<ui>& S, const vector<ui>& T) {
    if (S.size() == T.size())
        return S == T;
    else if (T.size() * log(S.size()) < S.size() + T.size()) {
        for (ui u : T)
            if (!binary_search(S.begin(), S.end(), u))
                return false;
        return true;
    }
    else
        return includes(S.begin(), S.end(), T.begin(), T.end());
}

struct mds_reduce_t : virtual mds_context_t {

    vector<vector<ui>> h;

    mds_reduce_t(const ugraph& g) : mds_context_t(g), h(n) {
        for (ui u = 0; u < n; ++u) {
            h[u].assign(g[u].begin(), g[u].end());
            sort(h[u].begin(), h[u].end(), [this](ui v1, ui v2) { return this->g[v1].size() < this->g[v2].size(); });
        }
    }

    //  Check whether there exists an undetermined vertex v other than u in N[w] whose coverage is a superset of u.
    bool check_subset(ui u) {
        if (coverage_size(u) == 0)
            return true;
        assert(undetermined(u) && "u must not be determined");

        //  Find the vertex in N[u]-N[S] with minimum frequency
        ui w = ~0;
        for (ui v : g[u]) 
            if (!dominated(v) && (!~w || frequency(v) < frequency(w)))
                w = v;

        //  Every vertex in vz=N[u]-N[S] should be adjacent to v
        static vector<ui> vz;
        vz.clear();
        for (ui z : h[u])
            if (z != w && !dominated(z))
                vz.push_back(z);

        for (ui v : g[w]) {
            if (v == u || !undetermined(v))
                continue;
            //  Check whether u's coverage is a subset of v's coverage.
            //  i.e., every vertex in N[u]-N[S] is incident to v.
            bool fail = false;
            for (ui z : vz)
                if (!g[v].contains(z)) {
                    fail = true;
                    break;
                }
            if (!fail)
                return true;
        }
        return false;
    }

    //  Reduce when u has only one dominator.
    bool reduce_single_dominator(ui u) {
        assertf(!dominated(u) && "u must not be dominated");
        if (frequency(u) != 1)
            return false;
        for (ui v : g[u]) {
            if (!undetermined(v))
                continue;
            select(v);
            break;
        }
        return true;
    }

    //  Reduce when there exists an undominated vertex v such that v is dominated implies u is dominated.
    bool reduce_ignore(ui u) {
        assertf(!dominated(u) && "u must not be dominated");
        assertf(frequency(u) >= 1 && "solution invalid");
        bool reduced = false;
        ui w = ~0;  //  Let w be a dominator of u that its coverage is minimized.
        for (ui v : g[u]) 
            if (!excluded(v) && (!~w || coverage_size(v) < coverage_size(w)))
                w = v;

        static vector<ui> vz;
        vz.clear();
        for (ui z : h[u])
            if (z != w && !excluded(z))
                vz.push_back(z);

        for (ui v : g[w]) {
            if (v == u || dominated(v) || frequency(v) < frequency(u))
                continue;
            //  v isn't dominated. check whether v is dominated implies u is dominated
            bool fail = false;
            for (ui z : vz)   //  z is a set that contains u. check whether z contains v.
                if (!g[z].contains(v)) {
                    fail = true;
                    break;
                }
            if (!fail) {
                ignore(v);
                reduced = true;
            }
        }
        return reduced;
    }

    bool reduce_subset(ui u) {
        if (check_subset(u)) {
            exclude(u);
            return true;
        }
        return false;
    }

    //  Apply the rules exhaustively.
    bool reduce() {
        size_t t = history.size();
        bool reduced;
        do {
            reduced = false;
            for (ui u = 0; u < n; ++u) {
                if (undetermined(u))
                    reduced |= reduce_subset(u);
                if (!dominated(u))
                    reduced |= reduce_single_dominator(u);
                if (!dominated(u))
                    reduced |= reduce_ignore(u);
            }
        } while (reduced);
        return history.size() != t;
    }

};

struct ijcai20_inf : virtual mds_context_t {

    ijcai20_inf(const ugraph& g) : mds_context_t(g) {}

    bool reduce_pass() {
        size_t t = history.size();
        //  isolated vertex rule
        for (ui v = 0; v < n; ++v) {
            if (!undetermined(v)) continue;
            if (g[v].size() == 1)
                select(v);
            else {
                ui cnt = 0;
                for (ui u : g[v])
                    if (excluded(u))
                        cnt++;
                if (cnt + 1 == g[v].size())
                    select(v);
            }
        }
        
        //  leaf rule
        for (ui v = 0; v < n; ++v) {
            if (dominated(v)) continue;
            ui f = frequency(v);
            if (f == 1) {
                for (ui u : g[v]) {
                    if (!excluded(u)){
                        select(u);
                        break;
                    }
                }
            }
            else if (f == 2 && coverage_size(v) == 2) {
                if (excluded(v)) continue;
                exclude(v);
            }
        }

        for (ui w = 0; w < n; ++w) {
            bool succ = false;
            for (ui v : g[w]) {
                if (v == w || g[v].size() != 3)
                    continue;
                for (ui u : g[v]) {
                    if (u == v || u == w || g[u].size() != 3 || !g[u].contains(w))
                        continue;
                    if (undetermined(u)) exclude(u);
                    if (undetermined(v)) exclude(v);
                    if (undetermined(w)) select(w);
                    succ = true;
                    break;
                }
            }
        }
        return history.size() != t;
    }

    bool reduce() {
        bool succ = false;
        while (reduce_pass())
            succ = true;
        return succ;
    }
};