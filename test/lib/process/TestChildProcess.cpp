#include "process/ChildProcess.hpp"
#include "process/ChildProcessResult.hpp"
#include "process/ExecWrapper.hpp"
#include "utility/io.hpp"
#include "utility/Syscall.hpp"
#include "utility/TempFile.hpp"

#include "ProcessIoHelpers.hpp"
#include "TestConfig.hpp"

#include <gtest/gtest.h>

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

namespace {
    typedef std::vector<std::string> VS;

    struct ForgotToCallExit : IoHelperBase {
        void operator()() {}
    };
}

// make sure that even when close on exec is set, we still get our fds
TEST(ChildProcess, remapFd_close_on_exec) {
    auto tmpfile = TempFile::create(TempFile::CLEANUP);
    std::string path = tmpfile->path();

    int out_fd = open(path.c_str(), O_CREAT|O_WRONLY, 0644);
    int flags;
    SYSCALL_RETRY_VAR(flags, fcntl(out_fd, F_GETFL, 0));
    ASSERT_GE(flags, 0);
    SYSCALL_RETRY(fcntl(out_fd, F_SETFL, flags | FD_CLOEXEC));
    SYSCALL_RETRY_VAR(flags, fcntl(out_fd, F_GETFL, 0));
    EXPECT_TRUE(flags & FD_CLOEXEC);

    FdWriter writer(3, "hello");
    auto proc = ChildProcess::create("test", writer);

    proc->fd_map().add_existing_fd(3, out_fd);

    auto pid = proc->start();
    ASSERT_GT(pid, 0);
    EXPECT_TRUE(proc->finish());
    close(out_fd);

    auto msg = io::read_file(path);
    EXPECT_EQ("hello", msg);
}

// make sure that even when close on exec is set, we still get our fds
TEST(ChildProcess, exit_failure) {
    ExitBot ebot(2);
    auto proc = ChildProcess::create("test", ebot);
    ASSERT_GT(proc->start(), 0);
    EXPECT_FALSE(proc->finish());
    EXPECT_THROW(proc->set_result(proc->result()), std::runtime_error);
    EXPECT_EQ(2, proc->exit_status());
}

// make sure that when a caller forgets to call exit in their work
// function, exit(1) is called by default
TEST(ChildProcess, forgot_exit) {
    ForgotToCallExit obj;
    auto proc = ChildProcess::create("test", obj);
    ASSERT_GT(proc->start(), 0);
    EXPECT_FALSE(proc->finish());
    EXPECT_THROW(proc->set_result(proc->result()), std::runtime_error);
    EXPECT_EQ(1, proc->exit_status());
}

TEST(ChildProcess, exec_wrapper) {
    // what executable can we shell out to?
    // well, we know that we at least have cmake!
    // we can use cmake -E echo_append "hello" to echo stuff w/o \n
    std::string message = "hello";
    VS args{TestConfig::CMAKE_EXE_PATH, "-E", "echo_append", message};
    ExecWrapper wrapper(args);

    auto tmpfile = TempFile::create(TempFile::CLEANUP);
    std::string path = tmpfile->path();

    int out_fd = open(path.c_str(), O_CREAT|O_WRONLY, 0644);
    auto proc = ChildProcess::create("test", wrapper);

    proc->fd_map().add_existing_fd(1, out_fd);

    auto pid = proc->start();
    ASSERT_GT(pid, 0);
    EXPECT_TRUE(proc->finish());
    close(out_fd);

    auto data = io::read_file(path);
    EXPECT_EQ(message, data);
}

TEST(ChildProcess, set_result) {
    ExitBot ebot(1);
    auto proc = ChildProcess::create("test", ebot);

    EXPECT_EQ(eNEW, proc->state());

    ChildProcessResult result{};
    result.status = 256;
    result.rsrc.ru_utime.tv_sec = 5;
    result.rsrc.ru_utime.tv_usec = 50;

    EXPECT_THROW(proc->set_result(result), std::runtime_error);

    proc->start();

    // Now we do an external wait4 instead of calling finish
    SYSCALL_RETRY_VAR(result.pid, wait4(proc->pid(), &result.status, 0, &result.rsrc));
    ASSERT_EQ(proc->pid(), result.pid);

    // try setting result for unrelated pid
    ++result.pid;
    EXPECT_THROW(proc->set_result(result), std::runtime_error);

    // now do it correctly
    result.pid = proc->pid();
    EXPECT_EQ(eRUNNING, proc->state());
    ASSERT_NO_THROW(proc->set_result(result));
    EXPECT_EQ(eCOMPLETE, proc->state());
    ASSERT_EQ(result, proc->result());

    // trying to set result again should be an error
    EXPECT_THROW(proc->set_result(result), std::runtime_error);

    // calling finish should be an error
    EXPECT_THROW(proc->finish(), std::runtime_error);
}
