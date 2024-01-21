#pragma once
#include <vector>

using std::vector;

template<class T>
struct cmap : vector<T> {
    using vector<T>::begin;
    using vector<T>::end;
    using vector<T>::erase;

    void build() {
        sort(begin(), end());
        erase(unique(begin(), end()), end());
    }

    size_t id(T t) {
        return lower_bound(begin(), end(), t) - begin();
    }
};