// 3E33912F8BAA7542FC4A1585D2DB6FE0312725B9

#include <getopt.h>
#include <algorithm>
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
    vector<double> dis;
    template <typename Dist, typename DistSq>
    double mst(Dist dist, DistSq distSq) {
        if (cages.size() <= 1)
            return 0;
        mstRes.reserve(cages.size());
        vector<uint16_t> pre(cages.size(), 0);
        vector<double> dis(cages.size(), doubleInf);
        dis[0] = 0;
        double len = 0;
        for (uint16_t count = 0; count < cages.size(); ++count) {
            double minDis = doubleInf;
            uint16_t minNode = 0;
            for (uint16_t j = 0; j < path.size(); ++j)
                if (dis[path[j]] < minDis) {
                    minDis = dis[path[j]];
                    minNode = j;
                }
            if (minDis == doubleInf) {
                cerr << "Cannot construct MST";
                exit(1);
            }
            swap(path[minNode], path.back());
            minNode = path.back();
            path.pop_back();
            if (count > 0) {
                mstRes.emplace_back(CagePair(minNode, pre[minNode]));
                len += dist(cages[minNode], cages[pre[minNode]]);
            }
            for (uint16_t j = 0; j < path.size(); ++j) {
                double newDis = distSq(cages[path[j]], cages[minNode]);
                if (newDis < dis[path[j]]) {
                    pre[path[j]] = minNode;
                    dis[path[j]] = newDis;
                }
            }
        }
        return len;
    }

    double mstLocal(uint16_t startIndex) {
        if (startIndex + 1U >= cages.size())
            return 0;
        vector<uint16_t> left(path.begin() + startIndex, path.end());
        uint16_t n = uint16_t(cages.size()) - startIndex;
        for (auto v : left) {
            dis[v] = doubleInf;
        }
        dis[left[0]] = 0;
        double len = 0;
        double minDis = 0;
        uint16_t minNode = 0;
        for (uint16_t count = 0; count < n; ++count) {
            swap(left[minNode], left.back());
            minNode = left.back();
            left.pop_back();
            len += minDis;
            minDis = doubleInf;
            uint16_t nxt = 0;
            for (auto it = left.begin(); it != left.end(); ++it) {
                if (adjMat[*it][minNode] < dis[*it]) {
                    dis[*it] = adjMat[*it][minNode];
                }
                if (dis[*it] < minDis) {
                    minDis = dis[*it];
                    nxt = uint16_t(it - left.begin());
                }
            }
            minNode = nxt;
        }
        return len;
    }

    bool promising(uint16_t permLength) {
        double min1 = doubleInf, min2 = doubleInf;
        for (uint16_t i = permLength; i < cages.size(); ++i) {
            min1 = min(min1, adjMat[path[0]][path[i]]);  // dist(cages[], cages[]));
            min2 = min(min2,
                       adjMat[path[permLength - 1]][path[i]]);  // dist(cages[path[permLength - 1]],cages[path[i]]));
        }
        return curLen + min1 + min2 + mstLocal(permLength) < bestLen;
    }

    void genPerms(uint16_t permLength) {
        if (permLength == path.size()) {
            curLen += adjMat[path[0]][path[permLength - 1]];
            if (curLen < bestLen) {
                bestLen = curLen;
                pathRes = path;
            }
            curLen -= adjMat[path[0]][path[permLength - 1]];
            return;
        }

        if (permLength > 2 && permLength < cages.size() - 1 && (!promising(permLength))) {
            return;
        }
        for (uint16_t i = permLength; i < path.size(); ++i) {
            swap(path[permLength], path[i]);
            curLen += adjMat[path[permLength]][path[permLength - 1]];
            genPerms(permLength + 1);
            curLen -= adjMat[path[permLength]][path[permLength - 1]];
            swap(path[permLength], path[i]);
        }
    }

    double nearestInsert() {
        vector<uint16_t> left(cages.size() - 1, 0);
        iota(left.begin(), left.end(), 1);
        list<uint16_t> path;
        path.push_back(0);
        path.push_back(0);
        vector<double> distNow(cages.size(), doubleInf);
        distNow[0] = 0;
        auto curV = path.begin();
        // int step = int(cages.size()) / 3 + 2;
        for (uint16_t count = 1; count < cages.size(); ++count) {
            uint16_t nxt = 0;
            double maxDis = doubleInf;
            for (uint16_t i = 0; i < left.size(); ++i) {
                // double distNow = distSq(cages[left[i]], cages[*nearest[left[i]]]);
                double challenger = adjMat[left[i]][*curV];  // distSq(cages[left[i]], cages[*curV]);
                if (distNow[left[i]] > challenger) {
                    distNow[left[i]] = challenger;
                }
                if (distNow[left[i]] < maxDis) {
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
                double curDis = adjMat[*it][nxt] + adjMat[*nxtIt][nxt] - adjMat[*it][*nxtIt];
                if (curDis < maxDis) {
                    maxDis = curDis;
                    toInsert = it;
                }
            }
            curV = path.insert(next(toInsert), nxt);
        }
        double len = 0;
        for (auto it = path.begin(), nxtIt = next(it); nxtIt != path.end(); it = nxtIt++) {
            len += adjMat[*it][*nxtIt];  // dist(cages[*it], cages[*nxtIt]);
        }
        return len;
    }

    double selectFarthestInsert(uint16_t startV) {
        vector<uint16_t> left(cages.size(), 0);
        iota(left.begin(), left.end(), 0);
        swap(left[startV], left.back());
        list<uint16_t> path;
        path.push_back(left.back());
        path.push_back(left.back());
        left.pop_back();
        vector<double> distNow(cages.size(), doubleInf);
        auto curV = path.begin();
        for (uint16_t count = 1; count < cages.size(); ++count) {
            uint16_t nxt = 0;
            double maxDis = 0;
            for (uint16_t i = 0; i < left.size(); ++i) {
                double challenger = adjMat[left[i]][*curV];
                if (distNow[left[i]] > challenger) {
                    distNow[left[i]] = challenger;
                }
                if (distNow[left[i]] > maxDis) {
                    maxDis = distNow[left[i]];
                    nxt = i;
                }
            }
            swap(left[nxt], left.back());
            nxt = left.back();
            left.pop_back();

            auto toInsert = path.begin();
            maxDis = doubleInf;
            for (auto it = path.begin(), nxtIt = next(it); nxtIt != path.end(); it = nxtIt++) {
                double curDis = adjMat[*it][nxt] + adjMat[*nxtIt][nxt] - adjMat[*it][*nxtIt];
                if (curDis < maxDis) {
                    maxDis = curDis;
                    toInsert = nxtIt;
                }
            }
            curV = path.insert(toInsert, nxt);
        }
        double len = 0;
        for (auto it = path.begin(), nxtIt = next(it); nxtIt != path.end(); it = nxtIt++) {
            len += adjMat[*it][*nxtIt];
        }
        return len;
    }

    double randomInsert() {
        vector<uint16_t> left(cages.size(), 0);
        iota(left.begin(), left.end(), 0);
        for (uint16_t i = 0; i < cages.size(); ++i) {
            uint16_t target = uint16_t(rand() % cages.size());
            swap(left[i], left[target]);
        }
        list<uint16_t> path;
        path.push_back(left.back());
        path.push_back(left.back());
        left.pop_back();
        vector<list<uint16_t>::iterator> nearest(cages.size(), path.begin());
        vector<double> distNow(cages.size(), doubleInf);
        auto curV = path.begin();
        for (uint16_t count = 1; count < cages.size(); ++count) {
            uint16_t nxt = 0;
            double maxDis = doubleInf;
            for (uint16_t i = 0; i < left.size(); ++i) {
                double challenger = adjMat[left[i]][*curV];
                if (distNow[left[i]] > challenger) {
                    nearest[left[i]] = curV;
                    distNow[left[i]] = challenger;
                }
            }
            nxt = uint16_t(rand() % left.size());
            uint16_t temp = left[nxt];
            left[nxt] = left.back();
            left.pop_back();
            nxt = temp;
            auto toInsert = path.begin();
            maxDis = doubleInf;
            for (auto it = path.begin(), nxtIt = next(it); nxtIt != path.end(); it = nxtIt++) {
                double curDis = adjMat[*it][nxt] + adjMat[*nxtIt][nxt] - adjMat[*it][*nxtIt];
                if (curDis < maxDis) {
                    maxDis = curDis;
                    toInsert = it;
                }
            }
            path.insert(next(toInsert), nxt);
            curV = toInsert;
        }
        double len = 0;
        for (auto it = path.begin(), nxtIt = next(it); nxtIt != path.end(); it = nxtIt++) {
            len += adjMat[*it][*nxtIt];
        }
        return len;
    }
};

