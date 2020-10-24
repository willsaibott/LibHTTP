#include <Test/Suites/port_binding_test.h>
#include <Test/consts.h>

namespace test {

  PortBindingTest::PortBindingTest() {

  }

  PortBindingTest::~PortBindingTest() {

  }

  void PortBindingTest::SetUp() {

  }

  void PortBindingTest::TearDown() {

  }

  testing::AssertionResult PortBindingTest::
  assert_port_is_in_use(const boost::asio::ip::tcp::endpoint &endpoint) {
    using namespace boost::asio;
    boost::asio::io_context svc;
    boost::system::error_code ec;
    ip::tcp::acceptor a(svc);

    a.open(endpoint.protocol(), ec) || a.bind(endpoint, ec);
    if (ec == boost::asio::error::basic_errors::address_in_use) {
      return testing::AssertionSuccess();
    }
    return testing::AssertionFailure()
              << "Port " << endpoint.port()
              << " has no tcp-listener binded to it";
  }

}
