#include "process/Process.hpp"
#include "process/ProcessGraph.hpp"
#include "utility/TimeUtil.hpp"

#include <glog/logging.h>

#include <iostream>
#include <stdexcept>
#include <vector>

#include <sys/wait.h>
#include <unistd.h>

namespace {

    void print_rusage(std::ostream& os, rusage const& rsrc) {
        os << "         user time: " << tv2sec(rsrc.ru_utime) << "s\n";
        os << "          sys time: " << tv2sec(rsrc.ru_stime) << "s\n";
        os << "           max rss: " << rsrc.ru_maxrss << "\n";
        os << " minor page faults: " << rsrc.ru_minflt << "\n";
        os << " major page faults: " << rsrc.ru_majflt << "\n";
        os << "      FS blocks in: " << rsrc.ru_inblock << "\n";
        os << "     FS blocks out: " << rsrc.ru_oublock << "\n";
        os << "  ctx switch (vol): " << rsrc.ru_nvcsw << "\n";
        os << "ctx switch (invol): " << rsrc.ru_nivcsw << "\n";

        // these aren't used on linux
        /*
         *os << "             ixrss: " << rsrc.ru_ixrss << "\n";
         *os << "             idrss: " << rsrc.ru_idrss << "\n";
         *os << "             isrss: " << rsrc.ru_idrss << "\n";
         *os << "             swaps: " << rsrc.ru_nswap << "\n";
         *os << " IPC msg sent (io): " << rsrc.ru_msgsnd << "\n";
         *os << " IPC msg recv (io): " << rsrc.ru_msgrcv << "\n";
         *os << " #signals received: " << rsrc.ru_nsignals << "\n";
         */
    }
}

int main(int argc, char* argv[]) {
    google::InitGoogleLogging(argv[0]);

    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <program> [args...]\n";
        return 1;
    }

    try {
        std::vector<std::string> args(&argv[1], &argv[argc]);
        Process::Ptr proc = Process::create(args);

        pid_t pid = proc->start();
        std::cerr << "Started process with pid " << pid << "\n";
        bool ok = proc->finish();
        std::cerr << "Status: " << ok << " exit status: " << proc->raw_status() << "\n";
        print_rusage(std::cerr, proc->resource_usage());
    }
    catch (std::exception const& ex) {
        std::cerr << "ERROR: " << ex.what() << "\n";
    }

    return 0;
}