// this is for part B: https://stemlounge.com/animated-algorithms-for-the-traveling-salesman-problem/

double farthestInsert(vector<Cage>& cages, vector<uint16_t>& res, int ratio) {
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
    int step = int(cages.size()) / ratio + 2;
    for (uint16_t count = 1; count < cages.size(); ++count) {
        uint16_t nxt = 0;
        double maxDis = 0;
        for (uint16_t i = 0; i < left.size(); ++i) {
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
        auto it = nearest[nxt];
        for (int i = 0; i < step && it != path.begin(); ++i)
            --it;
        for (int i = 0; i < (step << 1) && next(it) != path.end(); ++i, ++it) {
            double curDis = dist(cages[*it], cages[nxt]) + dist(cages[*(next(it))], cages[nxt]) -
                            dist(cages[*it], cages[*next(it)]);
            if (curDis < maxDis) {
                maxDis = curDis;
                toInsert = it;
            }
        }
        curV = path.insert(next(toInsert), nxt);
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
        double len = sol.mst(Cage::Distinguish(), Cage::DistinguishSq());
        cout << len << "\n";
        for (int i = 0; i < n - 1; ++i) {
            cout << min(sol.mstRes[i].i, sol.mstRes[i].j) << " " << max(sol.mstRes[i].i, sol.mstRes[i].j) << '\n';
        }
    } else if (mode == Mode::FASTTSP) {
        vector<uint16_t> res;
        double len = farthestInsert(sol.cages, res, 8);
        cout << len << '\n';
        for (uint16_t i = 0; i < n; ++i)
            cout << res[i] << ' ';
    } else {
        srand((unsigned)time(NULL));
        sol.dis.resize(n);
        sol.adjMat = vector<vector<double>>(n, vector<double>(n, 0));
        Cage::Mix dist;
        for (uint16_t i = 0; i < n; ++i) {
            for (uint16_t j = 0; j < i; ++j) {
                sol.adjMat[i][j] = dist(sol.cages[i], sol.cages[j]);
            }
            for (uint16_t j = i + 1; j < n; ++j) {
                sol.adjMat[i][j] = dist(sol.cages[i], sol.cages[j]);
            }
        }
        double len = sol.nearestInsert();
        for (uint16_t i = 1; i < n; ++i)
            len = min(len, sol.selectFarthestInsert(i));
        // for (uint16_t i = 0; i < 4; ++i)
        //     len = min(len, sol.randomInsert());
        // for (uint16_t i = 0; i < n; ++i)
        //     cout << sol.selectFarthestInsert(i) << "\n";
        vector<uint16_t> res;
        len = min(farthestInsert(sol.cages, res, 1), len);
        sol.bestLen = len;
        sol.curLen = 0;
        sol.path = res;
        sol.pathRes = res;

        sol.genPerms(1);
        cout << sol.bestLen << '\n';
        for (uint16_t i = 0; i < n; ++i)
            cout << sol.pathRes[i] << ' ';
    }

    // in part C, call part B first to be the starting point of part C.
    return 0;
}
// use nested loop prim
