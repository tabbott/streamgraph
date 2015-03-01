#pragma once

#include <boost/noncopyable.hpp>

#include "FdSource.hpp"

class FdMapping : public boost::noncopyable {
public:
    FdMapping();
    ~FdMapping();

    void add(int target_fd, FdSourceBase* source);

    void apply() const;

private:
    class Impl;
    Impl* impl_;
};
