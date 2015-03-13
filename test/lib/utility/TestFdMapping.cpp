#include "utility/FdMapping.hpp"
#include "utility/Syscall.hpp"
#include "utility/TempFile.hpp"
#include "utility/io.hpp"

#include <glog/logging.h>
#include <gtest/gtest.h>

#include <cstdlib>
#include <sstream>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

TEST(FdMapping, redirect_stdout_stderr) {
    int outfd = fileno(stdout);
    int errfd = fileno(stderr);

    auto tmp_out = TempFile::create(TempFile::CLEANUP);
    auto tmp_err = TempFile::create(TempFile::CLEANUP);
    auto const& stdout_path = tmp_out->path();
    auto const& stderr_path = tmp_err->path();

    auto pid = fork();
    ASSERT_GE(pid, 0);

    FdMapping redirect;
    redirect.add_file(outfd, stdout_path, O_CREAT|O_WRONLY|O_TRUNC, 0644);
    redirect.add_file(errfd, stderr_path, O_CREAT|O_WRONLY|O_TRUNC, 0644);

    FdMapping swap_std;
    swap_std.add_existing_fd(outfd, errfd);
    swap_std.add_existing_fd(errfd, outfd);

    if (pid == 0) {
        redirect.apply();
        std::cout << "stdout\n";
        std::cerr << "stderr\n";

        swap_std.apply();
        std::cout << "stdout\n";
        std::cerr << "stderr\n";
        std::exit(0);
    }
    else {
        int status = -1;
        pid_t caught = -1;
        SYSCALL_RETRY_VAR(caught, waitpid(pid, &status, 0));
        ASSERT_EQ(pid, caught);
        EXPECT_EQ(0, status);
    }

    EXPECT_EQ("stdout\nstderr\n", io::read_file(stdout_path));
    EXPECT_EQ("stderr\nstdout\n", io::read_file(stderr_path));
}

TEST(FdMapping, exceptions) {
    FdMapping redirect;
    redirect.add_existing_fd(1, 2);
    EXPECT_THROW(redirect.add_existing_fd(1, 3), std::runtime_error);
}
