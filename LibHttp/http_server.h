#pragma once

//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

//------------------------------------------------------------------------------
//
// Example: HTTP SSL server, asynchronous
//
//------------------------------------------------------------------------------

#pragma clang diagnostic ignored "-Wweak-vtables"

#include <algorithm>
#include <string_view>
#include <string>
#include <filesystem>
#include <codecvt>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/config.hpp>
#include <boost/regex.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include "libhttp_global.h"
#include "mime_type_helper.h"
#include "path_helper.h"


//namespace beast = boost::beast;    // from <boost/beast.hpp>
//namespace http = beast::http;      // from <boost/beast/http.hpp>
       // from <boost/asio.hpp>
//namespace ssl = boost::asio::ssl;  // from <boost/asio/ssl.hpp>
//using tcp = boost::asio::ip::tcp;  // from <boost/asio/ip/tcp.hpp>



//// Append an HTTP rel-path to a local filesystem path.
//// The returned path is normalized for the platform.
//std::string path_cat(beast::string_view base, beast::string_view path) {
//  if (base.empty()) return std::string(path);
//  std::string result(base);
//#ifdef BOOST_MSVC
//  char constexpr path_separator = '\\';
//  if (result.back() == path_separator) result.resize(result.size() - 1);
//  result.append(path.data(), path.size());
//  for (auto& c : result)
//    if (c == '/') c = path_separator;
//#else
//  char constexpr path_separator = '/';
//  if (result.back() == path_separator) result.resize(result.size() - 1);
//  result.append(path.data(), path.size());
//#endif
//  return result;
//}

//void handle_request(
//}

//------------------------------------------------------------------------------




//------------------------------------------------------------------------------

//------------------------------------------------------------------------------

namespace http {
  namespace net = boost::asio;

  template <class RequestHandler, class ErrorHandler>
  class https_session;

  // Handles an HTTP server connection
  class  IResponseSender {

    public:
    IResponseSender() = default;
    virtual ~IResponseSender() {}

    virtual void
    async_send(boost::beast::http::response<boost::beast::http::dynamic_body>&& msg)
    { boost::ignore_unused(msg); }

    virtual void
    async_send(boost::beast::http::response<boost::beast::http::string_body>&& msg)
    { boost::ignore_unused(msg); }

    virtual void
    async_send(boost::beast::http::response<boost::beast::http::file_body>&& msg)
    { boost::ignore_unused(msg); }

    virtual void
    async_send(boost::beast::http::response<boost::beast::http::empty_body>&& msg)
    { boost::ignore_unused(msg); }
  };

  class  ssl_dh_context_creator {
  public:

    inline net::ssl::context*
    operator()(const std::string& certificate,
               const std::string& key,
               const std::string& dh,
               const std::string& password) const
    {
      net::ssl::context* context =
        new net::ssl::context{ net::ssl::context::method::tlsv12 };

      context->set_password_callback(
        [password] (std::size_t max_length, auto password_purpose) {
          boost::ignore_unused(max_length);
          boost::ignore_unused(password_purpose);
          return password;
        });
      context->set_options(net::ssl::context::default_workarounds |
                           net::ssl::context::no_sslv2 |
                           net::ssl::context::single_dh_use);
      context->use_certificate_chain( boost::asio::buffer(certificate) );
      context->use_private_key(net::buffer(key),
                               boost::asio::ssl::context::file_format::pem);
      context->use_tmp_dh(boost::asio::buffer(dh));
      return context;
    }

  };

  class  http_regex_router {
  public:
    using http_request_t = boost::beast::http::request<boost::beast::http::string_body>;
    using middleware_t   = std::function<bool(const std::filesystem::path&,
                                              const http_request_t&,
                                              const boost::smatch&,
                                              IResponseSender& )>;
    using handler_t = std::pair<boost::regex, middleware_t>;

    public:

