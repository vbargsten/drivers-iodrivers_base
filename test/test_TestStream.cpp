#include <boost/test/unit_test.hpp>

#include <iodrivers_base/Driver.hpp>
#include <iodrivers_base/FixtureBoostTest.hpp>
#include <iodrivers_base/Exceptions.hpp>

using namespace iodrivers_base;
using namespace std;

BOOST_AUTO_TEST_SUITE(TestStreamSuite)

struct Driver : iodrivers_base::Driver
{
public:
    Driver()
        : iodrivers_base::Driver(100) {}

    int extractPacket(uint8_t const* buffer, size_t size) const
    {
        return size;
    }
};

struct Fixture : iodrivers_base::Fixture<Driver>
{
    Fixture()
    {
        driver.openURI("test://");
    }
};


BOOST_FIXTURE_TEST_CASE(it_sends_data_to_the_Driver, Fixture)
{
    uint8_t data[] = { 0, 1, 2, 3 };
    pushDataToDriver(data, data + 4);
    vector<uint8_t> buffer = readPacket();
    BOOST_REQUIRE(buffer == vector<uint8_t>(data, data + 4));
}

BOOST_FIXTURE_TEST_CASE(it_accumulates_bytes_not_read_by_the_driver, Fixture)
{
    uint8_t data[] = { 0, 1, 2, 3 };
    pushDataToDriver(data, data + 2);
    pushDataToDriver(data + 2, data + 4);
    vector<uint8_t> buffer = readPacket();
    BOOST_REQUIRE(buffer == vector<uint8_t>(data, data + 4));
}

BOOST_FIXTURE_TEST_CASE(it_does_not_repeat_data_already_read_by_the_Driver, Fixture)
{
    uint8_t data[] = { 0, 1, 2, 3 };
    pushDataToDriver(data, data + 2);
    readPacket();
    pushDataToDriver(data + 2, data + 4);
    vector<uint8_t> buffer = readPacket();
    BOOST_REQUIRE(buffer == vector<uint8_t>(data + 2, data + 4));
}

BOOST_FIXTURE_TEST_CASE(it_times_out_instantly, Fixture)
{
    BOOST_REQUIRE_THROW(readPacket(), TimeoutError);
}

BOOST_FIXTURE_TEST_CASE(it_gives_access_to_the_bytes_sent_by_the_driver, Fixture)
{
    uint8_t data[] = { 0, 1, 2, 3 };
    writePacket(data, 4);
    std::vector<uint8_t> received = readDataFromDriver();
    BOOST_REQUIRE(received == vector<uint8_t>(data, data + 4));
}

BOOST_FIXTURE_TEST_CASE(it_accumulates_unread_bytes, Fixture)
{
    uint8_t data[] = { 0, 1, 2, 3 };
    writePacket(data, 2);
    writePacket(data + 2, 2);
    std::vector<uint8_t> received = readDataFromDriver();
    BOOST_REQUIRE(received == vector<uint8_t>(data, data + 4));
}

BOOST_FIXTURE_TEST_CASE(it_does_not_repeat_data_already_read_from_the_device, Fixture)
{
    uint8_t data[] = { 0, 1, 2, 3 };
    writePacket(data, 2);
    readDataFromDriver();
    writePacket(data + 2, 2);
    std::vector<uint8_t> received = readDataFromDriver();
    BOOST_REQUIRE(received == vector<uint8_t>(data + 2, data + 4));
}

BOOST_FIXTURE_TEST_CASE(it_matches_expecation_with_data_sent_to_device, Fixture)
{
    IODRIVERS_BASE_MOCK();
    uint8_t exp[] = { 0, 1, 2, 3 };
    uint8_t rep[] = { 3, 2, 1, 0 };
    EXPECT_REPLY(vector<uint8_t>(exp, exp + 4),vector<uint8_t>(rep, rep + 4));
    writePacket(exp,4);
    vector<uint8_t> received = readPacket();
    BOOST_REQUIRE(received == vector<uint8_t>(rep,rep+4));
}

