#include <bits/stdc++.h>
using namespace std;

int main(int argc, char** argv) {
    int n = atoi(argv[1]);
    for (int k = 200; k <= 500; k += 30) {
        bernoulli_distribution d(0.1 * k);
        for (int s = 0; s < 10; ++s) {
            mt19937_64 mt(s);
            vector<pair<int, int>> es;
            for (int u = 0; u < n; ++u)
                for (int v = u + 1; v < n; ++v)
                        es.emplace_back(u, v);
            vector<pair<int, int>> fs;
            sample(es.begin(), es.end(), back_inserter(fs), k, mt);
            ofstream ofs("syn_"+to_string(n)+"_"+to_string(k)+"_"+to_string(s));
            for (auto p : fs)
                ofs << p.first << ' ' << p.second << endl;
        }
    }
    return 0;
}