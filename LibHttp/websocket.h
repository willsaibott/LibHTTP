#pragma once


#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <filesystem>
#include <vector>
#include <sstream>
#include <type_traits>
#include "http_server.h"

// Echoes back all received WebSocket messages

namespace http {

  class LIBHTTPSHARED_EXPORT websocket_message {

    std::string _buffer_data;

    public:
    websocket_message() = default;

    websocket_message(std::string_view message) : _buffer_data{ message }
    { }

    websocket_message(boost::string_view message) : _buffer_data{ message.data(), message.size() }
    { }

    websocket_message(const boost::beast::flat_buffer& buffer)
        : _buffer_data{ reinterpret_cast<const char*>(buffer.data().data()),
                        buffer.size() }
    { }

    std::string_view
    data() const {
      return _buffer_data;
    }

    boost::asio::const_buffer
    buffer() const {
      return boost::asio::const_buffer{ _buffer_data.data(), _buffer_data.size() };
    }

    websocket_message&
    write(const std::string& message) {
      _buffer_data += message;
      return *this;
    }

    template <class type>
    inline websocket_message&
    operator<<(const type& message) {
      std::stringstream ss;
      ss << message;
      return write(ss.str());
    }

  };

  struct LIBHTTPSHARED_EXPORT echo_message_handler {

    inline websocket_message
    operator()(const websocket_message& message) const {
      websocket_message response = message;
      return response.write(" \"[echo]\"");
    }
  };

  template <class MessageHandler,
            class Stream       = boost::beast::websocket::stream<boost::beast::tcp_stream>,
            class ErrorHandler = http_error_handler>
  class LIBHTTPSHARED_EXPORT websocket_section :
      public std::enable_shared_from_this<websocket_section<MessageHandler, Stream, ErrorHandler>>

  {
  protected:
      MessageHandler                                             _message_handler;
      ErrorHandler                                               _error_handler;
      Stream                                                     _ws;
      websocket_message                                          _input_message;
      websocket_message                                          _output_message;
      boost::beast::flat_buffer                                  _buffer;
      std::filesystem::path                                      _root_path;

  public:
      // Take ownership of the socket
      explicit
      websocket_section(boost::asio::ip::tcp::socket&&              socket,
                        std::shared_ptr<boost::asio::ssl::context>  ssl_context,
                        const std::filesystem::path&                doc_root)
          : _ws{ std::move(socket) },
            _root_path{ doc_root }
      {
        boost::ignore_unused(ssl_context);
      }

      explicit
      websocket_section(Stream&&                                    stream,
                        const std::filesystem::path&                doc_root)
          : _ws{ std::move(stream) },
            _root_path{ doc_root }
      { }

      // Get on the correct executor
      void
      run() {
          // We need to be executing within a strand to perform async operations
          // on the I/O objects in this session. Although not strictly necessary
          // for single-threaded contexts, this example code is written to be
          // thread-safe by default.
          boost::asio::
            dispatch(_ws.get_executor(),
                     boost::beast::bind_front_handler(&websocket_section::on_run,
                                                      this->shared_from_this()));
      }

      // Start the asynchronous operation
      virtual void
      on_run() {
          //if constexpr (std::is_same_v<Stream, boost::beast::websocket::stream<boost::beast::tcp_stream>>) {
          // Set suggested timeout settings for the websocket
          _ws.set_option(
              boost::beast::websocket::stream_base::timeout::suggested(
                  boost::beast::role_type::server));

          // Set a decorator to change the Server of the handshake
          _ws.set_option(
            boost::beast::websocket::stream_base::decorator(
              [](boost::beast::websocket::response_type& res) {
                  res.set(boost::beast::http::field::server,
                          std::string(BOOST_BEAST_VERSION_STRING) + " websocket-server-async");
              }));

          // Accept the websocket handshake
          _ws.async_accept(
            boost::beast::bind_front_handler(&websocket_section::on_accept,
                                              this->shared_from_this()));
      }

      virtual void
      on_handshake(boost::beast::error_code ec) {
        boost::ignore_unused(ec);
      }

      void
      on_accept(boost::beast::error_code ec) {
          if(ec) {
              _error_handler(ec, "accept");
              boost::beast::get_lowest_layer(_ws).socket().shutdown(boost::asio::socket_base::shutdown_send, ec);
              boost::beast::get_lowest_layer(_ws).socket().close();
              return boost::beast::get_lowest_layer(_ws).close();
          }

          // Read a message
          do_read();
      }

      void
      do_read() {
          // Read a message into our buffer
          _ws.async_read(_buffer,
                         boost::beast::bind_front_handler(&websocket_section::on_read,
                                                          this->shared_from_this()));
      }

      void
      on_read(boost::beast::error_code ec, std::size_t bytes_transferred) {
          boost::ignore_unused(bytes_transferred);

          // This indicates that the session was closed
          if (ec == boost::beast::websocket::error::closed) {
              boost::beast::get_lowest_layer(_ws).socket().shutdown(boost::asio::socket_base::shutdown_send, ec);
              return;
          }

          if (ec) {
              _error_handler(ec, "read");
          }

          // Echo the message
          _ws.text(_ws.got_text());
          _output_message = _message_handler(websocket_message{ _buffer });
          _ws.async_write(_output_message.buffer(),
                          boost::beast::bind_front_handler(&websocket_section::on_write,
                                                           this->shared_from_this()));
      }

      void
      on_write(boost::beast::error_code ec, std::size_t bytes_transferred) {
          boost::ignore_unused(bytes_transferred);

          /*if (ec == boost::beast::error_code::)*/

          if (ec) {
              _error_handler(ec, "write");
              boost::beast::get_lowest_layer(_ws).socket().shutdown(boost::asio::socket_base::shutdown_send, ec);
              boost::beast::get_lowest_layer(_ws).socket().close();
              return boost::beast::get_lowest_layer(_ws).close();
          }

          // Clear the buffer
          _buffer.consume(_buffer.size());

          // Do another read
          do_read();
      }
  };

