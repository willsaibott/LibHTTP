#pragma once

#include <LibHttp/http_server.h>

class http_custom_router2 : public http::http_regex_router {
public:
  http_custom_router2();
};
