#include "utility/FdMapping.hpp"
#include "utility/FdSource.hpp"
#include "utility/FdUtil.hpp"

#include <glog/logging.h>

#include <boost/format.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <iostream>

#include <map>

#include <unistd.h>

using boost::format;

namespace {
    // Can use this typedef instead when c++11 support arrives
    // dont forget to #include <memory>
    //typedef std::unique_ptr<FdSourceBase> FdSourceBasePtr;
    typedef boost::shared_ptr<FdSourceBase> FdSourceBasePtr;
}


class FdMapping::Impl {
public:
    /*
     *typedef boost::unordered_map<int, FdSourceBasePtr> MapType;
     *typedef boost::unordered_map<int, int> FdMapType;
     */
    typedef std::map<int, FdSourceBasePtr> MapType;
    typedef std::map<int, int> FdMapType;


    void add(int target_fd, FdSourceBasePtr const& source) {
        std::pair<MapType::iterator, bool> inserted = mapping_.insert(
            std::make_pair(target_fd, source)
            );

        if (!inserted.second) {
            throw std::runtime_error(str(format(
                "Attempted to remap existing fd %1%"
                ) % target_fd));
        }
    }

    void apply() const {
        apply_naive_and_wasteful();
    }

    void apply_naive_and_wasteful() const {
        FdMapType fd_map;
        for (MapType::const_iterator iter = mapping_.begin(); iter != mapping_.end(); ++iter) {
            int old_fd = iter->second->get_fd();
            int new_fd = -1;
            SYSCALL_RETRY_VAR(new_fd, dup(old_fd));
            // FIXME new_fd = -1 && die???
            fd_map[iter->first] = new_fd;
            LOG(INFO) << "temporary dup(" << old_fd << ") -> " << new_fd;
        }

        for (FdMapType::const_iterator iter = fd_map.begin(); iter != fd_map.end(); ++iter) {
            int status;
            SYSCALL_RETRY_VAR(status, dup2(iter->second, iter->first));
            LOG(INFO) << "final dup2(" << iter->second << ", " <<  iter->first
                << ") -> " << status;
        }

        int nfds = getdtablesize();
        for (int i = 3; i < nfds; ++i) {
            if (!mapping_.count(i)) {
                int status;
                SYSCALL_RETRY_VAR(status, close(i));
                LOG_IF(INFO, status == 0) << "closed unused fd " << i;
            }
        }
    }

private:
    MapType mapping_;
};

FdMapping::FdMapping()
    : impl_(new Impl)
{}

FdMapping::~FdMapping() {
    delete impl_;
}

/*
 *void FdMapping::add(int target_fd, FdSourceBase* source) {
 *    impl_->add(target_fd, FdSourceBasePtr(source));
 *}
 */

void FdMapping::add_existing_fd(int target_fd, int source_fd) {
    impl_->add(target_fd, FdSourceBasePtr(new FdSource(source_fd)));
}

void FdMapping::add_file(int target_fd, std::string const& path, int flags, mode_t mode) {
    impl_->add(target_fd, FdSourceBasePtr(new FileFdSource(path, flags, mode)));
}

    void add_existing_fd(int target_fd, FdSourceBase* source);
    void add_file(int target_fd, std::string const& path);


void FdMapping::apply() const {
    impl_->apply();
}