  template <class MessageHandler,
            class Stream       = boost::beast::websocket::stream<boost::beast::ssl_stream<boost::beast::tcp_stream>>,
            class ErrorHandler = http_error_handler>
  class websocket_ssl_section : public websocket_section<MessageHandler, Stream, ErrorHandler> {
    using base_section =  websocket_section<MessageHandler, Stream, ErrorHandler>;
  public:

    explicit
    websocket_ssl_section(boost::asio::ip::tcp::socket&&              socket,
                          std::shared_ptr<boost::asio::ssl::context>  ssl_context,
                          const std::filesystem::path&                doc_root)
        : base_section{ Stream{ std::move(socket), *ssl_context }, doc_root }
    { }

    virtual void
    on_run() override {
        // Set the timeout.
        boost::beast::get_lowest_layer(base_section::_ws).expires_after(std::chrono::seconds(30));

         // Perform the SSL handshake
        base_section::
        _ws.next_layer().async_handshake(
            boost::asio::ssl::stream_base::server,
            boost::beast::bind_front_handler(
                &base_section::on_handshake,
                this->shared_from_this()));
    }

    virtual void
    on_handshake(boost::beast::error_code ec) override {
        if(ec) {
            return base_section::_error_handler(ec, "handshake");
        }

        // Turn off the timeout on the tcp_stream, because
        // the websocket stream has its own timeout system.
        boost::beast::get_lowest_layer(base_section::_ws).expires_never();

        // Set suggested timeout settings for the websocket
        base_section::
        _ws.set_option(
            boost::beast::websocket::stream_base::timeout::
                suggested(boost::beast::role_type::server));

        // Set a decorator to change the Server of the handshake
        base_section::
        _ws.set_option(boost::beast::websocket::stream_base::decorator(
            [](boost::beast::websocket::response_type& res) {
                res.set(boost::beast::http::field::server,
                        std::string(BOOST_BEAST_VERSION_STRING) + " websocket-server-async-ssl");
                res.set(boost::beast::http::field::sec_websocket_protocol,
                        "websocket-server-async-ssl-protocol");
            }));

        // Accept the websocket handshake
        base_section::
        _ws.async_accept(
            boost::beast::bind_front_handler(
                &base_section::on_accept,
                this->shared_from_this()));
    }
  };

  template <class MessageHandler>
  using websocket_server =
      http_server<MessageHandler, http_listener<websocket_section<MessageHandler>, http_error_handler>>;

  template <class MessageHandler>
  using websocket_ssl_server =
      http_server<MessageHandler, http_listener<websocket_ssl_section<MessageHandler>, http_error_handler>>;

}
