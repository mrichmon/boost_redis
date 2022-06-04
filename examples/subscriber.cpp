/* Copyright (c) 2018-2022 Marcelo Zimbres Silva (mzimbres@gmail.com)
 *
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE.txt)
 */

#include <vector>
#include <string>
#include <vector>
#include <iostream>
#include <boost/asio.hpp>
#include <aedis/aedis.hpp>
#include <aedis/src.hpp>

namespace net = boost::asio;
namespace generic = aedis::generic;
using aedis::redis::command;
using aedis::generic::request;
using tcp_socket = net::use_awaitable_t<>::as_default_on_t<net::ip::tcp::socket>;
using connection = generic::connection<command, tcp_socket>;
using response_type = std::vector<aedis::resp3::node<std::string>>;

/* In this example we send a subscription to a channel and start
 * reading server side messages indefinitely.
 *
 * After starting the example you can test it by sending messages with
 * redis-cli like this
 *
 *    $ redis-cli -3
 *    127.0.0.1:6379> PUBLISH channel1 some-message
 *    (integer) 3
 *    127.0.0.1:6379>
 *
 * The messages will then appear on the terminal you are running the
 * example.
 */
net::awaitable<void> reader(std::shared_ptr<connection> db)
{
   request<command> req;
   req.push(command::subscribe, "channel");
   co_await db->async_exec(req);

   for (response_type resp;;) {
      auto n = co_await db->async_read_push(generic::adapt(resp));
      std::cout
         << "Size: "   << n << "\n"
         << "Event: "   << resp.at(1).value << "\n"
         << "Channel: " << resp.at(2).value << "\n"
         << "Message: " << resp.at(3).value << "\n"
         << std::endl;

      resp.clear();
   }
}

auto handler = [](auto ec, auto...)
   { std::cout << ec.message() << std::endl; };

int main()
{
   net::io_context ioc;
   auto db = std::make_shared<connection>(ioc);
   net::co_spawn(ioc, reader(db), net::detached);
   db->async_run(handler);
   ioc.run();
}
