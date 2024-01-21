# IJCAI24-MDS
Supplementary files for 'Exact Algorithms for Minimum Dominating Set and its Generalization'

Experimental results are listed in `results.xlsx`.

Source codes are placed inside `src`.

### Compile and Run

Before compilation, you need to install the HiGHS library from [here](https://highs.dev/).

```bash
g++ bib.cpp -o bib -std=c++17 -O3 -DNDEBUG -DLOG_LEVEL=2 -DNO_ASSERT 
-I'/path/to/include/highs' -L'/path/to/HiGHS/build/lib' -lhighs
```

```bash
./bib <graph_type> <graph_path> <algorithm>
```

Algorithms: `biblp,bibco,biblp-if,bibco-if,mcslb,ddlb`.

Graph type:  `DU` for all graphs in UDG and T1. `M` for all graphs in `BD3/BD6`. Corresponding extension name(`edges/mtx`) for all graphs in Network Repository.

### Converting BD3/BD6 to adapt EMOS implementation

EMOS does not read 0/1 matrix format.

We provide `conv.cpp` to convert graphs in BD3/BD6 into `.edges` format.

It should compile on any cpp compiler that supports C++11 standard.

### Synthetic graph generator

We use `gen.cpp` to generate random graphs for experiments.

It should compile on any cpp compiler that supports C++11 standard.

