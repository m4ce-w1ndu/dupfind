#include <gtest/gtest.h>
#include "test_fixtures.hpp"

class GlobalTestEnvironment : public ::testing::Environment {
public:
    void SetUp() override
    {
        dupfind::test::TestFixtures::get_instance().initialize();
    }

    void TearDown() override
    {
        dupfind::test::TestFixtures::get_instance().cleanup();
    }
};

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::AddGlobalTestEnvironment(new GlobalTestEnvironment);
    return RUN_ALL_TESTS();
}