    inline void
    on_delete(const std::filesystem::path& root,
              const http_request_t&        request,
              IResponseSender&             sender)
    {
      iterate_handlers(_on_delete_handlers, root, request, sender);
    }

    inline void
    on_put(const std::filesystem::path& root,
           const http_request_t&        request,
           IResponseSender&             sender)
    {
      iterate_handlers(_on_put_handlers, root, request, sender);
    }

    inline void
    on_post(const std::filesystem::path& root,
            const http_request_t&        request,
            IResponseSender&             sender)
    {
      iterate_handlers(_on_post_handlers, root, request, sender);
    }

    inline void
    on_get(const std::filesystem::path& root,
           const http_request_t&        request,
           IResponseSender&             sender)
    {
      iterate_handlers(_on_get_handlers, root, request, sender);
    }

    inline void
    on_options(const std::filesystem::path& root,
               const http_request_t&        request,
               IResponseSender&             sender)
    {
      iterate_handlers(_on_options_handlers, root, request, sender);
    }

    inline void
    on_head(const std::filesystem::path& root,
            const http_request_t&        request,
            IResponseSender&             sender)
    {
      iterate_handlers(_on_head_handlers, root, request, sender);
    }

    inline void
    non_implemented_http_verb(const std::filesystem::path& root,
                              const http_request_t&        request,
                              IResponseSender&             sender)
    {
      boost::ignore_unused(root);
      sender.async_send(not_implemented(request));
    }

    void
    post(const std::string& regex, middleware_t middleware) {
      _on_post_handlers.emplace_back(
        std::make_pair<>(
          boost::regex{ regex, boost::regex_constants::ECMAScript },
          middleware));
    }

    void
    get(const std::string& regex, middleware_t middleware) {
      _on_get_handlers.emplace_back(
        std::make_pair<>(
          boost::regex{ regex, boost::regex_constants::ECMAScript },
          middleware));
    }

    void
    put(const std::string& regex, middleware_t middleware) {
      _on_put_handlers.emplace_back(
        std::make_pair<>(
          boost::regex{ regex, boost::regex_constants::ECMAScript },
          middleware));
    }

    void
    options(const std::string& regex, middleware_t middleware) {
      _on_options_handlers.emplace_back(
        std::make_pair<>(
          boost::regex{ regex, boost::regex_constants::ECMAScript },
          middleware));
    }

    void
    head(const std::string& regex, middleware_t middleware) {
      _on_head_handlers.emplace_back(
        std::make_pair<>(
          boost::regex{ regex, boost::regex_constants::ECMAScript },
          middleware));
    }

    void
    delete_handler(const std::string& regex, middleware_t middleware) {
      _on_delete_handlers.emplace_back(
        std::make_pair<>(
          boost::regex{ regex, boost::regex_constants::ECMAScript },
          middleware));
    }

    private:

    boost::beast::http::response<boost::beast::http::string_body>
    bad_request(const http_request_t& request, const std::string& reason_why) {
      namespace http = boost::beast::http;
      http::response<http::string_body> res{ http::status::bad_request,
                                             request.version()};
      res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
      res.set(http::field::content_type, "text/html");
      res.keep_alive(request.keep_alive());
      res.body() = reason_why;
      res.prepare_payload();
      return res;
    }

    boost::beast::http::response<boost::beast::http::string_body>
    not_implemented(const http_request_t& request) {
      namespace http = boost::beast::http;
      http::response<http::string_body> res{ http::status::not_implemented,
                                             request.version()};
      res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
      res.set(http::field::content_type, "text/html");
      res.keep_alive(request.keep_alive());
      res.body() = std::string{ "Unknown HTTP-method: "} + request.method_string().data();
      res.prepare_payload();
      return res;
    }

    boost::beast::http::response<boost::beast::http::string_body>
    not_found(const http_request_t& request) {
      namespace http = boost::beast::http;
      http::response<http::string_body> res{ http::status::not_found,
                                             request.version()};
      res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
      res.set(http::field::content_type, "text/html");
      res.keep_alive(request.keep_alive());
      res.body() = std::string{ "Not Found: " } + request.target().data();
      res.prepare_payload();
      return res;
    }

