#pragma once 
#include "reduce.hpp"
#include "lower_bounds.hpp"
#include "highs.hpp"

enum {
    flg_red = 1,
    flg_noignore = 2,
    flg_ddlb = 4,
    flg_mcslb = 8,
    flg_lplb = 16,
};

template<size_t flags, class LB, class RED>
struct mds_branch_t : virtual mds_context_t, RED, mds_basic_lbs_t, LB {

    //  Size of the search tree
    size_t nodes;

    mds_branch_t(const ugraph& g) :
        mds_context_t(g),
        RED(g),
        mds_basic_lbs_t(g),
        LB(g) {}

    void branch(int d = 0) {
        nodes++;
        if (flags & flg_red) RED::reduce();
        
        if (dominated() == n) {
            update_best();
            return;
        }
        
        
        if (selected() + 1 >= best.size())
            return;
        if ((flags & flg_ddlb) && lb_disjoint_dominators() >= best.size())
            return;
        if ((flags & flg_mcslb) && lb_max_coverage_size() >= best.size())
            return;

        ui lplb = 0;
        if (flags & flg_lplb) {
            LB::push();
            if ((lplb = LB::lb_lp()) >= best.size())
                return LB::pop();
            auto sol = LB::extract();
            size_t t = history.size();
            for (ui u : sol)
                if (coverage_size(u) != 0)
                    select(u);
            if (update_best())
                logf<_debug_>("LP: %d\n", best.size());
            this->rollback(t);
        }

        ui u = ~0;    
        //  Select the vertex with the minimum frequency
        tuple<ui, ui, ui> tpl(n, 0, 0);
        for (ui v = 0; v < n; ++v) {
            if (dominated(v))
                continue;
            ui cs = 0;
            for (ui w : g[v])
                if (!excluded(w))
                    cs += coverage_size(w);
            tpl = min(tpl, make_tuple(frequency(v), n * n - cs, v));
            // if (!~u || frequency(v) <= frequency(u))
            //     u = v;
        }
        u = get<2>(tpl);

        //  Sort its dominators by decreasing coverage size.
        vector<ui> b = dominators(u);
        sort(b.begin(), b.end(), [this](ui v1, ui v2) { return coverage_size(v1) > coverage_size(v2); });

#ifdef LOG_BRANCH
        const int log_depth = 20;
        static int dc[log_depth], dp[log_depth];
        if (d <= log_depth) {
            char buf[1024], *p = buf;
            p += sprintf(p,  "[%lu]", best.size());
            for (int i = 0; i < d; ++i) 
                p += sprintf(p, "-%d(%d)", dc[i], dp[i]);
            logf<_debug_>("%s\n", buf);
        }
        if (d < log_depth)
            dc[d] = b.size(), dp[d] = u;
#endif
        for (ui v : b) {
#ifdef LOG_BRANCH
            if (d < log_depth) dc[d]--;
#endif
            if (selected(v)) {
                branch(d + 1);
                break;
            }
            size_t t = history.size();
            select(v);
            branch(d + 1);
            rollback(t);
            if (v == b.back())
                break;
            if ((flags & flg_lplb) && lplb >= best.size())
                break;
            exclude(v);
            if (flags & flg_lplb) {
                LB::current().exclude(v);
                if ((lplb = LB::lb_lp()) >= best.size())
                    break;
            } 
        }
        
        if (flags & flg_lplb) LB::pop();
    }

    void solve() {
        size_t t = history.size();
        nodes = 0;
        branch();
        rollback(t);
    }
};
