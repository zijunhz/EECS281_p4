#include <getopt.h>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <limits>
#include <vector>
#define doubleInf numeric_limits<double>::infinity()
using namespace std;

// #define DEBUG_INPUT
// #define DEBUG_MST

enum class Mode { MST, FASTTSP, OPTTSP };

class CagePair {
   public:
    uint16_t i;
    uint16_t j;
    CagePair(uint16_t x, uint16_t y) : i(x), j(y) {}
};

class Cage {
   public:
    int32_t x;
    int32_t y;
    enum Type { Wall, Safe, Wild };
    Type type;
    Cage() : x(0), y(0), type(Type::Wall) {}
    Cage(int32_t x, int32_t y) : x(x), y(y) {
        if ((x > 0 || y > 0) && (x != 0) && (y != 0))
            type = Type::Safe;
        else if (x < 0 && y < 0)
            type = Type::Wild;
        else
            type = Type::Wall;
    }
    Cage(const Cage& c) : x(c.x), y(c.y), type(c.type) {}
    Cage& operator=(const Cage& c) {
        Cage t(c);
        swap(*this, t);
        return *this;
    }
    struct Distinguish {
        double operator()(const Cage& a, const Cage& b) {
            if (a.type + b.type == 3)
                return doubleInf;
            return sqrt(double(a.x - b.x) * double(a.x - b.x) + double(a.y - b.y) * double(a.y - b.y));
        }
    };
    struct Mix {
        double operator()(const Cage& a, const Cage& b) {
            return sqrt(double(a.x - b.x) * double(a.x - b.x) + double(a.y - b.y) * double(a.y - b.y));
        }
    };
    void print() { cout << "    Cage(" << x << "," << y << ") type: " << type << "\n"; }
};

// this is for part B: https://stemlounge.com/animated-algorithms-for-the-traveling-salesman-problem/

// template <typename T>
// void genPerms(vector<T>& path, size_t permLength) {
//     if (permLength == path.size()) {
//         // Do something with the path
//         // curCost+=closing edge
//         // check the path and see if it's better

//         // curCost-= closing edge
//         return;
//     }  // if ..complete path

//     if (!promising(path, permLength)) {
//         return;
//     }  // if ..not promising

//     // if estimate > opt, cut
//     // do mst on the rest
//     // only calc estimate when # unvisited >= 5

//     for (size_t i = permLength; i < path.size(); ++i) {
//         swap(path[permLength], path[i]);
//         // curCost+=
//         // stop genPerm
//         genPerms(path, permLength + 1);
//         // curCost-=
//         swap(path[permLength], path[i]);
//     }  // for ..unpermuted elements
// }  // genPerms()

template <typename Dist>
double mst(vector<Cage>& cages, uint16_t startIndex, uint16_t endIndex, vector<CagePair>& res, Dist dist) {
    // index mapped to i-startIndex
    // [startIndex,endIndex)
    if (startIndex == endIndex)
        return 0;
    res.reserve(endIndex - startIndex);
    vector<uint16_t> pre(endIndex - startIndex, 0);
    vector<double> dis(endIndex - startIndex, doubleInf);
    vector<bool> vis(endIndex - startIndex, false);
    dis[0] = 0;
    double len = 0;
    for (uint16_t count = 0; count < endIndex - startIndex; ++count) {
        double minDis = doubleInf;
        uint16_t minNode = 0;
        for (uint16_t j = 0; j < endIndex - startIndex; ++j)
            if ((!vis[j]) && dis[j] < minDis) {
                minDis = dis[j];
                minNode = j;
            }
        if (minDis == doubleInf) {
            cerr << "Cannot construct MST";
            exit(1);
        }
        vis[minNode] = true;
        if (count > 0) {
            res.push_back(CagePair(minNode + startIndex, pre[minNode] + startIndex));
            len += dist(cages[minNode + startIndex], cages[pre[minNode] + startIndex]);
        }
        for (uint16_t j = 0; j < endIndex - startIndex; ++j) {
            if (vis[j])
                continue;
            double newDis = dist(cages[startIndex + j], cages[startIndex + minNode]);
            if (newDis < dis[j]) {
                pre[j] = minNode;
                dis[j] = newDis;
            }
        }
#ifdef DEBUG_MST
        cout << "--------\n";
        for (uint16_t i = 0; i < endIndex - startIndex; ++i) {
            cout << "    " << dis[i] << "\n";
        }
#endif
    }
    return len;
}

int main(int argc, char** argv) {
    ios_base::sync_with_stdio(false);
    cin.tie(0);
    cout.tie(0);
    cout << std::setprecision(2);  // Always show 2 decimal places
    cout << std::fixed;            // Disable scientific notation for large numbers

    Mode mode = Mode::MST;
    {  // command line args
        int optionIdx = 0;
        int option = 0;
        opterr = 0;
        struct option longOpts[] = {{"help", no_argument, nullptr, 'h'}, {"mode", required_argument, nullptr, 'm'}};
        while ((option = getopt_long(argc, argv, "hm:", longOpts, &optionIdx)) != -1) {
            switch (option) {
                case 'h':
                    cout << "I'm too lazy to write help. Check out https://eecs281staff.github.io/p4-zoo/\n";
                    return 0;
                case 'm':
                    if (!strcmp("MST", optarg)) {
                        mode = Mode::MST;
                    } else if (!strcmp("FASTTSP", optarg)) {
                        mode = Mode::FASTTSP;
                    } else if (!strcmp("OPTTSP", optarg)) {
                        mode = Mode::OPTTSP;
                    } else {
                        cerr << "Invalid mode";
                        exit(1);
                    }
                    break;
                default:
                    cerr << "Invalid command line option";
                    exit(1);
            }
        }
    }
    uint16_t n;
    cin >> n;
    vector<Cage> cages;
    cages.reserve(n);
    for (uint16_t i = 0; i < n; ++i) {
        int x, y;
        cin >> x >> y;
        cages.push_back(Cage(x, y));
#ifdef DEBUG_INPUT
        cages.back().print();
#endif
    }
    if (mode == Mode::MST) {
        vector<CagePair> res;
        double len = mst(cages, 0, n, res, Cage::Distinguish());
        cout << len << "\n";
        for (int i = 0; i < n - 1; ++i) {
            cout << min(res[i].i, res[i].j) << " " << max(res[i].i, res[i].j) << '\n';
        }
    } else if (mode == Mode::FASTTSP) {
    }

    // in part C, call part B first to be the starting point of part C.
    return 0;
}
// use nested loop prim
