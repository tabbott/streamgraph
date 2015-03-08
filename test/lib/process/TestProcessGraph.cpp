#include "process/ProcessGraph.hpp"

#include <gtest/gtest.h>

#include <vector>
#include <string>

namespace {
    typedef std::vector<std::string> VS;
}

TEST(ProcessGraph, invalid_indegree) {
    ProcessGraph g;
    int hi = g.add(VS{"bash", "-ec", "echo hi"});
    int there = g.add(VS{"bash", "-ec", "echo there"});
    int tr = g.add(VS{"tr", "[a-z]", "[A-Z]"});

    g.connect(hi, 1, tr, 0);
    EXPECT_THROW(g.connect(there, 1, tr, 0), std::runtime_error);
}

TEST(ProcessGraph, fanout) {
    ProcessGraph g;
    int hi = g.add(VS{"bash", "-ec", "echo hi"});
    int cat1 = g.add(VS{"cat"});
    int cat2 = g.add(VS{"cat"});

    g.connect(hi, 1, cat1, 0);
    g.connect(hi, 1, cat2, 0);
    EXPECT_TRUE(g.execute());
}
