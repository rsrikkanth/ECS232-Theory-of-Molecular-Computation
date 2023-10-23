#include <iostream>
#include <fstream>
#include <assert.h>

using namespace std;

const int EAST = 0, NORTH = 1, WEST = 2, SOUTH = 3;
const string dirname[4] = {"EAST", "NORTH", "WEST", "SOUTH"};

struct tile {
    string label, color;
    vector<pair<string, int>> glue;
    tile() {
        glue.resize(4);
        color = "Cyan";
    }
    string print() {
        string ret = "";
        for (int d=0;d<4;++d) {
            ret += dirname[d] + ":" + glue[d].first + ":" + to_string(glue[d].second) + (d == 3 ? "" : ", ");
        }
        return ret;
    }
};

tile genTile;
struct counter {
    int lef, rig, copy, val, carry;
    string print() {
        if (isEmpty()) return "";
        return to_string(lef) + to_string(rig) + to_string(copy) + to_string(val) + to_string(carry);
    }
    bool code(counter south, counter east, counter west) {
        if (south.isEmpty()) return false;
        lef = south.lef; rig = south.rig; copy = south.copy^1;
        val = (copy ? south.val : ((west.isEmpty() ? 1 : west.carry) + south.val) & 1);
        carry = (copy ? (east.isEmpty() ? south.carry : east.carry) : ((west.isEmpty() ? 1 : west.carry) + south.val) >> 1);

        if (east.isEmpty() != (!copy || (copy && rig))) return false;
        if (west.isEmpty() != (copy || (!copy && lef))) return false;
        if ((!west.isEmpty() && west.rig) || (!east.isEmpty() && east.lef)) return false;
        // cout << "here\n";
        if (!east.isEmpty() && copy != east.copy) return false;
        if (!west.isEmpty() && copy != west.copy) return false;

        string inp = "S", outn = "S", outv = "S";
        if (copy) {
            if (!rig) {inp += "E";}
            if (!lef) {outn += "W"; outv += "E";}
        } else {
            if (!lef) inp += "W";
            if (!rig) {outn += "E"; outv += "W";}
        }

        tile t;
        t.glue[SOUTH] = {south.print() + inp, 3 - inp.size()}; 
        t.glue[NORTH] = {print() + outn, 3 - outn.size()};
        if (!west.isEmpty()) {
            t.glue[WEST] = {west.print() + inp, 1};
        } else {
            if (!lef) t.glue[WEST] = {print() + outv, 1};
        }
        if (!east.isEmpty()) {
            t.glue[EAST] = {east.print() + inp, 1};
        } else {
            if (!rig) t.glue[EAST] = {print() + outv, 1};
            else {
                if (copy && carry) {
                    t.glue[EAST] = {"Lp", 2};
                } else {
                    t.glue[EAST] = {"Mp", 1};
                }
            }
        }

        if (copy && carry) {
            t.glue[NORTH] = {"", 0};
        }
        genTile = t;
        return true;
    }
    counter(int _lef, int _rig, int _copy, int _val, int _carry) {
        lef = _lef; rig = _rig; copy = _copy; val = _val; carry = _carry;
        // cout << "setting : " << lef << " " << rig << " " << copy << " " << val << " " << carry << '\n';
    }
    counter(int mask=-1) {
        if (mask == -1) {
            lef = rig = copy = val = carry = -1;
            return;
        }
        lef = ((mask >> 0) & 1);
        rig = ((mask >> 1) & 1);
        copy = ((mask >> 2) & 1);
        val = ((mask >> 3) & 1);
        carry = ((mask >> 4) & 1);
    }
    bool isEmpty() {
        return lef == -1;
    }
};

vector<tile> rotate90(vector<tile> tiles) {
    int sz = tiles.size();
    vector<tile> ret;
    for (int i=0;i<sz;++i) {
        tile rt;
            rt.label = tiles[i].label + "p";
        for (int dir=0;dir<4;++dir) {
            rt.glue[dir] = tiles[i].glue[(dir + 1) & 3];
            rt.glue[dir].first += "p";
        }
        ret.push_back(rt);
    }
    return ret;
}

struct tileSystem {
    vector<tile> seed;
    vector<tile> tilesets;
    void print() {
        int sz = tilesets.size();
        for (int i=0;i<sz;++i) {
            // tilesets[i].label = to_string(i);
            cout << "TILENAME " << tilesets[i].label << '\n';
            string str = tilesets[i].glue[NORTH].first;
            cout << "LABEL " << tilesets[i].label << '\n';
            for (int dir=0;dir<4;++dir) {
                cout << dirname[dir] << "BIND " << tilesets[i].glue[dir].second << '\n';
                cout << dirname[dir] << "LABEL " << tilesets[i].glue[dir].first << '\n';
            }
            cout << "TILECOLOR " << tilesets[i].color << '\n';
            cout << "CREATE\n\n";
        }
    }
};

