#include "ProcessManager.hpp"
#include "TimeUtil.hpp"

#include <iostream>
#include <vector>

#include <sys/wait.h>
#include <unistd.h>

namespace {


    void print_rusage(std::ostream& os, ProcessRecord const& rec) {
        rusage const& rsrc = rec.rsrc;
        os << "         user time: " << tv2sec(rsrc.ru_utime) << "s\n";
        os << "          sys time: " << tv2sec(rsrc.ru_stime) << "s\n";
        os << "         wall time: " << rec.wall_seconds() << "s\n";
        os << "           max rss: " << rsrc.ru_maxrss << "\n";
        os << "             ixrss: " << rsrc.ru_ixrss << "\n";
        os << "             idrss: " << rsrc.ru_idrss << "\n";
        os << "             isrss: " << rsrc.ru_idrss << "\n";
        os << " minor page faults: " << rsrc.ru_minflt << "\n";
        os << " major page faults: " << rsrc.ru_majflt << "\n";
        os << "             swaps: " << rsrc.ru_nswap << "\n";
        os << "    FS inputs (io): " << rsrc.ru_inblock << "\n";
        os << "   FS outputs (io): " << rsrc.ru_oublock << "\n";
        os << " IPC msg sent (io): " << rsrc.ru_msgsnd << "\n";
        os << " IPC msg recv (io): " << rsrc.ru_msgrcv << "\n";
        os << " #signals received: " << rsrc.ru_nsignals << "\n";
        os << "  ctx switch (vol): " << rsrc.ru_nvcsw << "\n";
        os << "ctx switch (invol): " << rsrc.ru_nivcsw << "\n";
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <program> [args...]\n";
        return 1;
    }

    ProcessManager& mgr = ProcessManager::get_instance();
    FdMapping fdmap;

    std::vector<std::string> args(&argv[1], &argv[argc]);
    ProcessSpec* spec = new ProcessSpec(args, fdmap);

    pid_t pid = mgr.launch(spec);
    ProcessRecord rec = mgr.waitpid(pid);

    std::cout << "status: " << rec.status << "\n";
    print_rusage(std::cout, rec);

    return 0;
}
