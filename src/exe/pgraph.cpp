#include "process/ProcessGraph.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {
    typedef std::vector<std::string> VS;
}

int main(int argc, char** argv) {
    ProcessGraph pg;
    int lsid = pg.add(VS{"ls", "-al"});
    int trid = pg.add(VS{"tr" , "[a-z]", "[A-Z]"});
    int sedid = pg.add(VS{"sed", "s/BIN/BEEEEEN/g"});

    pg.connect(lsid, 1, trid, 0);
    pg.connect(trid, 1, sedid, 0);

    for (int i = 0; i < 3; ++i) {
        std::stringstream path;
        path << "/tmp/meow" << i << ".txt";
        int catid = pg.add(VS{"cat"});
        pg.process(catid)->fd_map().add_file(1, path.str(), O_CREAT|O_WRONLY, 0644);
        pg.connect(sedid, 1, catid, 0);
    }

    bool rv = pg.execute();
    std::cerr << "ok: " << std::boolalpha << rv << "\n";


    return 0;
}
