#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <Test/Suites/http_server_test.h>
#include <Test/consts.h>
#include <LibHttp/http_server.h>
#include <Test/Routers/http_custom_router.h>

using namespace testing;
using namespace test;

TEST_F(HttpServerTest, HttpServerShouldListenNewConnections) {
  const boost::asio::ip::tcp::endpoint endpoint{
    boost::asio::ip::address::from_string(interface_address),
    18045
  };

  ASSERT_FALSE(HttpServerTest::assert_port_is_in_use(endpoint))
      << "Port In use Before Instantiating Server";

  http::http_server<http_custom_router2>
  http_server{ interface_address,
                endpoint.port(),
                std::filesystem::current_path() };

  EXPECT_FALSE(HttpServerTest::assert_port_is_in_use(endpoint))
    << "Port In use Before Starting Server";

  http_server.start(std::thread::hardware_concurrency(), false);
  std::this_thread::sleep_for(std::chrono::milliseconds{ 100 });

  EXPECT_TRUE(HttpServerTest::assert_port_is_in_use(endpoint))
    <<  "Server is not binded to port 18080";

  http_server.stop();

  EXPECT_FALSE(HttpServerTest::assert_port_is_in_use(endpoint))
    << "Server didn't leave port 18080";
}

