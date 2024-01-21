#pragma once
#include "util/log.hpp"
#include "graph.hpp"

typedef std::vector<ui> mds_solution_t;

class mds_context_t {
public:

    enum op_t { op_select, op_exclude, op_ignore };
    
    struct record {
        op_t op;
        ui u;
    };

    ui n;
    ugraph g;

    //  D[u]: Does u belongs to the current solution
    vector<char> D;

    //  X[u]: Does u excluded by the current solution
    vector<char> X;

    //  I[u]: does u ignored by the current solution
    vector<char> I;

    // c_d[u]: How many vertices in N[u] are selected into the solution.
    // c_d[u] = |N[u] \cap D| = |{(u, d) \in E | d \in D}| + [u \in D]
    // c_d[u] > 0 ==> u is dominated.
    vector<ui> c_d;

    // c_nd[u]: How many vertices in N[u] are dominated
    // c_nd[u] = |N[u] \cap (N[D])| = |{(u, v) \in E | v \in N[D]}| + [u \in N[D]]
    // Select u into solution will dominate |N[u]|-c_nd[u] more vertices.
    vector<ui> c_nd;

    // c_x[u]: How many vertices in N[u] are excluded by the solution.
    // c_x[u] = |N[u] \cap X| = |{(u,v) \in E | v \in X}+[u \in X]|
    vector<ui> c_x;

    ui cnt_s;   //  Size of the current solution, |D|
    ui cnt_d;   //  Number of the dominated vertices, |N[D]|
    ui cnt_x;   //  Number of the excluded vertices, |X|

    //  recorded history for rolling back
    vector<record> history;

    mds_solution_t best;

    mds_context_t(const ugraph& g) : 
        n(g.off.size() - 1),
        g(g),
        D(n, 0), X(n, 0), I(n, 0),
        c_d(n, 0), c_nd(n, 0), c_x(n, 0),
        cnt_s(0), cnt_d(0), cnt_x(0),
        best(n) { iota(best.begin(), best.end(), 0); }
    
    bool selected(ui u) const { return D[u]; }

    bool dominated(ui u) const { return c_d[u] > 0; }

    bool excluded(ui u) const { return X[u]; }
    
    bool undetermined(ui u) const { return !X[u] && !D[u]; }

    bool ignored(ui u) const { return I[u]; }

    ui selected() const { return cnt_s; }

    ui selected(const vector<ui>& V) const {
        return count_if(V.begin(), V.end(), [this](ui v) { return selected(v); });
    }

    ui dominated() const { return cnt_d; }

    ui dominated(const vector<ui>& V) const {
        return count_if(V.begin(), V.end(), [this](ui v) { return dominated(v); });
    }

    ui excluded() const { return cnt_x; }
    
    ui undetermined() const { return n - cnt_s - cnt_x; }

    ui coverage_size(ui u) const {
        assert(undetermined(u));
        return g[u].size() - c_nd[u];
    }

    vector<ui> coverage(ui u) const {
        assert(undetermined(u));
        vector<ui> w;
        for (ui v : g.adj(u))
            if (!dominated(v))
                w.push_back(v);
        return w;
    }

    ui frequency(ui u) const {
        assert(!dominated(u));
        return g.adj(u).size() - c_x[u];
    }

    vector<ui> dominators(ui u) const {
        assert(!dominated(u));
        vector<ui> w;
        for (ui v : g[u])
            if (!excluded(v))
                w.push_back(v);
        return w;
    }

    //  Basic operations
    void mark_dominated(ui u) {
        if (!c_d[u]++) {
            cnt_d++;
            for (ui w : g[u])
                c_nd[w]++;
        }
    }

    void mark_undominated(ui u) {
        if (!--c_d[u]) {
            cnt_d--;
            for (ui w : g[u])
                c_nd[w]--;
        }
    }   

    void select(ui u) {
        assertf(undetermined(u) && "u must be undetermined before select(u)");
        history.push_back({ op_select, u });
        D[u] = 1;
        cnt_s++;
        for (ui v : g[u])
            mark_dominated(v);
    }

    void unselect(ui u) {
        assertf(selected(u) && "u must be selected before unselect(u)");
        D[u] = 0;
        cnt_s--;
        for (ui v : g[u])
            mark_undominated(v);
    }

    void exclude(ui u) {
        assertf(undetermined(u) && "u must be undetermined before exclude(u)");
        history.push_back({ op_exclude, u });
        X[u] = 1;
        cnt_x++;
        for (ui v : g[u]) {
            c_x[v]++;
            if (!dominated(v) && frequency(v) == 1) {
                for (ui w : g[v])
                    if (!excluded(w)) {
                        select(w);
                        break;
                    }
            }
        }
        logf<_trace_>("exclude %d from solution\n", u);
    }

    void unexclude(ui u) {
        assertf(excluded(u) && "u must be excluded before unexclude(u)");
        X[u] = 0;
        cnt_x--;
        for (ui v : g[u])
            c_x[v]--;
        logf<_trace_>("unexclude %d from solution\n", u);
    }

    void ignore(ui u) {
        assertf(!ignored(u) && "u must not be ignored before ignore(u)\n");
        history.push_back({ op_ignore, u });
        I[u] = 1;
        mark_dominated(u);
        logf<_trace_>("ignore %d from solution\n", u);
    }

    void unignore(ui u) {
        assertf(ignored(u) && "u must be ignored before unignore(u)\n");
        I[u] = 0;
        mark_undominated(u);
        logf<_trace_>("unignore %d from solution\n", u);
    }

    void rollback(size_t t) {
        while (history.size() > t) {
            ui u = history.back().u;
            switch (history.back().op) {
            case op_select:
                unselect(u);
                break;
            case op_exclude:
                unexclude(u);
                break;
            case op_ignore:
                unignore(u);
                break;
            }
            history.pop_back();
        }
    }
    
    bool update_best() {
        assertf(dominated() == n && "must be a valid solution");
        if (selected() >= best.size())
            return false;
        best.clear();
        for (ui u = 0; u < n; ++u)
            if (selected(u))
                best.push_back(u);
        return true;
    }
    
};