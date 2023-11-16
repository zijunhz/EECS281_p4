// 3E33912F8BAA7542FC4A1585D2DB6FE0312725B9

#include <getopt.h>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <limits>
#include <list>
#include <vector>
#define doubleInf numeric_limits<double>::infinity()
using namespace std;

// #define DEBUG_INPUT
// #define DEBUG_MST
// #define DEBUG_PROMISING

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
        if (x < 0 && y < 0)
            type = Type::Wild;
        else if ((x == 0 && y <= 0) || (y == 0 && x <= 0))
            type = Type::Wall;
        else
            type = Type::Safe;
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
    struct DistinguishSq {
        double operator()(const Cage& a, const Cage& b) {
            if (a.type + b.type == 3)
                return doubleInf;
            return double(a.x - b.x) * double(a.x - b.x) + double(a.y - b.y) * double(a.y - b.y);
        }
    };
    struct Mix {
        double operator()(const Cage& a, const Cage& b) {
            return sqrt(double(a.x - b.x) * double(a.x - b.x) + double(a.y - b.y) * double(a.y - b.y));
        }
    };
    struct MixSq {
        double operator()(const Cage& a, const Cage& b) {
            return double(a.x - b.x) * double(a.x - b.x) + double(a.y - b.y) * double(a.y - b.y);
        }
    };
    void print() { cout << "    Cage(" << x << "," << y << ") type: " << type << "\n"; }
};

// this is for part B: https://stemlounge.com/animated-algorithms-for-the-traveling-salesman-problem/

