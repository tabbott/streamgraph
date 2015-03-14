#include "process/ProcessGraph.hpp"
#include "utility/TempFile.hpp"
#include "utility/io.hpp"

#include "TestConfig.hpp"

#include <gtest/gtest.h>

#include <vector>
#include <string>

namespace {
    typedef std::vector<std::string> VS;
}

TEST(ProcessGraph, invalid_indegree) {
    ProcessGraph g;
    int hi = g.add("p1", VS{TestConfig::CMAKE_EXE_PATH, "-E", "echo", "hi"});
    int there = g.add("p1", VS{TestConfig::CMAKE_EXE_PATH, "-E", "echo", "there"});
    int tr = g.add("p3", VS{"tr", "[a-z]", "[A-Z]"});

    g.connect(hi, 1, tr, 0);
    EXPECT_THROW(g.connect(there, 1, tr, 0), std::runtime_error);
}

TEST(ProcessGraph, fanout) {
    auto tmpfile1 = TempFile::create(TempFile::CLEANUP);
    auto tmpfile2 = TempFile::create(TempFile::CLEANUP);
    ProcessGraph g;
    int hi = g.add("p1", VS{TestConfig::CMAKE_EXE_PATH, "-E", "echo", "hi"});
    int cat1 = g.add("p2", VS{"cat"});
    int cat2 = g.add("p3", VS{"cat"});

    g.connect(hi, 1, cat1, 0);
    g.connect(hi, 1, cat2, 0);
    g.connect_output_file(cat1, 1, tmpfile1->path());
    g.connect_output_file(cat2, 1, tmpfile2->path());
    EXPECT_TRUE(g.execute());

    EXPECT_EQ("hi\n", io::read_file(tmpfile1->path()));
    EXPECT_EQ("hi\n", io::read_file(tmpfile2->path()));
}
