#include "ProcessManager.hpp"
#include "TimeUtil.hpp"

#include <boost/format.hpp>
#include <boost/unordered_map.hpp>

#include <cerrno>
#include <cstring>

using boost::format;

ProcessManager ProcessManager::instance_;

ProcessRecord::ProcessRecord()
    : pid(-1)
    , status(0)
    , rsrc()
    , start_time()
    , end_time()
{}

ProcessRecord::ProcessRecord(boost::shared_ptr<ProcessSpec> const& spec)
    : pid(-1)
    , status(0)
    , spec(spec)
    , rsrc()
    , start_time()
    , end_time()
{}

double ProcessRecord::wall_seconds() const {
    return tv2sec(end_time) - tv2sec(start_time);
}
class ProcessManager::Impl {
public:
    typedef boost::unordered_map<pid_t, ProcessRecord> ProcMap;

    pid_t launch(ProcessSpec::Ptr const& spec) {
        ProcessRecord rec(spec);
        rec.pid = fork();
        if (rec.pid == -1) {
            throw std::runtime_error("fork failed");
        }

        if (rec.pid) {
            gettimeofday(&rec.start_time, NULL);
            std::pair<ProcMap::iterator, bool> inserted = procs_.insert(
                std::make_pair(rec.pid, rec)
                );

            if (!inserted.second) {
                throw std::runtime_error(str(format(
                    "Duplicate pid %1%") % rec.pid));
            }
        }
        else {
            rec.spec->execute();
            // ^ calls exec or exits, will never return;
        }
        return rec.pid;
    }

    ProcessRecord waitpid(pid_t pid) {
        ProcMap::iterator iter = procs_.find(pid);
        if (iter == procs_.end()) {
            throw std::runtime_error(str(format(
                "Attempted to wait for unmanaged pid %1%"
                ) % pid));
        }

        pid_t cpid;
        ProcessRecord& rec = iter->second;
        while ((cpid = wait4(pid, &rec.status, 0, &rec.rsrc)) == -1 && errno == EINTR)
            ;

        if (cpid == -1) {
            throw std::runtime_error(str(format(
                "Failed waiting for child %1%, wait4: %2%"
                ) % pid % strerror(errno)));
        }

        gettimeofday(&rec.end_time, NULL);

        return rec;
    }

    std::vector<ProcessRecord> wait_all() {
        std::vector<ProcessRecord> rv;
        while (!procs_.empty()) {
            rv.push_back(this->waitpid(procs_.begin()->second.pid));
        }
        return rv;
    }

private:
    ProcMap procs_;
};

ProcessManager& ProcessManager::get_instance() {
    return instance_;
}

ProcessManager::ProcessManager()
    : impl_(new Impl)
{}

ProcessManager::~ProcessManager() {
    delete impl_;
}

pid_t ProcessManager::launch(ProcessSpec* process) {
    return impl_->launch(ProcessSpec::Ptr(process));
}

ProcessRecord ProcessManager::waitpid(pid_t pid) {
    return impl_->waitpid(pid);
}

std::vector<ProcessRecord> ProcessManager::wait_all() {
    return impl_->wait_all();
}
