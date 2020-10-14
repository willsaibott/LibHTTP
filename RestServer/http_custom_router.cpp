#include "http_custom_router.h"

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
    /*res.body() = std::string{ "{ \"print\": \"" } +
                  std::string{ matches[matches.size() - 1]} +
                  "\"}";*/
    res.body() = "<!DOCTYPE html><html><head><title>Express C++</title><link rel=\"stylesheet\" href=\"/stylesheets/style.css\"></head><body><h1>Express</h1><p>Welcome to Express C++ using Boost Beast (Asio)</p><p>Parameter: <b>" + matches[1] + "</b></p></body></html>";
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
                 std::string{ matches[static_cast<int>(matches.size() - 1)]} +
                 "\"}";
    res.prepare_payload();
    sender.async_send(std::move(res));

    boost::ignore_unused(root);
    boost::ignore_unused(matches);
    return true;
  });
}
