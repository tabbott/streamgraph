#include "process/Process.hpp"
#include "utility/io.hpp"
#include "utility/FdUtil.hpp"
#include "utility/TempFile.hpp"

#include <gtest/gtest.h>

#include <vector>
#include <string>

#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

namespace {
    typedef std::vector<std::string> VS;
}

// make sure that even when close on exec is set, we still get our fds
TEST(Process, remapFd_close_on_exec) {
    auto tmpfile = TempFile::create(TempFile::CLEANUP);
    std::string path = tmpfile->path();

    int out_fd = open(path.c_str(), O_CREAT|O_WRONLY, 0644);
    int flags;
    SYSCALL_RETRY_VAR(flags, fcntl(out_fd, F_GETFL, 0));
    ASSERT_GE(flags, 0);
    SYSCALL_RETRY(fcntl(out_fd, F_SETFL, flags | FD_CLOEXEC));
    SYSCALL_RETRY_VAR(flags, fcntl(out_fd, F_GETFL, 0));
    EXPECT_TRUE(flags & FD_CLOEXEC);

    auto proc = Process::create(VS{"bash", "-ec", "echo hello 1>&3"});

    proc->fd_map().add_existing_fd(3, out_fd);

    auto pid = proc->start();
    ASSERT_GT(pid, 0);
    EXPECT_TRUE(proc->finish());
    close(out_fd);

    auto msg = io::read_file(path);
    EXPECT_EQ("hello\n", msg);
}

// make sure that even when close on exec is set, we still get our fds
TEST(Process, exit_failure) {
    auto proc = Process::create(VS{"bash", "-ec", "exit 7"});
    ASSERT_GT(proc->start(), 0);
    EXPECT_FALSE(proc->finish());
    EXPECT_EQ(7, proc->exit_status());
}
