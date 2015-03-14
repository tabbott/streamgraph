#include "ExecWrapper.hpp"
#include "utility/CStringArrayAdaptor.hpp"

#include <glog/logging.h>

#include <utility>

#include <unistd.h>


ExecWrapper::ExecWrapper(std::vector<std::string> args)
    : args_(std::move(args))
{}

void ExecWrapper::operator()() {
    CStringArrayAdaptor argv(args_);
    execvp(argv.arr()[0], argv.arr().data());
    LOG(FATAL) << "Failed to execute " << argv.arr()[0] << ": " << strerror(errno);
}
