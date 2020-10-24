#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include <boost/utility/string_view.hpp>
#include <boost/asio/streambuf.hpp>
#include <LibHttp/http_server.h>
#include <LibHttp/websocket.h>
#include <condition_variable>
#include "http_custom_router.h"



namespace LibHttpTest
{
	TEST_CLASS(LibHttpTest)
	{
	public:

    inline boost::beast::http::response<boost::beast::http::dynamic_body>
    do_http_request_to(std::string_view   host,
                       std::string_view   port,
                       boost::string_view target)
    {
      namespace beast = boost::beast;     // from <boost/beast.hpp>
      namespace http = beast::http;       // from <boost/beast/http.hpp>
      namespace net = boost::asio;        // from <boost/asio.hpp>
      using tcp = net::ip::tcp;           // from <boost/asio/ip/tcp.hpp>

      net::io_context ioc;

      // These objects perform our I/O
      tcp::resolver resolver(ioc);
      beast::tcp_stream stream(ioc);

      // Look up the domain name
      auto const results = resolver.resolve(host, port);

      // Make the connection on the IP address we get from a lookup
      stream.connect(results);

      // Set up an HTTP GET request message
      http::request<http::string_body> req{ http::verb::get, target, 11 };
      req.set(http::field::host, host);
      req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

      // Send the HTTP request to the remote host
      http::write(stream, req);

      // This buffer is used for reading and must be persisted
      beast::flat_buffer buffer;

      // Declare a container to hold the response
      http::response<http::dynamic_body> res;

      // Receive the HTTP response
      beast::error_code ec;
      Assert::AreNotEqual(0ull, http::read(stream, buffer, res, ec));
      Assert::IsFalse(static_cast<bool>(ec));

      // Gracefully close the socket
      stream.socket().shutdown(tcp::socket::shutdown_both, ec);

      Assert::IsFalse(static_cast<bool>(ec));
      return res;
    }

    inline boost::beast::http::response<boost::beast::http::dynamic_body>
    do_https_request_to(std::string_view   host,
                        std::string_view   port,
                        boost::string_view target)
    {
      namespace beast = boost::beast;     // from <boost/beast.hpp>
      namespace http = beast::http;       // from <boost/beast/http.hpp>
      namespace net = boost::asio;        // from <boost/asio.hpp>
      using tcp = net::ip::tcp;           // from <boost/asio/ip/tcp.hpp>

      net::io_context ioc;

      // These objects perform our I/O
      tcp::resolver resolver(ioc);
      net::ssl::context ctx(net::ssl::context::method::tlsv12_client);
      net::ssl::stream<beast::tcp_stream> stream(ioc, ctx);

      // Look up the domain name
      auto const results = resolver.resolve(host, port);

      // Make the connection on the IP address we get from a lookup
      beast::error_code ec;
      stream.next_layer().connect(results);
      stream.set_verify_mode(net::ssl::verify_none);
      stream.handshake(net::ssl::stream_base::handshake_type::client, ec);

      // Set up an HTTP GET request message
      http::request<http::string_body> req{ http::verb::get, target, 11 };
      req.set(http::field::host, host);
      req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

      // Send the HTTP request to the remote host
      http::write(stream, req);

      // This buffer is used for reading and must be persisted
      beast::flat_buffer buffer;

      // Declare a container to hold the response
      http::response<http::dynamic_body> res;

      // Receive the HTTP response
      Assert::AreNotEqual(0ull, http::read(stream, buffer, res, ec));
      Assert::IsFalse(static_cast<bool>(ec));

      // Gracefully close the socket
      stream.lowest_layer().shutdown(tcp::socket::shutdown_both, ec);

      Assert::IsFalse(static_cast<bool>(ec));
      return res;
    }

    inline bool port_in_use(unsigned short port) {
      using namespace boost::asio;
      using ip::tcp;

      boost::asio::io_context svc;
      tcp::acceptor a(svc);

      boost::system::error_code ec;
      auto ip_address = ip::tcp::endpoint{ boost::asio::ip::make_address(interface_address), port };
      a.open(tcp::v4(), ec) || a.bind(ip_address, ec);

      return ec == error::address_in_use;
    }

    TEST_METHOD(TestHttpServerBind) {
      const short server_port = 18080;
      Assert::IsFalse(port_in_use(server_port), L"Port In use Before Instantiating Server", LINE_INFO());

      http::http_server<http_custom_router>
        http_server{ interface_address, server_port, std::filesystem::current_path() };

      Assert::IsFalse(port_in_use(server_port), L"Port In use Before Starting Server", LINE_INFO());

      http_server.start(std::thread::hardware_concurrency(), false);
      std::this_thread::sleep_for(std::chrono::milliseconds{ 100 });

      {
        Assert::IsTrue(port_in_use(server_port), L"Server is not binded to port 18080", LINE_INFO());

        auto response = do_http_request_to(interface_address, std::to_string(server_port), "/");

        std::string actual = boost::beast::buffers_to_string(response.body().data());
        std::string expected{ "{ \"Root\": \"reached\" }" };

        Assert::AreEqual(expected, actual);
      }

      {
        auto response = do_http_request_to(interface_address, std::to_string(server_port), "/print");

        std::string actual = boost::beast::buffers_to_string(response.body().data());
        std::string expected{ "<!DOCTYPE html><html><head><title>Express C++</title><link rel=\"stylesheet\" href=\"/stylesheets/style.css\"></head><body><h1>Express</h1><p>Welcome to Express C++ using Boost Beast (Asio)</p><p>Parameter: <b>" "</b></p></body></html>" };

        Assert::AreEqual(expected, actual);
      }

      http_server.stop();

      Assert::IsFalse(port_in_use(server_port), L"Server didn't leave port 18080", LINE_INFO());
    }

    TEST_METHOD(TestWebSocketServerBind) {
      const short server_port = 18090;
      Assert::IsFalse(port_in_use(server_port), L"Port In use Before Instantiating Server", LINE_INFO());

      http::websocket_server<http::echo_message_handler>
        websocket_server{ interface_address, server_port, std::filesystem::current_path() };

      Assert::IsFalse(port_in_use(server_port));
      websocket_server.start(std::thread::hardware_concurrency(), false);
      std::this_thread::sleep_for(std::chrono::milliseconds{ 100 });
      Assert::IsTrue(port_in_use(server_port), L"Server is not binded to port 18090", LINE_INFO());
      websocket_server.stop();
      Assert::IsFalse(port_in_use(server_port), L"Server didn't leave port 18090", LINE_INFO());
    }

    TEST_METHOD(TestHttpsServerBind) {
      const short server_port = 18045;
      Assert::IsFalse(port_in_use(server_port), L"Port In use Before Instantiating Server", LINE_INFO());

        http::https_server<http_custom_router>
          https_server{ interface_address, server_port, std::filesystem::current_path() };
        https_server.set_ssl_context(http::ssl_dh_context_creator{}(cert, key, dh, "test"));

        Assert::IsFalse(port_in_use(server_port), L"Port In use Before Instantiating Server", LINE_INFO());
        https_server.start(std::thread::hardware_concurrency(), false);
        std::this_thread::sleep_for(std::chrono::milliseconds{ 100 });

        Assert::IsTrue(port_in_use(server_port), L"Server is not binded to port 18045", LINE_INFO());
        {
          auto response = do_https_request_to(interface_address, std::to_string(server_port), "/");

          std::string actual = boost::beast::buffers_to_string(response.body().data());
          std::string expected{ "{ \"Root\": \"reached\" }" };

          Assert::AreEqual(expected, actual);
        }

        {
          auto response = do_https_request_to(interface_address, std::to_string(server_port), "/print");

          auto bufs = response.body().data();
          std::string actual = boost::beast::buffers_to_string(response.body().data());
          std::string expected{ "<!DOCTYPE html><html><head><title>Express C++</title><link rel=\"stylesheet\" href=\"/stylesheets/style.css\"></head><body><h1>Express</h1><p>Welcome to Express C++ using Boost Beast (Asio)</p><p>Parameter: <b>" "</b></p></body></html>" };

          Assert::AreEqual(expected, actual);
        }

        https_server.stop();
        Assert::IsFalse(port_in_use(server_port), L"Server didn't leave port 18045", LINE_INFO());
    }

    TEST_METHOD(TestSecureWebSocketServerBind) {
      const short server_port = 18095;
      Assert::IsFalse(port_in_use(server_port), L"Port In use Before Instantiating Server", LINE_INFO());

       http::websocket_ssl_server<http::echo_message_handler>
         websocket_ssl_server{ interface_address, server_port, std::filesystem::current_path() };
       websocket_ssl_server.set_ssl_context(http::ssl_dh_context_creator{}(cert, key, dh, "test"));

       Assert::IsFalse(port_in_use(server_port), L"Port In use Before Instantiating Server", LINE_INFO());
       websocket_ssl_server.start(std::thread::hardware_concurrency(), false);
       std::this_thread::sleep_for(std::chrono::milliseconds{ 100 });

       Assert::IsTrue(port_in_use(server_port), L"Server is not binded to port 18095", LINE_INFO());
       websocket_ssl_server.stop();

       Assert::IsFalse(port_in_use(server_port), L"Server didn't leave port 18095", LINE_INFO());
    }

    TEST_METHOD(TestInvalidInterfaceBinding) {
      const short server_port = 18080;
      std::atomic_bool stopped{ false };
      bool timed_out{ false };
      std::condition_variable cv;

      http::http_server<http_custom_router>
        http_server{ invalid_interface_address, server_port };

      std::thread thread2{
        [&cv, &http_server, &stopped, &timed_out]() {
          std::mutex mtx;
          std::unique_lock<std::mutex> lock{ mtx };
          timed_out = !
            cv.wait_for(lock,
                        std::chrono::milliseconds{ 200 },
                        [&stopped]() { return (bool)stopped; });

          http_server.stop();
        }
      };

      std::thread thread{
        [&http_server, &stopped, &cv]() {
          http_server.start(std::thread::hardware_concurrency());
          stopped = true;
          cv.notify_one();
        }
      };

      //Assert::AreEqual(boost::beast::ehttp_server.listener()->error());

      thread.join();
      thread2.join();

      auto ec = http_server.last_error();
      Assert::IsTrue(static_cast<bool>(ec));
      Assert::IsFalse(timed_out, L"Timed out: Server was still running...");
    }
  };

}
