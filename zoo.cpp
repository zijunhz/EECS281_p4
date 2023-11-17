// 3E33912F8BAA7542FC4A1585D2DB6FE0312725B9

#include <getopt.h>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <limits>
#include <list>
#include <numeric>
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

class Solution {
   public:
    vector<Cage> cages;
    vector<CagePair> mstRes;
    vector<uint16_t> pathRes;
    vector<uint16_t> path;
    double bestLen;
    double curLen;
    vector<vector<double>> adjMat;
    template <typename Dist, typename DistSq>
    double mst(uint16_t startIndex, uint16_t endIndex, Dist dist, DistSq distSq, bool needRes) {
        // index mapped to i-startIndex
        // [startIndex,endIndex)
        if (startIndex == endIndex)
            return 0;
        if (needRes)
            mstRes.reserve(endIndex - startIndex);
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
                if (needRes)
                    mstRes.emplace_back(CagePair(path[minNode + startIndex], path[pre[minNode] + startIndex]));
                len += needRes ? dist(cages[path[minNode + startIndex]], cages[path[pre[minNode] + startIndex]])
                               : sqrt(adjMat[path[minNode + startIndex]][path[pre[minNode] + startIndex]]);
            }
            for (uint16_t j = 0; j < endIndex - startIndex; ++j) {
                if (vis[j])
                    continue;
                double newDis = needRes ? distSq(cages[path[startIndex + j]], cages[path[startIndex + minNode]])
                                        : adjMat[path[startIndex + j]][path[startIndex + minNode]];
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

    bool promising(uint16_t permLength) {
        // return true;
        // return curLen < bestLen;
        double min1 = doubleInf, min2 = doubleInf;
        for (uint16_t i = permLength; i < cages.size(); ++i) {
            min1 = min(min1, sqrt(adjMat[path[0]][path[i]]));  // dist(cages[], cages[]));
            min2 = min(
                min2,
                sqrt(adjMat[path[permLength - 1]][path[i]]));  // dist(cages[path[permLength - 1]], cages[path[i]]));
        }
#ifdef DEBUG_PROMISING
        if (true)
            return curLen + min1 + min2 < bestLen;
#endif
        if (cages.size() < permLength + 6U)
            return curLen + min1 + min2 < bestLen;
        double mstLen = mst(permLength, uint16_t(cages.size()), Cage::Mix(), Cage::MixSq(), false);
        return curLen + min1 + min2 + mstLen < bestLen;
    }

    void genPerms(uint16_t permLength) {
        if (permLength == path.size()) {
            // Do something with the path
            curLen += sqrt(adjMat[path[0]][path.back()]);  // dist(cages[path[0]], cages[path.back()]);
            // check the path and see if it's better
            if (curLen < bestLen) {
                bestLen = curLen;
                pathRes = path;
            }
            curLen -= sqrt(adjMat[path[0]][path.back()]);  // dist(cages[path[0]], cages[path.back()]);
            return;
        }  // if ..complete path

        if (!promising(permLength)) {
            return;
        }  // if ..not promising

        // if estimate > opt, cut
        // do mst on the rest
        // only calc estimate when # unvisited >= 5

        for (uint16_t i = permLength; i < path.size(); ++i) {
            swap(path[permLength], path[i]);
            curLen +=
                sqrt(adjMat[path[permLength]]
                           [path[permLength - 1]]);  // dist(cages[path[permLength]], cages[path[permLength - 1]]);
            // stop genPerm
            genPerms(permLength + 1);
            curLen -=
                sqrt(adjMat[path[permLength]]
                           [path[permLength - 1]]);  // dist(cages[path[permLength]], cages[path[permLength - 1]]);
            swap(path[permLength], path[i]);
        }  // for ..unpermuted elements
    }      // genPerms()
};

// this is for part B: https://stemlounge.com/animated-algorithms-for-the-traveling-salesman-problem/

double farthestInsert(vector<Cage>& cages, vector<uint16_t>& res) {
    vector<uint16_t> left(cages.size() - 1, 0);
    iota(left.begin(), left.end(), 1);
    list<uint16_t> path;
    path.push_back(0);
    path.push_back(0);
    vector<list<uint16_t>::iterator> nearest(cages.size(), path.begin());
    vector<double> distNow(cages.size(), doubleInf);
    distNow[0] = 0;
    Cage::MixSq distSq;
    Cage::Mix dist;
    auto curV = path.begin();
    // int step = int(cages.size()) / 3 + 2;
    for (uint16_t count = 1; count < cages.size(); ++count) {
        uint16_t nxt = 0;
        double maxDis = 0;
        for (uint16_t i = 0; i < left.size(); ++i) {
            // double distNow = distSq(cages[left[i]], cages[*nearest[left[i]]]);
            double challenger = distSq(cages[left[i]], cages[*curV]);
            if (distNow[left[i]] > challenger) {
                nearest[left[i]] = curV;
                distNow[left[i]] = challenger;
            }
            if (distNow[left[i]] > maxDis) {
                maxDis = distNow[left[i]];
                nxt = i;
            }
        }
        uint16_t temp = left[nxt];
        left[nxt] = left.back();
        left.pop_back();
        nxt = temp;

        auto toInsert = path.begin();
        maxDis = doubleInf;
        for (auto it = path.begin(), nxtIt = next(it); nxtIt != path.end(); it = nxtIt++) {
            double curDis =
                dist(cages[*it], cages[nxt]) + dist(cages[*nxtIt], cages[nxt]) - dist(cages[*it], cages[*nxtIt]);
            if (curDis < maxDis) {
                maxDis = curDis;
                toInsert = it;
            }
        }
        // auto it = nearest[nxt];
        // for (int i = 0; i < step && it != path.begin(); ++i)
        //     --it;

        // for (int i = 0; i < (step << 1) && next(it) != path.end(); ++i, ++it) {
        //     double curDis = dist(cages[*it], cages[nxt]) + dist(cages[*(next(it))], cages[nxt]) -
        //                     dist(cages[*it], cages[*next(it)]);
        //     if (curDis < maxDis) {
        //         maxDis = curDis;
        //         toInsert = it;
        //     }
        // }
        path.insert(next(toInsert), nxt);
        curV = toInsert;
    }
    double len = 0;
    res.reserve(cages.size() + 1);
    for (auto v : path) {
        if (res.size() > 0)
            len += dist(cages[res.back()], cages[v]);
        res.push_back(v);
    }
    res.pop_back();
    return len;
}

double nearestInsert(vector<Cage>& cages, vector<uint16_t>& res) {
    vector<uint16_t> left(cages.size() - 1, 0);
    iota(left.begin(), left.end(), 1);
    list<uint16_t> path;
    path.push_back(0);
    path.push_back(0);
    vector<list<uint16_t>::iterator> nearest(cages.size(), path.begin());
    Cage::MixSq distSq;
    Cage::Mix dist;
    int step = int(cages.size()) / 20 + 10;
    for (uint16_t count = 1; count < cages.size(); ++count) {
        uint16_t nxt = 0;
        double minDis = doubleInf;
        for (uint16_t i = 0; i < left.size(); ++i) {
            double distNow = distSq(cages[left[i]], cages[*nearest[left[i]]]);
            if (distNow < minDis) {
                minDis = distNow;
                nxt = i;
            }
        }
        uint16_t temp = left[nxt];
        left[nxt] = left.back();
        left.pop_back();
        nxt = temp;

        auto toInsert = path.begin();
        minDis = doubleInf;
        auto it = nearest[nxt];
        for (int i = 0; i < step && it != path.begin(); ++i)
            --it;

        for (int i = 0; i < (step << 1) && next(it) != path.end(); ++i, ++it) {
            double curDis = dist(cages[*it], cages[nxt]) + dist(cages[*(next(it))], cages[nxt]) -
                            dist(cages[*it], cages[*next(it)]);
            if (curDis < minDis) {
                minDis = curDis;
                toInsert = it;
            }
        }
        path.insert(next(toInsert), nxt);

        for (uint16_t i = 0; i < left.size(); ++i) {
            if (distSq(cages[left[i]], cages[*nearest[left[i]]]) > distSq(cages[left[i]], cages[nxt])) {
                nearest[left[i]] = toInsert;
            }
        }
    }
    double len = 0;
    res.reserve(cages.size() + 1);
    for (auto v : path) {
        if (res.size() > 0)
            len += dist(cages[res.back()], cages[v]);
        res.push_back(v);
    }
    res.pop_back();
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
    Solution sol;
    sol.cages.reserve(n);
    for (uint16_t i = 0; i < n; ++i) {
        int x, y;
        cin >> x >> y;
        sol.cages.emplace_back(Cage(x, y));
#ifdef DEBUG_INPUT
        cages.back().print();
#endif
    }
    if (mode == Mode::MST) {
        sol.path.resize(n);
        iota(sol.path.begin(), sol.path.end(), 0);
        double len = sol.mst(0, n, Cage::Distinguish(), Cage::DistinguishSq(), true);
        cout << len << "\n";
        for (int i = 0; i < n - 1; ++i) {
            cout << min(sol.mstRes[i].i, sol.mstRes[i].j) << " " << max(sol.mstRes[i].i, sol.mstRes[i].j) << '\n';
        }
    } else if (mode == Mode::FASTTSP) {
        vector<uint16_t> res;
        double len = farthestInsert(sol.cages, res);
        cout << len << '\n';
        for (uint16_t i = 0; i < n; ++i)
            cout << res[i] << ' ';
    } else {
        vector<uint16_t> res;
        double len = farthestInsert(sol.cages, res);
        sol.bestLen = len;
        sol.curLen = 0;
        sol.path = res;
        sol.pathRes = res;
        sol.adjMat = vector<vector<double>>(n, vector<double>(n, doubleInf));
        Cage::MixSq dist;
        for (uint16_t i = 0; i < n; ++i) {
            for (uint16_t j = 0; j < i; ++j)
                sol.adjMat[i][j] = dist(sol.cages[i], sol.cages[j]);
            for (uint16_t j = i + 1; j < n; ++j)
                sol.adjMat[i][j] = dist(sol.cages[i], sol.cages[j]);
        }
        sol.genPerms(1);
        cout << sol.bestLen << '\n';
        for (uint16_t i = 0; i < n; ++i)
            cout << sol.pathRes[i] << ' ';
    }

    // in part C, call part B first to be the starting point of part C.
    return 0;
}
// use nested loop prim
