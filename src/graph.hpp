#pragma once
#include <bits/stdc++.h>
#include "util/cmap.hpp"
using namespace std;

typedef unsigned ui;
typedef pair<ui, ui> pii;
typedef pair<ui, vector<pii>> edge_list_graph;

template<class T>
struct closed_adj_t {
    T p_begin;
    T p_end;
    const char* p_mat;
    vector<unordered_set<ui>>::const_iterator p_set;
    

    T begin() { return p_begin; }
    T end() { return p_end; }
    size_t size() { return p_end - p_begin; }

    bool contains(ui v) const noexcept { return p_mat ? p_mat[v] : p_set->count(v); }
};

const ui thr_big = 5000;

class ugraph {
public:
    typedef vector<ui>::iterator iterator;
    typedef vector<ui>::const_iterator const_iterator;

    ui n;
    vector<ui> off, dst;
    vector<char> mat;
    vector<unordered_set<ui>> umap;

    ugraph() = default;
    ugraph(const ugraph&) = default;

    ugraph(ui n, const vector<pii>& es): n(n), umap(n) {
        assign(n, es);
    }

    closed_adj_t<iterator> adj(ui u) {
        return closed_adj_t<iterator>{
            dst.begin() + off[u],
            dst.begin() + off[u + 1],
            n < thr_big ? &mat.front() + u * n : nullptr,
            umap.begin() + u,
        };
    }

    closed_adj_t<const_iterator> adj(ui u) const {
        return closed_adj_t<const_iterator>{
            dst.begin() + off[u],
            dst.begin() + off[u + 1],
            n < thr_big ? &mat.front() + u * n : nullptr,
            umap.begin() + u,
        };
    }

    closed_adj_t<iterator> operator[](ui u) {
        return adj(u);
    }

    closed_adj_t<const_iterator> operator[](ui u) const {
        return adj(u);        
    }

    void assign(ui n, const vector<pii>& es) {
        this->n = n;
        off.assign(n + 1, 1); off.back() = 0;
        dst.assign(es.size() * 2 + n, ~0u);
        if (n < thr_big) {
            mat.assign(n * n, 0);
            for (ui i = 0; i < n; ++i)
                mat[n * i + i] = 1;
            for (pii e : es) 
                mat[n * e.first + e.second] = mat[n * e.second + e.first] = 1;
        }
        for (pii e : es) {
            off[e.first]++;
            off[e.second]++;
            umap[e.first].insert(e.second);
            umap[e.second].insert(e.first); 
        }
        for (ui i = 0; i < n; ++i)
            off[i + 1] += off[i];
        for (pii e : es) {
            dst[--off[e.first]] = e.second;
            dst[--off[e.second]] = e.first;
        }
        for (ui i = 0; i < n; ++i) {
            dst[--off[i]] = i;
            sort(adj(i).begin() + 1, adj(i).end());
        }
        
    }

};

ugraph read_graph_dimacs(istream& is) {
    string line;
    getline(is, line);
    ui n, m; sscanf(line.c_str(), "p edge %d %d", &n, &m);
    vector<pii> es(m);
    for (ui i = 0; i < m; ++i) {
        getline(is, line);
        sscanf(line.c_str(), "e %d %d", &es[i].first, &es[i].second);
        es[i].first--;
        es[i].second--;
    }
    return ugraph(n, es);
}

ugraph read_graph_simple(istream& is, bool header = false) {
    string line;
    vector<pii> es;
    if (header) {
        ui n, m; 
        getline(is, line);
        sscanf(line.c_str(), "%d %d", &n, &m);
        es.reserve(m);
    }
    ui n = 0;
    while (getline(is, line)) {
        ui u, v;
        sscanf(line.c_str(), "%d %d", &u, &v);
        if (u == v) continue;
        if (u > v) swap(u, v);
        es.emplace_back(u, v);
        n = max(n, max(u, v) + 1);
    }
    sort(es.begin(), es.end());
    es.erase(unique(es.begin(), es.end()), es.end());
    return ugraph(n, es);
}

ugraph read_graph_matrix(istream& is) {
    ui n, m;
    is >> n >> m;
    vector<pii> es;
    for (ui u = 0; u < n; ++u)
        for (ui v = 0; v < n; ++v) {
            ui a;
            is >> a;
            if (a && u < v)
                es.emplace_back(u,v);
        }
    return ugraph(n, es);
}

ugraph read_edges(istream& is, ui minn = 0) {
    string line;
    vector<pii> es;
    while (getline(is, line)) {
        ui u, v;
        sscanf(line.c_str(), "%d %d", &u, &v);
        if (u == v) continue;
        if (u > v) swap(u, v);
        es.emplace_back(u, v);
        minn = max(minn, max(u, v) + 1);
    }
    sort(es.begin(), es.end());
    es.erase(unique(es.begin(), es.end()), es.end());
    return ugraph(minn, es);
}

ugraph read_mtx(istream& is) {
    string line;
    getline(is, line);
    while (line[0] == '%')
        getline(is, line);
    int n1, n2, m;
    sscanf(line.c_str(),"%d %d %d", &n1, &n2, &m);
    return read_edges(is, max(n1, n2));
}

ugraph read_graph(istream& is, string graph_type) {
    if (graph_type == "U")
        return read_graph_simple(is, true);
    else if (graph_type == "LU")
        return read_graph_simple(is, false);
    else if (graph_type == "DU")
        return read_graph_dimacs(is);
    else if (graph_type == "M")
        return read_graph_matrix(is);
    else if (graph_type == "edges")
        return read_edges(is);
    else if (graph_type == "mtx")
        return read_mtx(is);
    else
        return ugraph();
}