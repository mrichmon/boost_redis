/* Copyright (c) 2018-2022 Marcelo Zimbres Silva (mzimbres@gmail.com)
 *
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE.txt)
 */

#include <tuple>
#include <string>
#include <iostream>

#include <boost/asio.hpp>
#include <aedis.hpp>

// Include this in no more than one .cpp file.
#include <aedis/src.hpp>

namespace net = boost::asio;
using aedis::adapt;
using aedis::resp3::request;
using connection = aedis::connection<net::ip::tcp::socket>;

auto const logger = [](auto ec, auto...)
   { std::cout << ec.message() << std::endl; };

auto main() -> int
{
   try {
      request req;
      req.get_config().cancel_on_connection_lost = true;
      req.push("HELLO", 3);
      req.push("PING");
      req.push("QUIT");

      std::tuple<aedis::ignore, std::string, aedis::ignore> resp;

      net::io_context ioc;
      net::ip::tcp::resolver resv{ioc};
      auto const endpoints = resv.resolve("127.0.0.1", "6379");
      connection conn{ioc};
      net::connect(conn.next_layer(), endpoints);
      conn.async_exec(req, adapt(resp),logger);
      conn.async_run({}, logger);
      ioc.run();

      std::cout << std::get<1>(resp) << std::endl;
   } catch (std::exception const& e) {
      std::cerr << "Error: " << e.what() << std::endl;
   }
}