    void
    iterate_handlers(const std::vector<handler_t>& handlers,
                     const std::filesystem::path&  root,
                     const http_request_t&         request,
                     IResponseSender&              sender)
    {
      std::string target{ request.target().data(), request.target().size() };

      if (target.empty()) {
        target += "/";
      }

      if (handlers.empty()) {
        sender.async_send(not_implemented(request));
      }
      else if (target.find("..") != std::string::npos) {
        const std::string reason{ std::string{ "Illegal path: " }.append(target) };
        sender.async_send(bad_request(request, reason));
      }
      else {
        bool route_found{ false };

        for (const auto& handler : handlers) {
          const auto& regex      { handler.first  };
          const auto& middleware { handler.second };
          boost::smatch matches;
          const auto utf16 = std::wstring(target.begin(), target.end());
          if (boost::regex_match(target, matches, regex)) {
            route_found = true;
            if (middleware(root, request, matches, sender)) {
              break;
            }
          }
        }

        if (!route_found) {
          sender.async_send(not_found(request));
        }
      }
    }


    // handlers:

    std::vector<handler_t>  _on_delete_handlers;
    std::vector<handler_t>  _on_put_handlers;
    std::vector<handler_t>  _on_get_handlers;
    std::vector<handler_t>  _on_post_handlers;
    std::vector<handler_t>  _on_options_handlers;
    std::vector<handler_t>  _on_head_handlers;

  };

  template <class Router = http_regex_router>
  class  http_request_handler {

  public:
    using http_request_t = boost::beast::http::request<boost::beast::http::string_body>;

    // This function produces an HTTP response for the given
    // request. The type of the response object depends on the
    // contents of the request, so the interface requires the
    // caller to pass a generic lambda for receiving the response.
    inline void
    operator()(const std::filesystem::path    &doc_root,
               const http_request_t           &req,
               IResponseSender                &send)
    {
      switch (req.method()) {
        case boost::beast::http::verb::get:      _router.on_get(doc_root, req, send);                      break;
        case boost::beast::http::verb::post:     _router.on_post(doc_root, req, send);                     break;
        case boost::beast::http::verb::put:      _router.on_head(doc_root, req, send);                     break;
        case boost::beast::http::verb::head:     _router.on_head(doc_root, req, send);                     break;
        case boost::beast::http::verb::options:  _router.on_options(doc_root, req, send);                  break;
        case boost::beast::http::verb::delete_:  _router.on_delete(doc_root, req, send);                   break;
        default:                                 _router.non_implemented_http_verb(doc_root, req, send);   break;
      }
    }

    protected:

      Router _router;
  };

  class  http_error_handler {
  public:

  // Report a failure
    inline void
    operator()(boost::beast::error_code ec, char const* what) {
      // ssl::error::stream_truncated, also known as an SSL "short read",
      // indicates the peer closed the connection without performing the
      // required closing handshake (for example, Google does this to
      // improve performance). Generally this can be a security issue,
      // but if your communication protocol is self-terminated (as
      // it is with both HTTP and WebSocket) then you may simply
      // ignore the lack of close_notify.
      //
      // https://github.com/boostorg/beast/issues/38
      //
      // https://security.stackexchange.com/questions/91435/how-to-handle-a-malicious-ssl-tls-shutdown
      //
      // When a short read would cut off the end of an HTTP message,
      // Beast returns the error boost::beast::http::error::partial_message.
      // Therefore, if we see a short read here, it has occurred
      // after the message has been completed, so it is safe to ignore it.

      if (ec != net::ssl::error::stream_truncated) {
        std::cerr << what << ": " << ec.message() << "\n";
      }
    }
  };

