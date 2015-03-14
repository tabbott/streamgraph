#include "process/ProcessGroup.hpp"
#include "utility/TempFile.hpp"
#include "utility/io.hpp"

#include "ProcessIoHelpers.hpp"

#include <boost/lexical_cast.hpp>
#include <boost/filesystem/path.hpp>

#include <gtest/gtest.h>

#include <sstream>

#include <signal.h>

namespace bfs = boost::filesystem;

namespace {
    typedef std::vector<std::string> VS;
}

TEST(ProcessGroup, run) {
    unsigned nprocs = 10;
    auto tmpdir = TempDir::create(TempDir::CLEANUP);

    ProcessGroup pgroup;

    for (unsigned i = 0; i < nprocs; ++i) {
        bfs::path path(tmpdir->path());
        path /= boost::lexical_cast<std::string>(i) + ".txt";

        FdWriter writer(1, path.string());
        auto proc = ChildProcess::create("test", writer);
        proc->fd_map().add_file(1, path.string(), O_WRONLY|O_CREAT, 0644);
        pgroup.add(proc);
    }

    EXPECT_EQ(nprocs, pgroup.processes().size());
    pgroup.start();
    EXPECT_TRUE(pgroup.finish());

    for (unsigned i = 0; i < nprocs; ++i) {
        bfs::path path(tmpdir->path());
        path /= boost::lexical_cast<std::string>(i) + ".txt";

        EXPECT_EQ(path.string(), io::read_file(path.string()));

        auto const& proc = pgroup.processes()[i];
        EXPECT_TRUE(proc->succeeded());
        EXPECT_EQ(0, proc->raw_status());
    }
}

class TestProcessGroupSignals : public ::testing::TestWithParam<int> {
};

TEST_P(TestProcessGroupSignals, signal_all) {
    unsigned nprocs = 10;

    int sig = GetParam();
    ProcessGroup pgroup;

    for (unsigned i = 0; i < nprocs; ++i) {
        Sleeper sleepy(10);
        auto proc = ChildProcess::create("test", sleepy);
        pgroup.add(proc);
    }

    EXPECT_EQ(nprocs, pgroup.processes().size());
    pgroup.start();
    pgroup.signal_all(sig);
    EXPECT_FALSE(pgroup.finish());

    for (unsigned i = 0; i < nprocs; ++i) {
        auto const& proc = pgroup.processes()[i];
        EXPECT_FALSE(proc->succeeded());
        EXPECT_EQ(sig, proc->exit_signal());
    }
}

// All signals that terminate the process by default (excluding those that
// may produce a core)
INSTANTIATE_TEST_CASE_P(ProcessGroupSignals, TestProcessGroupSignals,
        ::testing::Values(
            SIGHUP, SIGINT, SIGKILL, SIGPIPE, SIGALRM,
            SIGTERM, SIGUSR1, SIGUSR2));
