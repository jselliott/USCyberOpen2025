#include <cstdlib>
#include <iostream>
#include <utility>
#include <vector>
#include <string>
#include <map>

// Based on Busy Beaver code here: https://catonmat.net/busy-beaver

using namespace std;

typedef vector<char> Array;
typedef map<string, string> Mapping;

class TM {
private:
    Array tape;
    Mapping program;
    char start, halt, init, state;
    unsigned char tape_changed;
    int moves;
    int pos;
public:
    TM(Mapping program, char start, char halt, char init):
        tape(1, init), program(program), start(start), halt(halt),
        init(init), state(start), moves(0), tape_changed(1), pos(0)
    { }

    void run() {
        while (state != halt) {
            print_array();
            string lhs = get_lhs();
            string rhs = get_rhs(lhs);

            char new_state = rhs[0];
            char new_symbol = rhs[1];
            char move = rhs[2];

            char old_symbol = lhs[1];
            update_array(old_symbol, new_symbol);
            update_tool(new_state);
            move_ptr(move);
        }
        print_array();
    }

    int get_moves() {
        return moves;
    }

private:
    inline void print_array() {
        if (tape_changed) {
            for (int i=0; i<tape.size(); i++)
                cout << tape[i];
            cout << endl;
        }
    }

    inline string get_lhs() {
        char sp[3] = {0};
        sp[0] = state;
        sp[1] = tape[pos];
        return string(sp);
    }

    inline string get_rhs(string &lhs) {
        return program[lhs];
    }

    inline void update_array(char old_symbol, char new_symbol) {
        if (old_symbol != new_symbol) {
            tape[pos] = new_symbol;
            tape_changed++;
        }
        else {
            tape_changed = 0;
        }
    }

    inline void update_tool(char new_state) {
        state = new_state;
    }

    inline void move_ptr(char move) {
        if (move == 'l')
            pos -= 1;
        else if (move == 'r')
            pos += 1;
        else
            throw string("unknown tool");

        if (pos < 0) {
            tape.insert(tape.begin(), init);
            pos = 0;
        }
        if (pos >= tape.size()) {
            tape.push_back(init);
        }
        moves++;
    }
};

vector<Mapping> workers;

void init_bb() 
{
    Mapping bb5;
    // Inject the corrupted pairings below
    bb5.insert(make_pair("a0", "b1r"));
    bb5.insert(make_pair("b0", "c1l"));
    bb5.insert(make_pair("c0", "a1l"));
    bb5.insert(make_pair("d0", "!@#"));
    //bb5.insert(make_pair("d0", "h1r"));
    bb5.insert(make_pair("e0", "$%^"));
    //bb5.insert(make_pair("e0", "b0r"));

    bb5.insert(make_pair("a1", "&*("));
    //bb5.insert(make_pair("a1", "d0l"));
    bb5.insert(make_pair("b1", "c1r"));
    bb5.insert(make_pair("c1", "c0r"));
    bb5.insert(make_pair("d1", "e0l"));
    bb5.insert(make_pair("e1", "):?"));
    //bb5.insert(make_pair("e1", "d1l"));

    workers.push_back(bb5);
}


void init_workers()
{
    workers.push_back(Mapping());
    init_bb();
}

void worker()
{
    TM tm(workers[1], 'a', 'h', '0');
    tm.run();
    cout << "Barney Beaver finished the dam with " << tm.get_moves() << " branches!" << endl;
}


int main(int argc, char **argv)
{
    init_workers();
    worker();
}