  // Handles an HTTPS server connection
  template <class RequestHandler = http_request_handler<http_regex_router>,
            class ErrorHandler   = http_error_handler>
  class  https_session
    : public std::enable_shared_from_this<https_session<RequestHandler, ErrorHandler>>,
      public IResponseSender
  {
  public:

    boost::beast::ssl_stream<boost::beast::tcp_stream>               _stream;
    boost::beast::flat_buffer                                        _buffer;
    std::filesystem::path                                            _doc_root;
    boost::beast::http::request<boost::beast::http::string_body>     _req;
    std::shared_ptr<void>                                            _res;
    ErrorHandler                                                     _error_handler;
    RequestHandler                                                   _request_handler;

 public:
    // Take ownership of the socket
    explicit
    https_session(boost::asio::ip::tcp::socket&&      socket,
                  std::shared_ptr<net::ssl::context>  ssl_context,
                  const std::filesystem::path&        doc_root)
        : _stream(std::move(socket), *ssl_context),
          _doc_root(doc_root) { }

    // Start the asynchronous operation
    void
    run() {
      // We need to be executing within a strand to perform async operations
      // on the I/O objects in this Session. Although not strictly necessary
      // for single-threaded contexts, this example code is written to be
      // thread-safe by default.
      net::dispatch(_stream.get_executor(),
                    boost::beast::bind_front_handler(&https_session::on_run,
                                                     this->shared_from_this()));
    }

    virtual void
    on_run() {
      // Set the timeout.
      boost::beast::get_lowest_layer(_stream).expires_after(std::chrono::seconds(30));
      // Perform the SSL handshake
      _stream.async_handshake(
          net::ssl::stream_base::server,
          boost::beast::bind_front_handler(&https_session::on_handshake,
                                    this->shared_from_this()));
    }

    void
    on_handshake(boost::beast::error_code ec) {
      if (ec) return (_error_handler(ec, "handshake"));

      do_read();
    }

    void
    do_read() {
      // Make the request empty before reading,
      // otherwise the operation behavior is undefined.
      _req = {};

      // Set the timeout.
      boost::beast::get_lowest_layer(_stream).expires_after(std::chrono::seconds(30));

      // Read a request
      boost::beast::http::async_read(
          _stream, _buffer, _req,
          boost::beast::bind_front_handler(&https_session::on_read,
                                           this->shared_from_this()));
    }

    void
    on_read(boost::beast::error_code ec, std::size_t bytes_transferred) {
      boost::ignore_unused(bytes_transferred);

      // This means they closed the connection
      if (ec == boost::beast::http::error::end_of_stream) return do_close();

      if (ec) return (_error_handler(ec, "read"));

      /*_req.body() = reinterpret_cast*/
      // Send the response
      _request_handler(_doc_root, std::move(_req), *this);
    }

    void
    on_write(bool                     close,
             boost::beast::error_code ec,
             std::size_t              bytes_transferred)
    {
      boost::ignore_unused(bytes_transferred);

      if (ec) return (_error_handler(ec, "write"));

      if (close) {
        // This means we should close the connection, usually because
        // the response indicated the "Connection: close" semantic.
        return do_close();
      }

      // We're done with the response so delete it
      _res = nullptr;

      // Read another request
      do_read();
    }

    void
    do_close() {
      // Set the timeout.
      boost::beast::get_lowest_layer(_stream).expires_after(std::chrono::seconds(30));

      // Perform the SSL shutdown
      _stream.async_shutdown(
          boost::beast::bind_front_handler(&https_session::on_shutdown,
                                           this->shared_from_this()));
      /*boost::beast::http::async_write(_stream, )*/
    }

    void on_shutdown(boost::beast::error_code ec) {
      if (ec) {
        _error_handler(ec, "shutdown");
        boost::beast::get_lowest_layer(_stream).socket().shutdown(boost::asio::socket_base::shutdown_send, ec);
        boost::beast::get_lowest_layer(_stream).socket().close();
        return boost::beast::get_lowest_layer(_stream).close();
      }
      boost::beast::get_lowest_layer(_stream).socket().shutdown(boost::asio::socket_base::shutdown_send, ec);
    }

    void
    on_close(boost::beast::error_code ec) {
      if (ec) {
        _error_handler(ec, "close");
      }
      boost::beast::get_lowest_layer(_stream).socket().close();
    }

    protected:

    virtual void
    async_send(boost::beast::http::response<boost::beast::http::dynamic_body>&& msg) override
    {
      generic_async_send(std::move(msg));
    }

    virtual void
    async_send(boost::beast::http::response<boost::beast::http::string_body>&& msg) override
    {
      generic_async_send(std::move(msg));
    }

    virtual void
    async_send(boost::beast::http::response<boost::beast::http::file_body>&& msg) override
    {
      generic_async_send(std::move(msg));
    }

    virtual void
    async_send(boost::beast::http::response<boost::beast::http::empty_body>&& msg) override
    {
      generic_async_send(std::move(msg));
    }

    template <class ResponseType> void
    generic_async_send(ResponseType&& msg) {
      // The lifetime of the message has to extend
      // for the duration of the async operation so
      // we use a shared_ptr to manage it.
      auto sp = std::make_shared<ResponseType>(std::move(msg));

      // Store a type-erased version of the shared
      // pointer in the class to keep it alive.
      _res = sp;

      // Write the response
      boost::beast::http::async_write(
          _stream,
          *sp,
          boost::beast::bind_front_handler(&https_session::on_write,
                                           this->shared_from_this(),
                                           sp->need_eof()));
    }

    std::string
    to_utf8(const wchar_t* utf16) const {
      std::wstring utf16String{ utf16 };
      std::wstring_convert<std::codecvt_utf8<wchar_t>> convert;
      return convert.to_bytes( utf16String );
    }
  };