BOOST_FIXTURE_TEST_CASE(it_fails_expecation_with_data_sent_to_device, Fixture)
{
    IODRIVERS_BASE_MOCK();
    uint8_t exp[] = { 0, 1, 2, 3 };
    uint8_t msg[] = { 0, 1, 2, 4 };
    uint8_t rep[] = { 3, 2, 1, 0 };
    EXPECT_REPLY(vector<uint8_t>(exp, exp + 4),vector<uint8_t>(rep, rep + 4));
    BOOST_REQUIRE_THROW(writePacket(msg,4), std::invalid_argument); 
    
}

BOOST_FIXTURE_TEST_CASE(it_tries_to_set_expectation_without_calling_mock_context, Fixture)
{
    uint8_t exp[] = { 0, 1, 2, 3 };
    uint8_t rep[] = { 3, 2, 1, 0 };
    BOOST_REQUIRE_THROW(EXPECT_REPLY(vector<uint8_t>(exp, exp + 4),vector<uint8_t>(rep, rep + 4)), MockContextException);
}

BOOST_FIXTURE_TEST_CASE(it_matches_more_than_one_expecation, Fixture)
{
    IODRIVERS_BASE_MOCK();
    uint8_t exp1[] = { 0, 1, 2, 3 };
    uint8_t rep1[] = { 3, 2, 1, 0 };
    uint8_t exp2[] = { 0, 1, 2, 3, 4 };
    uint8_t rep2[] = { 4, 3, 2, 1, 0 };
    EXPECT_REPLY(vector<uint8_t>(exp1, exp1 + 4),vector<uint8_t>(rep1, rep1 + 4));
    EXPECT_REPLY(vector<uint8_t>(exp2, exp2 + 5),vector<uint8_t>(rep2, rep2 + 5));
    writePacket(exp1,4);
    vector<uint8_t> received_1 = readPacket();
    BOOST_REQUIRE(received_1 == vector<uint8_t>(rep1,rep1+4));
    
    writePacket(exp2,5);
    vector<uint8_t> received_2 = readPacket();
    for(size_t i =0; i<received_2.size(); i++)
    BOOST_REQUIRE(received_2 == vector<uint8_t>(rep2,rep2+5));
}

BOOST_FIXTURE_TEST_CASE(it_does_not_matches_all_expecations, Fixture)
{
    IODRIVERS_BASE_MOCK();
    uint8_t exp1[] = { 0, 1, 2, 3 };
    uint8_t rep1[] = { 3, 2, 1, 0 };
    uint8_t exp2[] = { 0, 1, 2, 3, 4 };
    uint8_t rep2[] = { 4, 3, 2, 1, 0 };
    EXPECT_REPLY(vector<uint8_t>(exp1, exp1 + 4),vector<uint8_t>(rep1, rep1 + 4));
    EXPECT_REPLY(vector<uint8_t>(exp2, exp2 + 5),vector<uint8_t>(rep2, rep2 + 5));
    writePacket(exp1,4);
    vector<uint8_t> received_1 = readPacket();
    BOOST_REQUIRE(received_1 == vector<uint8_t>(rep1,rep1+4));
    BOOST_REQUIRE_THROW(validateExpectationsAreEmpty(), TestEndsWithExpectationsLeftException);
    clearExpectations();
}

BOOST_FIXTURE_TEST_CASE(it_sends_more_messages_than_expecations_set, Fixture)
{
    IODRIVERS_BASE_MOCK();
    uint8_t exp1[] = { 0, 1, 2, 3 };
    uint8_t rep1[] = { 3, 2, 1, 0 };
    uint8_t exp2[] = { 0, 1, 2, 3, 4 };
    EXPECT_REPLY(vector<uint8_t>(exp1, exp1 + 4),vector<uint8_t>(rep1, rep1 + 4));
    writePacket(exp1,4);
    vector<uint8_t> received_1 = readPacket();
    BOOST_REQUIRE(received_1 == vector<uint8_t>(rep1,rep1+4));
    
    BOOST_REQUIRE_THROW(writePacket(exp2,5),std::runtime_error);
}
BOOST_AUTO_TEST_SUITE_END()