template <typename Dist, typename DistSq>
double mst(vector<Cage>& cages,
           uint16_t startIndex,
           uint16_t endIndex,
           vector<CagePair>& res,
           Dist dist,
           DistSq distSq) {
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
            double newDis = distSq(cages[startIndex + j], cages[startIndex + minNode]);
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

double nearestInsert(vector<Cage>& cages, vector<uint16_t>& res) {
    vector<bool> isIn(cages.size(), false);
    res.resize(cages.size() + 1);
    Cage::Mix dist;
    Cage::MixSq distSq;
    if (cages.size() > 1) {
        double minDis = doubleInf;
        uint16_t cage = 1;
        for (uint16_t i = 1; i < cages.size(); ++i) {
            double nowDis = distSq(cages[0], cages[i]);
            if (nowDis < minDis) {
                minDis = nowDis;
                cage = i;
            }
        }
        isIn[0] = true;
        isIn[cage] = true;
        res[0] = 0;
        res[1] = cage;
        res[2] = 0;
    }
    vector<uint16_t> nearest(cages.size(), 0);
    for (uint16_t i = 1; i < cages.size(); ++i) {
        if (!isIn[i]) {
            nearest[i] = distSq(cages[i], cages[0]) < distSq(cages[i], cages[res[1]]) ? 0 : res[1];
        }
    }

    for (uint16_t count = 2; count < cages.size(); ++count) {
        double minDis = doubleInf;
        uint16_t cage = 1;
        for (uint16_t i = 1; i < cages.size(); ++i) {
            if (isIn[i])
                continue;
            double nowDis = distSq(cages[i], cages[nearest[i]]);
            if (nowDis < minDis) {
                minDis = nowDis;
                cage = i;
            }
        }
        double minCost = doubleInf;
        uint16_t insertBefore = 0;
        for (uint16_t i = count; i > 0; --i) {
            double nowCost = dist(cages[res[i]], cages[cage]) + dist(cages[res[i - 1]], cages[cage]) -
                             dist(cages[res[i]], cages[res[i - 1]]);
            if (nowCost < minCost) {
                minCost = nowCost;
                insertBefore = i;
            }
        }
        for (uint16_t i = count + 1; i > insertBefore; --i) {
            res[i] = res[i - 1];
        }
        res[insertBefore] = cage;
        isIn[cage] = true;
        for (uint16_t i = 1; i < cages.size(); ++i) {
            if ((!isIn[i]) && distSq(cages[cage], cages[i]) < distSq(cages[i], cages[nearest[i]])) {
                nearest[i] = cage;
            }
        }
    }

    // for (uint16_t i = 1; i < cages.size(); ++i) {
    //     for (uint16_t j = i + 2; j < cages.size(); ++j) {
    //         if (dist(cages[res[i - 1]], cages[res[i]]) + dist(cages[res[i + 1]], cages[res[i]]) +
    //                 dist(cages[res[j - 1]], cages[res[j]]) + dist(cages[res[j + 1]], cages[res[j]]) >
    //             dist(cages[res[i - 1]], cages[res[j]]) + dist(cages[res[i + 1]], cages[res[j]]) +
    //                 dist(cages[res[j - 1]], cages[res[i]]) + dist(cages[res[j + 1]], cages[res[i]]))
    //             swap(res[i], res[j]);
    //     }
    // }

    double len = 0;
    for (uint16_t i = 0; i < cages.size(); ++i)
        len += dist(cages[res[i]], cages[res[i + 1]]);
    return len;
}

bool promising(vector<Cage>& cages, vector<uint16_t>& path, uint16_t permLength, double curLen, double& bestLen) {
    double min1 = doubleInf, min2 = doubleInf;
    Cage::Mix dist;
    for (uint16_t i = permLength; i < cages.size(); ++i) {
        min1 = min(min1, dist(cages[path[0]], cages[path[i]]));
        min2 = min(min2, dist(cages[path[permLength - 1]], cages[path[i]]));
    }
#ifdef DEBUG_PROMISING
    if (true)
        return curLen + min1 + min2 < bestLen;
#endif
    if (cages.size() < permLength + 6U)
        return curLen + min1 + min2 < bestLen;
    vector<CagePair> temp;
    double mstLen = mst(cages, permLength, uint16_t(cages.size()), temp, Cage::Mix(), Cage::MixSq());
    return curLen + min1 + min2 + mstLen < bestLen;
}

void genPerms(vector<Cage>& cages,
              vector<uint16_t>& ans,
              double& bestLen,
              vector<uint16_t>& path,
              double curLen,
              uint16_t permLength) {
    Cage::Mix dist;
    if (permLength == path.size()) {
        // Do something with the path
        curLen += dist(cages[path[0]], cages[path.back()]);
        // check the path and see if it's better
        if (curLen < bestLen) {
            bestLen = curLen;
            ans = path;
        }
        return;
    }  // if ..complete path

    if (!promising(cages, path, permLength, curLen, bestLen)) {
        return;
    }  // if ..not promising

    // if estimate > opt, cut
    // do mst on the rest
    // only calc estimate when # unvisited >= 5

    for (uint16_t i = permLength; i < path.size(); ++i) {
        swap(path[permLength], path[i]);
        curLen += dist(cages[path[permLength]], cages[path[permLength - 1]]);
        // stop genPerm
        genPerms(cages, ans, bestLen, path, curLen, permLength + 1);
        curLen -= dist(cages[path[permLength]], cages[path[permLength - 1]]);
        swap(path[permLength], path[i]);
    }  // for ..unpermuted elements
}  // genPerms()

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
        double len = mst(cages, 0, n, res, Cage::Distinguish(), Cage::DistinguishSq());
        cout << len << "\n";
        for (int i = 0; i < n - 1; ++i) {
            cout << min(res[i].i, res[i].j) << " " << max(res[i].i, res[i].j) << '\n';
        }
    } else if (mode == Mode::FASTTSP) {
        vector<uint16_t> res;
        double len = nearestInsert(cages, res);
        cout << len << '\n';
        for (uint16_t i = 0; i < n; ++i)
            cout << res[i] << ' ';
    } else {
        vector<uint16_t> res;
        double len = nearestInsert(cages, res);
        vector<uint16_t> temp(res);
        temp.pop_back();
        genPerms(cages, res, len, temp, 0, 1);
        cout << len << '\n';
        for (uint16_t i = 0; i < n; ++i)
            cout << res[i] << ' ';
    }

    // in part C, call part B first to be the starting point of part C.
    return 0;
}
// use nested loop prim
