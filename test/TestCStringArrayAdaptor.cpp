#include "CStringArrayAdaptor.hpp"

#include <gtest/gtest.h>

#include <vector>
#include <string>

TEST(CStringArrayAdaptor, adapt) {
    std::vector<std::string> vec;
    vec.push_back("ls");
    vec.push_back("-alrt");

    CStringArrayAdaptor arr(vec);
    EXPECT_EQ(3u, arr.arr().size());
    EXPECT_STREQ(arr.arr()[0], "ls");
    EXPECT_STREQ(arr.arr()[1], "-alrt");
    EXPECT_EQ(0, arr.arr()[2]);
}
