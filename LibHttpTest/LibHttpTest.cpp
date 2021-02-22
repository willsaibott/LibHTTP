#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include <boost/utility/string_view.hpp>
#include <boost/asio/streambuf.hpp>
#include <LibHttp/http_server.h>
#include <LibHttp/websocket.h>
#include <condition_variable>
#include "http_custom_router.h"

std::string const cert =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIDaDCCAlCgAwIBAgIJAO8vBu8i8exWMA0GCSqGSIb3DQEBCwUAMEkxCzAJBgNV\n"
    "BAYTAlVTMQswCQYDVQQIDAJDQTEtMCsGA1UEBwwkTG9zIEFuZ2VsZXNPPUJlYXN0\n"
    "Q049d3d3LmV4YW1wbGUuY29tMB4XDTE3MDUwMzE4MzkxMloXDTQ0MDkxODE4Mzkx\n"
    "MlowSTELMAkGA1UEBhMCVVMxCzAJBgNVBAgMAkNBMS0wKwYDVQQHDCRMb3MgQW5n\n"
    "ZWxlc089QmVhc3RDTj13d3cuZXhhbXBsZS5jb20wggEiMA0GCSqGSIb3DQEBAQUA\n"
    "A4IBDwAwggEKAoIBAQDJ7BRKFO8fqmsEXw8v9YOVXyrQVsVbjSSGEs4Vzs4cJgcF\n"
    "xqGitbnLIrOgiJpRAPLy5MNcAXE1strVGfdEf7xMYSZ/4wOrxUyVw/Ltgsft8m7b\n"
    "Fu8TsCzO6XrxpnVtWk506YZ7ToTa5UjHfBi2+pWTxbpN12UhiZNUcrRsqTFW+6fO\n"
    "9d7xm5wlaZG8cMdg0cO1bhkz45JSl3wWKIES7t3EfKePZbNlQ5hPy7Pd5JTmdGBp\n"
    "yY8anC8u4LPbmgW0/U31PH0rRVfGcBbZsAoQw5Tc5dnb6N2GEIbq3ehSfdDHGnrv\n"
    "enu2tOK9Qx6GEzXh3sekZkxcgh+NlIxCNxu//Dk9AgMBAAGjUzBRMB0GA1UdDgQW\n"
    "BBTZh0N9Ne1OD7GBGJYz4PNESHuXezAfBgNVHSMEGDAWgBTZh0N9Ne1OD7GBGJYz\n"
    "4PNESHuXezAPBgNVHRMBAf8EBTADAQH/MA0GCSqGSIb3DQEBCwUAA4IBAQCmTJVT\n"
    "LH5Cru1vXtzb3N9dyolcVH82xFVwPewArchgq+CEkajOU9bnzCqvhM4CryBb4cUs\n"
    "gqXWp85hAh55uBOqXb2yyESEleMCJEiVTwm/m26FdONvEGptsiCmF5Gxi0YRtn8N\n"
    "V+KhrQaAyLrLdPYI7TrwAOisq2I1cD0mt+xgwuv/654Rl3IhOMx+fKWKJ9qLAiaE\n"
    "fQyshjlPP9mYVxWOxqctUdQ8UnsUKKGEUcVrA08i1OAnVKlPFjKBvk+r7jpsTPcr\n"
    "9pWXTO9JrYMML7d+XRSZA1n3856OqZDX4403+9FnXCvfcLZLLKTBvwwFgEFGpzjK\n"
    "UEVbkhd5qstF6qWK\n"
    "-----END CERTIFICATE-----\n";

