#include <gtest/gtest.h>
#include <glog/logging.h>

int main(int argc, char** argv) {
    // disable logging in tests
    FLAGS_logtostderr = true;
    FLAGS_minloglevel = 5;

    ::google::InitGoogleLogging(argv[0]);

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
