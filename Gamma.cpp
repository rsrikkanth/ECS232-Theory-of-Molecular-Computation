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
            ret += dirname[d] + ":" + glue[d].first + ":" + 
            to_string(glue[d].second) + (d == 3 ? "" : ", ");
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
        }

        if (copy && carry) {
            if (lef) t.glue[NORTH] = {"fL", 2};
            else if (rig) t.glue[NORTH] = {"fR", 1};
            else t.glue[NORTH] = {"fM", 1};
        }
        genTile = t;
        return true;
    }
    counter(int _lef, int _rig, int _copy, int _val, int _carry) {
        lef = _lef; rig = _rig; copy = _copy; val = _val; carry = _carry;
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

tile sqTile;
struct squareTile{
    int lef, rig, state; // 0 => diag, 1 => besDiag, 2 => topLef, 3 => botRig
    squareTile() {
        lef = rig = state = -1;
    }
    squareTile(int p) {
        state = p % 4; int pos = p / 4; 
        if (pos == 0) {lef = 0; rig = 0;}
        else if (pos == 1) {lef = 1; rig = 0;}
        else {lef = 0; rig = 1;}
    }
    bool isEmpty() {
        return lef == -1;
    }
    string print() {
        if (isEmpty()) return "";
        return to_string(lef) + to_string(rig) + to_string(state);
    }
    bool code(squareTile south, squareTile west, squareTile east) {
        string inp = "";
        if (!south.isEmpty()) inp += 'S'; if (!west.isEmpty()) inp += 'W';
        if (!east.isEmpty()) inp += 'E';
        tile t;
        lef = south.lef; rig = south.rig;
        if (inp == "S") {
            if (south.state != 1) return false;
            state = 0;
            t.glue[SOUTH] = {south.print() + "S", 2};
            if (rig) {
                t.glue[NORTH] = {"null", 0};
                t.glue[EAST] = {"10100Sp", 2};
            } else {
                t.glue[NORTH] = {print() + "SE", 1};
                t.glue[EAST] = {print() + "SW", 1};
            }
            t.glue[WEST] = {print() + "SE", 1};
        } else if (inp == "SW") {
            if (west.state == 0) {
                state = 1;
            } else if (west.state == 1) {
                state = 3;
            } else if (west.state == 3) {
                state = 3;
            } else {
                return false;
            }
            if (state == 1) {
                t.glue[NORTH] = {print() + "S", 2};
            } else {
                t.glue[NORTH] = {print() + "SW", 1};
            }
            t.glue[SOUTH] = {south.print() + "SW", 1};
            if (rig) {
                t.glue[EAST] = {"00100SWp", 1};
            } else {
                t.glue[EAST] = {print() + "SW", 1};
            }
            t.glue[WEST] = {west.print() + "SW", 1};
        } else if (inp == "SE") {
            state = 2;
            t.glue[SOUTH] = {south.print() + "SE", 1};
            t.glue[EAST] = {east.print() + "SE", 1};
            t.glue[NORTH] = {print() + "SE", 1};
            t.glue[WEST] = {print() + "SE", 1};
        } else {
            return false;
        }
        sqTile = t; sqTile.color = "Red";
        return true;
    }
};

struct tileSystem {
    vector<tile> seed;
    vector<tile> tilesets;
    void print() {
        int sz = tilesets.size();
        for (int i=0;i<sz;++i) {
            // tilesets[i].label = to_string(i);
            cout << "TILENAME " << tilesets[i].label << '\n';
            string str = tilesets[i].glue[NORTH].first;
            cout << "LABEL " << (str.size() < 4 ? 'n' : str[3]) << '\n';
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
    tile lseed, rseed, mseed;
    rseed.label = "0"; rseed.glue[NORTH] = {"01010S", 2}; 
    rseed.glue[WEST] = {"fill", 2}; rseed.glue[EAST] = {"null", 0}; 
    rseed.glue[SOUTH] = {"null", 0};


    mseed.label = "1"; mseed.glue[NORTH] = {"00000SE", 1}; 
    mseed.glue[WEST] = {"fill", 2}; mseed.glue[EAST] = {"fill", 2}; 
    mseed.glue[SOUTH] = {"null", 0};


    lseed.label = "2"; lseed.glue[NORTH] = {"10010SE", 1}; 
    lseed.glue[WEST] = {"null", 0}; lseed.glue[EAST] = {"fill", 2}; 
    mseed.glue[SOUTH] = {"null", 0};

    auto nxt = rotate90(tiles);

    tiles.push_back(rseed);
    tiles.push_back(mseed);
    tiles.push_back(lseed);

    int st = tiles.size();
    vector<tile> sqtileset;
    for (int s=0;s<3*4;++s) {
        int pos = s / 4, state = s % 4;
        squareTile sq;
        if (sq.code(squareTile(s), squareTile(), squareTile())) {
            tiles.push_back(sqTile);
            sqtileset.push_back(sqTile);
        }
        for (int ew=0;ew<3*4;++ew) {
            if (sq.code(squareTile(s), squareTile(ew), squareTile())) {
                tiles.push_back(sqTile);
                sqtileset.push_back(sqTile);
            }
            if (sq.code(squareTile(s), squareTile(), squareTile(ew))) {
                tiles.push_back(sqTile);
                sqtileset.push_back(sqTile);
            }
        }
    }
    for (int i=st;i<tiles.size();++i) tiles[i].label = to_string(i);
    tile lseed2, mseed2, rseed2, mmseed2, mseed3;
    lseed2.label = "2p"; mseed2.label = "1p"; mmseed2.label = "15p"; rseed2.label = "0p";
    lseed2.glue = {{"oL", 1}, {"ffL", 1}, {"null", 0}, {"fL", 2}};
    mseed2.glue = {{"mL", 1}, {"fgM", 2}, {"oL", 1}, {"fM", 1}};
    mmseed2.glue = {{"mL", 1}, {"ffM", 1}, {"mL", 1}, {"fM", 1}};
    rseed2.glue = {{"01110SWp", 1}, {"ffR", 1}, {"mL", 1}, {"fR", 1}};

    tiles.push_back(lseed2); tiles.push_back(mseed2); tiles.push_back(mmseed2); tiles.push_back(rseed2);

    lseed2.label = "2pp"; mseed2.label = "1pp"; mmseed2.label = "15pp"; rseed2.label = "0pp";
    lseed2.glue = {{"oL", 1}, {"102SE", 1}, {"null", 0}, {"ffL", 1}};
    mseed2.glue = {{"mmL", 1}, {"102SE", 1}, {"oL", 1}, {"fgM", 2}};
    mseed3.glue = {{"mL", 1}, {"001S", 2}, {"mmL", 1}, {"ffM", 1}};
    mseed3.label = "125pp";
    mmseed2.glue = {{"mL", 1}, {"003SW", 1}, {"mL", 1}, {"ffM", 1}};
    rseed2.glue = {{"00110SWp", 1}, {"013SW", 1}, {"mL", 1}, {"ffR", 1}};

    tiles.push_back(lseed2); tiles.push_back(mseed2); tiles.push_back(mmseed2); tiles.push_back(rseed2); tiles.push_back(mseed3);



    for (auto &tl : nxt) {
        if (tl.glue[EAST].first == "fLp") {
            tl.glue[EAST] = {"101Sp", 2};
        } else if (tl.glue[EAST].first == "fMp") {
            tl.glue[EAST] = {"003SWp", 1};
        } else if (tl.glue[EAST].first == "fRp") {
            tl.glue[EAST] = {"013SWp", 1};
        }
        tiles.push_back(tl);
    }

    auto nxtSet = rotate90(sqtileset);
    for (auto &tl : nxtSet) {
        tl.color = "Red";
        if (tl.glue[WEST].first[1] == '1') {
            if (tl.glue[EAST].second == 0) {
                tl.glue[SOUTH].first = "101Spp";
            } else {
                tl.glue[SOUTH].first = "003SWpp";
            }
        }
        tiles.push_back(tl);
    }

    auto lastSet = rotate90(nxtSet);
    for (auto &tl : lastSet) {
        tl.color = "Green";
        tiles.push_back(tl);
    }

    sort(tiles.begin(), tiles.end());
    ts.tilesets = tiles;
    ts.print();
    return ts;
}

int main() {
    genCounter();
    return 0;
}