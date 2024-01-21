#pragma once
#include <Highs.h>
#include "context.hpp"

struct mds_highs_lb_t : virtual mds_context_t {
    
    struct mds_model_t {
        ui sel;
        size_t t;
        cmap<ui> c;
        unique_ptr<Highs> highs;

        void select(ui u) {
            HighsInt idx = c.id(u);
            double val = 1;
            highs->addRow(1, 1, 1, &idx, &val);
        }

        void exclude(ui u) {
            HighsInt idx = c.id(u);
            double val = 1;
            highs->addRow(0, 0, 1, &idx, &val);
        }

        ui lb() {
            highs->run();
            return ceil(highs->getObjectiveValue() - 1e-6) + sel;
        }

        mds_solution_t extract() {
            const auto& hsol = highs->getSolution();
            mds_solution_t sol;
            for (ui u = 0; u < c.size(); ++u)
                if (hsol.col_value[u] > 1e-6)
                    sol.push_back(c[u]);
            sort(sol.begin(), sol.end(), [&hsol, this](ui u1, ui u2) { return hsol.col_value[c.id(u1)] > hsol.col_value[c.id(u2)]; });
            return sol;
        }
    };

    vector<mds_model_t> stack;

    void push() {
        cmap<ui> c;
        for (ui u = 0; u < n; ++u)
            if (undetermined(u))
                c.push_back(u);

        HighsModel model;
        model.lp_.num_col_ = undetermined();
        model.lp_.num_row_ = g.n - dominated();
        model.lp_.sense_ = ObjSense::kMinimize;

        model.lp_.col_cost_ = vector<double>(undetermined(), 1);
        model.lp_.col_lower_ = vector<double>(undetermined(), 0);
        model.lp_.col_upper_ = vector<double>(undetermined(), 1);
        model.lp_.row_lower_ = vector<double>(g.n - dominated(), 1);
        model.lp_.row_upper_ = vector<double>(g.n - dominated(), kHighsInf);

        HighsSparseMatrix a;
        a.format_ = MatrixFormat::kRowwise;
        a.start_ = vector<HighsInt>(1, 0);  
        for (ui u = 0; u < g.n; ++u) {
            if (dominated(u)) continue;
            for (ui v : g[u]) {
                if (!undetermined(v)) continue;
                a.index_.push_back(c.id(v));
                a.value_.push_back(1);
            }
            HighsInt last = a.start_.back() + frequency(u);
            a.start_.push_back(last);
        }
        model.lp_.a_matrix_ = a;

        stack.push_back({ selected(), history.size(), c, make_unique<Highs>()});
        const auto& highs = stack.back().highs;
        highs->setOptionValue("presolve", "off");
        highs->setOptionValue("parallel", "off");
        highs->setOptionValue("simplex_strategy", kSimplexStrategyDual);
        highs->setOptionValue("output_flag", false);
        highs->passModel(model);
    }

    void pop() {
        stack.pop_back();
    }

    mds_highs_lb_t(const ugraph& g) : mds_context_t(g) {
        stack.reserve(n);
    }

    mds_model_t& current() {
        return stack.back();
    }

    ui lb_lp() {
        return current().lb();
    }

    mds_solution_t extract() {
        return current().extract();
    }
};