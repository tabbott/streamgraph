#include "process/ProcessGraph.hpp"

#include <iostream>
#include <sstream>

int main(int argc, char** argv) {
    std::vector<std::string> args;
    args.push_back("ls");
    args.push_back("-al");

    ProcessGraph pg;
    int lsid = pg.add(args);

    args.clear();
    args.push_back("tr");
    args.push_back("[a-z]");
    args.push_back("[A-Z]");
    int trid = pg.add(args);

    args.clear();
    args.push_back("sed");
    args.push_back("s/BIN/BEEEEEN/g");
    int sedid = pg.add(args);

    pg.connect(lsid, 1, trid, 0);
    pg.connect(trid, 1, sedid, 0);

    args.clear();
    args.push_back("cat");
    for (int i = 0; i < 3; ++i) {
        std::stringstream path;
        path << "/tmp/meow" << i << ".txt";
        int catid = pg.add(args);
        pg.process(catid)->fd_map().add_file(1, path.str(), O_CREAT|O_WRONLY, 0644);
        pg.connect(sedid, 1, catid, 0);
    }

    bool rv = pg.execute();
    std::cerr << "ok: " << std::boolalpha << rv << "\n";


    return 0;
}
