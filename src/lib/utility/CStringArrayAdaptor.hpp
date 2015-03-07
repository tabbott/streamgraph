#pragma once

#include <boost/noncopyable.hpp>

#include <cstring>
#include <vector>
#include <string>

struct CStringArrayAdaptor : public boost::noncopyable {
    typedef std::vector<std::string>::size_type vsize_type;

    CStringArrayAdaptor(std::vector<std::string> const& vstr)
        : arr_(vstr.size() + 1)
    {
        for (vsize_type i = 0; i < vstr.size(); ++i) {
            arr_[i] = strdup(vstr[i].c_str());
        }
        arr_.back() = 0;
    }

    ~CStringArrayAdaptor() {
        for (vsize_type i = 0; i < arr_.size(); ++i) {
            free(arr_[i]);
            arr_[i] = 0;
        }
    }

    std::vector<char*> const& arr() const {
        return arr_;
    }

private:
    std::vector<char*> arr_;
};
