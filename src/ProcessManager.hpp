#pragma once

#include <ProcessSpec.hpp>

#include <boost/shared_ptr.hpp>

#include <vector>

#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

struct ProcessRecord {
    ProcessRecord();
    ProcessRecord(ProcessSpec::Ptr const& spec);

    pid_t pid;
    int status;
    boost::shared_ptr<ProcessSpec> spec;
    rusage rsrc;
    timeval start_time;
    timeval end_time;

    double wall_seconds() const;
};

class ProcessManager {
public:
    static ProcessManager& get_instance();

    pid_t launch(ProcessSpec* process);

    ProcessRecord waitpid(pid_t pid);
    std::vector<ProcessRecord> wait_all();

private:
    ProcessManager();
    ~ProcessManager();

    class Impl;
    Impl* impl_;

    static ProcessManager instance_;
};
