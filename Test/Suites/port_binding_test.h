#pragma once

#include <gtest/gtest.h>
#include <boost/asio/ip/tcp.hpp>

namespace test {
  class PortBindingTest : public ::testing::Test
  {
  public:
    PortBindingTest();

    virtual ~PortBindingTest() override;

    void SetUp() override;

    void TearDown() override;

    static ::testing::AssertionResult
    assert_port_is_in_use(const boost::asio::ip::tcp::endpoint& endpoint);

  };
}