  // Handles an HTTP server connection
  template <class RequestHandler = http_request_handler<http_regex_router>,
            class ErrorHandler   = http_error_handler>
  class  http_session
    : public std::enable_shared_from_this<http_session<RequestHandler, ErrorHandler>>,
      public IResponseSender
  {
  public:

    boost::beast::tcp_stream                                         _stream;
    boost::beast::flat_buffer                                        _buffer;
    std::filesystem::path                                            _doc_root;
    boost::beast::http::request<boost::beast::http::string_body>     _req;
    std::shared_ptr<void>                                            _res;
    ErrorHandler                                                     _error_handler;
    RequestHandler                                                   _request_handler;

 public:
    // Take ownership of the socket
    explicit
    http_session(boost::asio::ip::tcp::socket&&      socket,
                 std::shared_ptr<net::ssl::context>  ssl_context,
                 const std::filesystem::path&        doc_root)
        : _stream(std::move(socket)),
          _doc_root(doc_root)
    {
      boost::ignore_unused(ssl_context);
    }


    // Start the asynchronous operation
    void
    run() {
      // We need to be executing within a strand to perform async operations
      // on the I/O objects in this Session. Although not strictly necessary
      // for single-threaded contexts, this example code is written to be
      // thread-safe by default.
      net::dispatch(_stream.get_executor(),
                    boost::beast::bind_front_handler(&http_session::on_run,
                                                     this->shared_from_this()));
    }

    virtual void
    on_run() {
      // Set the timeout.
      _stream.expires_after(std::chrono::seconds(30));
      // We need to be executing within a strand to perform async operations
      // on the I/O objects in this session. Although not strictly necessary
      // for single-threaded contexts, this example code is written to be
      // thread-safe by default.
      net::dispatch(_stream.get_executor(),
                    boost::beast::bind_front_handler(&http_session::do_read,
                                                     this->shared_from_this()));
    }

    void
    do_read() {
      // Make the request empty before reading,
      // otherwise the operation behavior is undefined.
      _req = {};

      // Set the timeout.
      _stream.expires_after(std::chrono::seconds(30));

      // Read a request
      boost::beast::http::async_read(
          _stream, _buffer, _req,
          boost::beast::bind_front_handler(&http_session::on_read,
                                           this->shared_from_this()));
    }

    void
    on_read(boost::beast::error_code ec, std::size_t bytes_transferred) {
      boost::ignore_unused(bytes_transferred);

      // This means they closed the connection
      if (ec == boost::beast::http::error::end_of_stream) return do_close();

      if (ec) return (_error_handler(ec, "read"));

      // Send the response
      _request_handler(_doc_root, std::move(_req), *this);
    }

    void
    on_write(bool                     close,
             boost::beast::error_code ec,
             std::size_t              bytes_transferred)
    {
      boost::ignore_unused(bytes_transferred);

      if (ec) return (_error_handler(ec, "write"));

      if (close) {
        // This means we should close the connection, usually because
        // the response indicated the "Connection: close" semantic.
        return do_close();
      }

      // We're done with the response so delete it
      _res = nullptr;

      // Read another request
      do_read();
    }

    void
    do_close() {
      // Set the timeout.
      _stream.expires_after(std::chrono::seconds(30));

      boost::beast::error_code ec;
      _stream.socket().shutdown(boost::asio::socket_base::shutdown_send, ec);

      if (ec) { _error_handler(ec, "close"); }
      // At this point the connection is closed gracefully
    }

    void on_shutdown(boost::beast::error_code ec) {
      if (ec) {
        _error_handler(ec, "shutdown");
        _stream.socket().shutdown(boost::asio::socket_base::shutdown_send, ec);
        _stream.socket().close();
        return _stream.close();
      }
      _stream.socket().shutdown(boost::asio::socket_base::shutdown_send, ec);
    }

    protected:

    virtual void
    async_send(boost::beast::http::response<boost::beast::http::dynamic_body>&& msg) override
    {
      generic_async_send(std::move(msg));
    }

    virtual void
    async_send(boost::beast::http::response<boost::beast::http::string_body>&& msg) override
    {
      generic_async_send(std::move(msg));
    }

    virtual void
    async_send(boost::beast::http::response<boost::beast::http::file_body>&& msg) override
    {
      generic_async_send(std::move(msg));
    }

    virtual void
    async_send(boost::beast::http::response<boost::beast::http::empty_body>&& msg) override
    {
      generic_async_send(std::move(msg));
    }

    template <class ResponseType> void
    generic_async_send(ResponseType&& msg) {
      // The lifetime of the message has to extend
      // for the duration of the async operation so
      // we use a shared_ptr to manage it.
      auto sp = std::make_shared<ResponseType>(std::move(msg));

      // Store a type-erased version of the shared
      // pointer in the class to keep it alive.
      _res = sp;

      // Write the response
      boost::beast::http::async_write(
          _stream,
          *sp,
          boost::beast::bind_front_handler(&http_session::on_write,
                                           this->shared_from_this(),
                                           sp->need_eof()));
    }
  };

