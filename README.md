# LibHTTP
LibHTTP is a C++ library similar to NodeJS Express in usage, and is based on boost beast. [In development]

## HTTP Router Usage Example:

### router.h:
```C++
#include <LibHttp\http_server.h>

class http_custom_router : public http::http_regex_router {
public:
  http_custom_router();
};

```

### router.cpp:
```C++
#include "router.h"

http_custom_router::
http_custom_router() {

  get("/", [](const auto& root,
              const auto& request,
              const auto& matches,
              auto&       sender)
  {
    namespace http = boost::beast::http;
    http::response<http::string_body> res{ http::status::ok, request.version() };
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, helper::mime_type(".json"));
    res.keep_alive(request.keep_alive());
    res.body() = "{ \"Root\": \"reached\" }";
    res.prepare_payload();
    sender.async_send(std::move(res));

    boost::ignore_unused(root);
    boost::ignore_unused(matches);
    return true;
  });

  get("/print(.*)/?", [](const auto& root,
                         const auto& request,
                         const auto& matches,
                         auto&       sender)
  {
    namespace http = boost::beast::http;
    http::response<http::string_body> res{http::status::ok, request.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "text/html; charset=utf-8");
    res.keep_alive(request.keep_alive());
    res.body() = "<!DOCTYPE html><html>"
                 "<head>"
                   "<title>Express C++</title>"
                   "<link rel=\"stylesheet\" href=\"/stylesheets/style.css\">"
                 "</head>"
                 "<body>"
                   "<h1>Express</h1>"
                   "<p>Welcome to Express C++ using Boost Beast (Asio)</p>"
                   "<p>Parameter: <b>" + matches[1] + "</b></p>"
                 "</body>"
                 "</html>";
    res.prepare_payload();
    sender.async_send(std::move(res));

    boost::ignore_unused(root);
    return true;
  });
  
  post("/post(.*)", [](const auto& root,
                       const auto& request,
                       const auto& matches,
                       auto&       sender)
  {
    namespace http = boost::beast::http;
    http::response<http::string_body> res{http::status::ok, request.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, helper::mime_type(".json"));
    res.keep_alive(request.keep_alive());
    res.body() = std::string{ "{ \"print\": \"" } +
                 std::string{ matches[static_cast<int>(matches.size() - 1ull)] } +
                 "\"}";
    res.prepare_payload();
    sender.async_send(std::move(res));

    boost::ignore_unused(root);
    boost::ignore_unused(matches);
    return true;
  });
}

```

### http_server.cpp:
```C++
#include <thread>
#include <string>
#include <filesystem>
#include <LibHttp/http_server.h>
#include "router.h"

int main() {
  const std::string interface_address{ "127.0.0.1" };
  http::http_server<http_custom_router>
        http_server { interface_address, 18080, std::filesystem::current_path() };
        
  std::thread thread {
    [&http_server]() {
      http_server.start(std::thread::hardware_concurrency());
      if (http_server.last_error()) {
        std::cerr << "Error starting HTTP server: "
                  << http_server.last_error().message()
                  << std::endl;
      }
    }
  };
  std::cout << "HTTP server running at http://" << interface_address << ":18080..." <<  std::endl;
  std::this_thread::sleep_for(std::chrono::minutes{ 1000 });
  http_server.stop();
  thread.join();
  return 0;
}

```
## HTTPS Router Usage Example:
### https_server.cpp:
```C++
#include <thread>
#include <string>
#include <filesystem>
#include <LibHttp/http_server.h>
#include "router.h"

int main() {
  const std::string interface_address{ "127.0.0.1" };
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
          
  http::https_server<http_custom_router>
        https_server { interface_address, 18045, std::filesystem::current_path() };
        
  https_server.set_ssl_context(http::ssl_dh_context_creator{}(cert, key, dh, "test"));
  
  std::thread thread {
    [&https_server]() {
      https_server.start(std::thread::hardware_concurrency());
      if (httsp_server.last_error()) {
        std::cerr << "Error starting HTTPS server: "
                  << https_server.last_error().message()
                  << std::endl;
      }
    }
  };
  
  std::cout << "HTTPS server running at https://" << interface_address << ":18045..." <<  std::endl;
  std::this_thread::sleep_for(std::chrono::minutes{ 1000 });
  https_server.stop();
  thread.join();
  return 0;
}

```
## WebSocket server Usage Example:
```C++

int main() {
  const std::string interface_address{ "127.0.0.1" };
  http::websocket_server<http::echo_message_handler>
        websocket_server{ interface_address, 18090, std::filesystem::current_path() };
        
  std::thread thread {
    [&websocket_server]() {
      websocket_server.start(std::thread::hardware_concurrency());
      if (websocket_server.last_error()) {
        std::cerr << "Error starting WebSocket server: "
                  << websocket_server.last_error().message()
                  << std::endl;
      }
    }
  };
  
  std::cout << "WebSocket server running at ws://" << interface_address << ":18090..." <<  std::endl;
  std::this_thread::sleep_for(std::chrono::minutes{ 1000 });
  websocket_server.stop();
  thread.join();
  return 0;
}

```

## WebSocket SSL server Usage Example:
```C++

int main() {
  ... declarations of std::string cert, key, dh...
  
  const std::string interface_address{ "127.0.0.1" };
  http::websocket_ssl_server<http::echo_message_handler>
        websocket_ssl_server{ interface_address, 18095, std::filesystem::current_path() };
  
  ... Similar to HTTPS:
  websocket_ssl_server.set_ssl_context(http::ssl_dh_context_creator{}(cert, key, dh, "test"));
  
  std::thread thread {
    [&websocket_ssl_server]() {
      websocket_ssl_server.start(std::thread::hardware_concurrency());
      if (websocket_ssl_server.last_error()) {
        std::cerr << "Error starting WebSocket server: "
                  << websocket_ssl_server.last_error().message()
                  << std::endl;
      }
    }
  };
  
  std::cout << "WebSocket Secure Server running at wss://" << interface_address << ":18095..." <<  std::endl;
  std::this_thread::sleep_for(std::chrono::minutes{ 1000 });
  websocket_ssl_server.stop();
  thread.join();
  return 0;
}

```
# Build Status:

Master:

![Windows Build](https://github.com/willsaibott/LibHTTP/workflows/Windows%20Build/badge.svg?branch=master)
![Linux_Build](https://github.com/willsaibott/LibHTTP/workflows/Linux_Build/badge.svg?branch=master)

Dev:

![Windows Build](https://github.com/willsaibott/LibHTTP/workflows/Windows%20Build/badge.svg?branch=dev)
![Linux_Build](https://github.com/willsaibott/LibHTTP/workflows/Linux_Build/badge.svg?branch=dev)
