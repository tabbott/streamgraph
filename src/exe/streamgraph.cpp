#include "process/GraphXml.hpp"
#include "process/ProcessGraph.hpp"
#include "ui/RunGraphCommand.hpp"
#include "ui/App.hpp"

#include <glog/logging.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

int main(int argc, char** argv) {
    try {
        App app;
        app.exec(argc, argv);
        return 0;
    }
    catch (CmdlineHelpException const& ex) {
        std::cout << ex.what() << "\n";
        return 0;
    }
    catch (std::exception const& ex) {
        std::cerr << "ERROR: " << ex.what() << "\n";
        return 1;
    }
}