  // Accepts incoming connections and launches the sessions
  template <class Session         = http_session<http_request_handler<http_regex_router>, http_error_handler>,
            class ErrorHandler    = http_error_handler>
  class  http_listener : public std::enable_shared_from_this<http_listener<Session, ErrorHandler>> {

   public:

    http_listener(net::io_context&                    ioc,
                  std::shared_ptr<net::ssl::context>  ssl_context,
                  boost::asio::ip::tcp::endpoint      endpoint,
                  const std::filesystem::path&        doc_root)
         : _ioc         { ioc         },
           _ssl_context { ssl_context },
           _acceptor    { ioc         },
           _doc_root    { doc_root    }
    {
      boost::beast::error_code ec;

      // Open the acceptor
      _acceptor.open(endpoint.protocol(), ec);
      if (ec) {
        _error_handler(ec, "open");
        return;
      }

      // Allow address reuse
      _acceptor.set_option(net::socket_base::reuse_address(true), ec);
      if (ec) {
        _error_handler(ec, "set_option");
        return;
      }

      // Bind to the server address
      _acceptor.bind(endpoint, ec);
      if (ec) {
        _error_handler(ec, "bind");
        return;
      }

      // Start listening for connections
      _acceptor.listen(net::socket_base::max_listen_connections, ec);
      if (ec) {
        _error_handler(ec, "listen");
        return;
      }
    }

