#include "process/ProcessGroup.hpp"
#include "utility/TempFile.hpp"
#include "utility/io.hpp"

#include <gtest/gtest.h>

#include <sstream>

namespace {
    typedef std::vector<std::string> VS;
}

TEST(ProcessGroup, run) {
    unsigned nprocs = 10;
    auto tmpdir = TempDir::create(TempDir::CLEANUP);

    ProcessGroup pgroup;

    for (unsigned i = 0; i < nprocs; ++i) {
        std::stringstream cmd;
        cmd << "echo " << i << " > " << tmpdir->path() << "/" << i << ".txt";

        auto proc = Process::create(VS{"bash", "-ec", cmd.str()});
        pgroup.add(proc);
    }

    EXPECT_EQ(nprocs, pgroup.processes().size());
    pgroup.start();
    EXPECT_TRUE(pgroup.finish());

    for (unsigned i = 0; i < nprocs; ++i) {
        std::stringstream path;
        path << tmpdir->path() << "/" << i << ".txt";
        std::stringstream expected;
        expected << i << "\n";

        EXPECT_EQ(expected.str(), io::read_file(path.str()));

        auto const& proc = pgroup.processes()[i];
        EXPECT_TRUE(proc->succeeded());
        EXPECT_EQ(0, proc->raw_status());
    }
}

TEST(ProcessGroup, signal_all) {
    unsigned nprocs = 10;
    int signum = 2;

    auto tmpdir = TempDir::create(TempDir::CLEANUP);

    ProcessGroup pgroup;

    for (unsigned i = 0; i < nprocs; ++i) {
        std::stringstream cmd;
        cmd << "echo " << i << " > " << tmpdir->path() << "/" << i << ".txt";

        auto proc = Process::create(VS{"bash", "-ec", "sleep 10"});
        pgroup.add(proc);
    }

    EXPECT_EQ(nprocs, pgroup.processes().size());
    pgroup.start();
    pgroup.signal_all(signum);
    EXPECT_FALSE(pgroup.finish());

    for (unsigned i = 0; i < nprocs; ++i) {
        auto const& proc = pgroup.processes()[i];
        EXPECT_FALSE(proc->succeeded());
        EXPECT_EQ(signum, proc->exit_signal());
    }
}
