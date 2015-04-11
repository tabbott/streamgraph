#pragma once

#include <cstdio>
#include <cstdlib>
#include <string>

#include <unistd.h>

namespace {
    struct IoHelperBase {
        std::vector<std::string> const& args() const {
            return args_;
        };
        std::vector<std::string> args_;
    };


    // no lambdas allowed in gcc 4.4 :(
    struct FdWriter : IoHelperBase {
        FdWriter(int fd, std::string const& message)
            : fd(fd)
            , message(message)
        {}

        // write message on file descriptor fd
        void operator()() const {
            FILE* fp = fdopen(fd, "w");
            auto bytes_written = fwrite(message.data(), 1, message.size(), fp);
            fclose(fp);
            std::exit(bytes_written != message.size());
        }

        int fd;
        std::string message;
    };

    struct ExitBot : IoHelperBase {
        explicit ExitBot(int status)
            : status(status)
        {}

        void operator()() const {
            std::exit(status);
        }

        int status;
    };

    struct Sleeper : IoHelperBase {
        explicit Sleeper(int seconds)
            : seconds(seconds)
        {}

        void operator()() const {
            sleep(seconds);
        }

        int seconds;
    };
}