std::string const key =
    "-----BEGIN PRIVATE KEY-----\n"
    "MIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQDJ7BRKFO8fqmsE\n"
    "Xw8v9YOVXyrQVsVbjSSGEs4Vzs4cJgcFxqGitbnLIrOgiJpRAPLy5MNcAXE1strV\n"
    "GfdEf7xMYSZ/4wOrxUyVw/Ltgsft8m7bFu8TsCzO6XrxpnVtWk506YZ7ToTa5UjH\n"
    "fBi2+pWTxbpN12UhiZNUcrRsqTFW+6fO9d7xm5wlaZG8cMdg0cO1bhkz45JSl3wW\n"
    "KIES7t3EfKePZbNlQ5hPy7Pd5JTmdGBpyY8anC8u4LPbmgW0/U31PH0rRVfGcBbZ\n"
    "sAoQw5Tc5dnb6N2GEIbq3ehSfdDHGnrvenu2tOK9Qx6GEzXh3sekZkxcgh+NlIxC\n"
    "Nxu//Dk9AgMBAAECggEBAK1gV8uETg4SdfE67f9v/5uyK0DYQH1ro4C7hNiUycTB\n"
    "oiYDd6YOA4m4MiQVJuuGtRR5+IR3eI1zFRMFSJs4UqYChNwqQGys7CVsKpplQOW+\n"
    "1BCqkH2HN/Ix5662Dv3mHJemLCKUON77IJKoq0/xuZ04mc9csykox6grFWB3pjXY\n"
    "OEn9U8pt5KNldWfpfAZ7xu9WfyvthGXlhfwKEetOuHfAQv7FF6s25UIEU6Hmnwp9\n"
    "VmYp2twfMGdztz/gfFjKOGxf92RG+FMSkyAPq/vhyB7oQWxa+vdBn6BSdsfn27Qs\n"
    "bTvXrGe4FYcbuw4WkAKTljZX7TUegkXiwFoSps0jegECgYEA7o5AcRTZVUmmSs8W\n"
    "PUHn89UEuDAMFVk7grG1bg8exLQSpugCykcqXt1WNrqB7x6nB+dbVANWNhSmhgCg\n"
    "VrV941vbx8ketqZ9YInSbGPWIU/tss3r8Yx2Ct3mQpvpGC6iGHzEc/NHJP8Efvh/\n"
    "CcUWmLjLGJYYeP5oNu5cncC3fXUCgYEA2LANATm0A6sFVGe3sSLO9un1brA4zlZE\n"
    "Hjd3KOZnMPt73B426qUOcw5B2wIS8GJsUES0P94pKg83oyzmoUV9vJpJLjHA4qmL\n"
    "CDAd6CjAmE5ea4dFdZwDDS8F9FntJMdPQJA9vq+JaeS+k7ds3+7oiNe+RUIHR1Sz\n"
    "VEAKh3Xw66kCgYB7KO/2Mchesu5qku2tZJhHF4QfP5cNcos511uO3bmJ3ln+16uR\n"
    "GRqz7Vu0V6f7dvzPJM/O2QYqV5D9f9dHzN2YgvU9+QSlUeFK9PyxPv3vJt/WP1//\n"
    "zf+nbpaRbwLxnCnNsKSQJFpnrE166/pSZfFbmZQpNlyeIuJU8czZGQTifQKBgHXe\n"
    "/pQGEZhVNab+bHwdFTxXdDzr+1qyrodJYLaM7uFES9InVXQ6qSuJO+WosSi2QXlA\n"
    "hlSfwwCwGnHXAPYFWSp5Owm34tbpp0mi8wHQ+UNgjhgsE2qwnTBUvgZ3zHpPORtD\n"
    "23KZBkTmO40bIEyIJ1IZGdWO32q79nkEBTY+v/lRAoGBAI1rbouFYPBrTYQ9kcjt\n"
    "1yfu4JF5MvO9JrHQ9tOwkqDmNCWx9xWXbgydsn/eFtuUMULWsG3lNjfst/Esb8ch\n"
    "k5cZd6pdJZa4/vhEwrYYSuEjMCnRb0lUsm7TsHxQrUd6Fi/mUuFU/haC0o0chLq7\n"
    "pVOUFq5mW8p0zbtfHbjkgxyF\n"
    "-----END PRIVATE KEY-----\n";

std::string const dh =
    "-----BEGIN DH PARAMETERS-----\n"
    "MIIBCAKCAQEArzQc5mpm0Fs8yahDeySj31JZlwEphUdZ9StM2D8+Fo7TMduGtSi+\n"
    "/HRWVwHcTFAgrxVdm+dl474mOUqqaz4MpzIb6+6OVfWHbQJmXPepZKyu4LgUPvY/\n"
    "4q3/iDMjIS0fLOu/bLuObwU5ccZmDgfhmz1GanRlTQOiYRty3FiOATWZBRh6uv4u\n"
    "tff4A9Bm3V9tLx9S6djq31w31Gl7OQhryodW28kc16t9TvO1BzcV3HjRPwpe701X\n"
    "oEEZdnZWANkkpR/m/pfgdmGPU66S2sXMHgsliViQWpDCYeehrvFRHEdR9NV+XJfC\n"
    "QMUk26jPTIVTLfXmmwU0u8vUkpR7LQKkwwIBAg==\n"
    "-----END DH PARAMETERS-----\n";

const std::string interface_address{ "127.0.0.1" };
const std::string invalid_interface_address{ "128.255.255.1" };

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

    TEST_METHOD(TestTargetUrlDecoded) {
        const short server_port = 18080;
        http::http_server<http_custom_router>
            http_server{ interface_address, server_port, std::filesystem::current_path() };
        http_server.start(1, false);

        auto response = do_http_request_to(interface_address,
                                           std::to_string(server_port),
                                           "/alice/wonderland?filter={\"name\"=\"bob\"}");

        std::string actual = boost::beast::buffers_to_string(response.body().data());
        std::string expected{ "{\"bob\":\"hello world\"}" };

        Assert::AreEqual(expected, actual);
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
