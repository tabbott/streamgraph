#include "FdMapping.hpp"
#include "io.hpp"

#include <gtest/gtest.h>

#include <cstdlib>
#include <sstream>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

TEST(FdMapping, redirect_stdout_stderr) {

    int outfd = fileno(stdout);
    int errfd = fileno(stderr);

    int save_out = dup(outfd);
    int save_err = dup(errfd);

    char const* stdout_path = "/tmp/stdout.txt";
    char const* stderr_path = "/tmp/stderr.txt";

    FdMapping redirect;
    redirect.add(outfd, new FileFdSource(stdout_path, O_CREAT|O_WRONLY|O_TRUNC, 0644));
    redirect.add(errfd, new FileFdSource(stderr_path, O_CREAT|O_WRONLY|O_TRUNC, 0644));
    redirect.apply();
    std::cout << "stdout\n";
    std::cerr << "stderr\n";

    FdMapping swap_std;
    swap_std.add(outfd, new FdSource(errfd));
    swap_std.add(errfd, new FdSource(outfd));
    swap_std.apply();
    std::cout << "stdout\n";
    std::cerr << "stderr\n";

    FdMapping restore;
    restore.add(outfd, new FdSource(save_out));
    restore.add(errfd, new FdSource(save_err));
    restore.apply();

    EXPECT_EQ(io::read_file(stdout_path), "stdout\nstderr\n");
    EXPECT_EQ(io::read_file(stderr_path), "stderr\nstdout\n");
}

TEST(FdMapping, exceptions) {
    FdMapping redirect;
    redirect.add(1, new FdSource(2));
    EXPECT_THROW(redirect.add(1, new FdSource(3)), std::runtime_error);
}