bool operator<(tile a, tile b) {
    for (int dir=0;dir<4;++dir) {
        if (a.glue[dir] != b.glue[dir]) {
            return a.glue[dir] < b.glue[dir];
        }
    }
    return false;
}
bool operator==(tile a, tile b) {
    return (!(a < b)) && (!(b < a));
}

tileSystem genCounter() {
    tileSystem ts;
    vector<tile> tiles;
    for (int mask=0;mask<32;++mask) {
        counter south = counter(mask), east = counter(-1), west = counter(-1);
        for (int mask2=-1;mask2<32;++mask2) {   
            east = counter(mask2);
            counter c;
            if (!c.code(south, east, west)) continue;
            // genTile.print();
            tiles.push_back(genTile);
        }
        east = counter(-1);
        for (int mask2=-1;mask2<32;++mask2) {
            west = counter(mask2);
            counter c;
            if (!c.code(south, east, west)) continue;
            tiles.push_back(genTile);
        }
    }
    sort(tiles.begin(), tiles.end());
    tiles.erase(unique(tiles.begin(), tiles.end()), tiles.end());
    int tsz = tiles.size(); for (int i=0;i<tsz;++i) tiles[i].label = to_string(i + 3);
    // for (auto &tl : tiles) cout << tl.print() << '\n';
    // cout << tiles.size() << '\n';
    tile lseed, rseed, mseed,mseed2;
    rseed.label = "0"; rseed.glue[NORTH] = {"01010S", 2}; rseed.glue[WEST] = {"fill", 2}; rseed.glue[EAST] = {"Rp", 1}; rseed.glue[SOUTH] = {"null", 0};

    mseed.label = "1"; mseed.glue[NORTH] = {"00000SE", 1}; mseed.glue[WEST] = {"fill", 2}; mseed.glue[EAST] = {"fill", 2}; mseed.glue[SOUTH] = {"null", 0};

    // mseed2.label = "3"; mseed2.glue[NORTH] = {"00000SE", 1}; mseed2.glue[WEST] = {"10010SW", 2}; mseed2.glue[EAST] = {"00000SW", 1}; mseed2.glue[SOUTH] = {"00100SW", 0};

    lseed.label = "2"; lseed.glue[NORTH] = {"10010SE", 1}; lseed.glue[WEST] = {"", 0}; lseed.glue[EAST] = {"fill", 2}; lseed.glue[SOUTH] = {"null", 0};

    tiles.push_back(rseed);
    tiles.push_back(mseed);
    tiles.push_back(lseed);

    auto nxt = rotate90(tiles);
    int cnt = tiles.size();
    for (auto &tl : nxt) {
        tl.label = to_string(cnt++);
        tl.color = "magenta";
        tiles.push_back(tl);
    }
    tile lt, rt, mt;
    rt.label = "0p"; rt.glue[NORTH] = {"fill", 1}; rt.glue[WEST] = {"Rp", 1}; rt.glue[EAST] = {"01010Sp", 2}; rt.glue[SOUTH] = {"Rpp", 1};
    mt.label = "1p"; mt.glue[NORTH] = {"fill", 1}; mt.glue[WEST] = {"Mp", 1}; mt.glue[EAST] = {"00000SEp", 1}; mt.glue[SOUTH] = {"fill", 1};
    lt.label = "2p"; lt.glue[NORTH] = {"fill", 1}; lt.glue[WEST] = {"Lp", 2}; lt.glue[EAST] = {"10010SEp", 1}; lt.glue[SOUTH] = {"fill", 1};
    lt.color = mt.color = rt.color = "magenta";

    tiles.push_back(lt);
    tiles.push_back(rt);
    tiles.push_back(mt);

    auto nxt3 = rotate90(nxt);
    cnt = tiles.size();
    for (auto &tl : nxt3) {
        tl.label = to_string(cnt++);
        tl.color = "green";
        tiles.push_back(tl);
    }

    rt.label = "0pp"; rt.glue[EAST] = {"fill", 1}; rt.glue[NORTH] = {"Rpp", 1}; rt.glue[SOUTH] = {"01010Spp", 2}; rt.glue[WEST] = {"Rppp", 0};
    mt.label = "1p"; mt.glue[EAST] = {"fill", 1}; mt.glue[NORTH] = {"Mpp", 1}; mt.glue[SOUTH] = {"00000SEpp", 1}; mt.glue[WEST] = {"fill", 1};
    lt.label = "2p"; lt.glue[EAST] = {"fill", 1}; lt.glue[NORTH] = {"Lpp", 2}; lt.glue[SOUTH] = {"10010SEpp", 1}; lt.glue[WEST] = {"fill", 1};
    lt.color = mt.color = rt.color = "green";

    tiles.push_back(lt);
    tiles.push_back(rt);
    tiles.push_back(mt);

    sort(tiles.begin(), tiles.end());
    ts.tilesets = tiles;
    ts.print();
    return ts;
}

int main() {
    genCounter();
    // squareTile a()
    // counter south(0), east(6), west(-1), c;
    // cout << c.code(south, east, west) << '\n';
    // cout << genTile.print() << '\n';
    return 0;
}