#include "FdMapping.hpp"

#include <boost/format.hpp>
#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <iostream>

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
    typedef boost::unordered_map<int, FdSourceBasePtr> MapType;
    typedef boost::unordered_map<int, int> FdMapType;

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
            int new_fd = dup(old_fd);
            fd_map[iter->first] = new_fd;
        }

        for (FdMapType::const_iterator iter = fd_map.begin(); iter != fd_map.end(); ++iter) {
            dup2(iter->second, iter->first);
        }

        for (FdMapType::const_iterator iter = fd_map.begin(); iter != fd_map.end(); ++iter) {
            if (mapping_.count(iter->second))
                continue;

            close(iter->second);
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

void FdMapping::add(int target_fd, FdSourceBase* source) {
    impl_->add(target_fd, FdSourceBasePtr(source));
}

void FdMapping::apply() const {
    impl_->apply();
}