    // Start accepting incoming connections
    void run() {
      do_accept();
    }

   private:

    void
    do_accept() {
      // The new connection gets its own strand
      _acceptor.async_accept(
          net::make_strand(_ioc),
          boost::beast::bind_front_handler(&http_listener::on_accept, this->shared_from_this()));
    }

    void
    on_accept(boost::beast::error_code ec, boost::asio::ip::tcp::socket socket) {
      if (ec) {
        _error_handler(ec, "accept");
        do_close(socket);
      }
      else {
        // Create the Session and run it
        std::make_shared<Session>(std::move(socket), _ssl_context, _doc_root)->run();
      }

      // Accept another connection
      do_accept();
    }

    void
    do_close(boost::asio::ip::tcp::socket& socket) {
      // Set the timeout.

      try {
        socket.shutdown(boost::asio::socket_base::shutdown_type::shutdown_send);
        socket.close();
      }
      catch (const boost::system::system_error& err) {
        _error_handler(err.code(), "shutdown");
      }
    }

    net::io_context&                    _ioc;
    std::shared_ptr<net::ssl::context>  _ssl_context;
    boost::asio::ip::tcp::acceptor      _acceptor;
    std::filesystem::path               _doc_root;
    ErrorHandler                        _error_handler;
  };

  template <class Router   = http_regex_router,
            class Listener = http_listener<http_session<http_request_handler<Router>, http_error_handler>>>
  class  http_server {

    public:

    http_server(const std::string&            address  = "127.0.0.1",
                std::uint16_t                 port     = 8080,
                const std::filesystem::path&  rootpath = "/") :
      _address{ address },
      _port{ port     },
      _root{ rootpath }
    { }

    void
    start(std::size_t number_of_threads) {
      if (!_ioc) {
        _ioc = std::make_unique<net::io_context>(number_of_threads);
        create_new_listener();
      }

      _listener->run();
      _threads.reserve(number_of_threads);
      for (auto ii = 0ull; ii < number_of_threads - 1; ii++) {
        _threads.emplace_back([this] { _ioc->run(); });
      }
      _ioc->run();
    }

    void
    stop() {
      if (_ioc) {
        _ioc->stop();

        for (auto& t : _threads) {
          t.join();
        }

        _ioc.reset();
      }
    }

    void
    set_root(const std::filesystem::path& root) {
      _root = root;
    }

    const std::shared_ptr<Listener>&
    listener() const {
      return _listener;
    }

    http_server&
    create_new_listener() {
      _listener =
        std::make_shared<Listener>(*_ioc,
                                   _ssl_context,
                                   boost::asio::ip::tcp::endpoint{ boost::asio::ip::make_address(_address), _port },
                                   _root);
      return *this;
    }

    http_server&
    set_ssl_context(net::ssl::context* context) {
      if (context) {
        _ssl_context = std::shared_ptr<net::ssl::context>(context);
      }
      return *this;
    }

    private:

    std::filesystem::path               _root;
    std::string                         _address;
    std::uint16_t                       _port;
    std::unique_ptr<net::io_context>    _ioc;
    std::shared_ptr<net::ssl::context>  _ssl_context;
    std::vector<std::thread>            _threads;
    std::shared_ptr<Listener>           _listener;
  };

  template <class Router>
  using https_server =
      http_server<Router,
                  http_listener<https_session<http_request_handler<Router>, http_error_handler>>>;
}

