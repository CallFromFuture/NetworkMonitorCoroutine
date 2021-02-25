#pragma once

#include <ctime>

#include <boost/asio.hpp>


using namespace boost::asio::ip;

#include "connection.hpp"

#include "proxy_handler.h"
#include "io_context_pool.h"
#include "breakpoint_manager.h"


#include "display_filter.h"

#include <iostream>
#include <cstdio>
#include <string>

#if defined(BOOST_ASIO_ENABLE_HANDLER_TRACKING)
# define use_awaitable \
  boost::asio::use_awaitable_t(__FILE__, __LINE__, __PRETTY_FUNCTION__)
#endif






namespace proxy_tcp {

struct config {
	breakpoint_filter req_filter;
	breakpoint_filter rsp_filter;

	int column_width[sizeof(_table_header_name) / sizeof(const char*)] = {
	20,200,35,45,200,40,70
	};

	bool ssl_decrypt = true;
};

class proxy_server
{
public:
	//��ֹ�����͸�ֵ
	proxy_server(const proxy_server&) = delete;
	proxy_server& operator=(const proxy_server&) = delete;

	explicit proxy_server(const string& address, const string& port, size_t io_context_pool_size);

	config& get_config_ref() { return _config; }

	

	void start();//�첽����
	display_filter* get_display_filter() { return &_display_filter; }

private:

	// Initiate an asynchronous accept operation.
	awaitable<void> _listener();

	void _stop();
	io_context_pool _io_context_pool;
	
	// The signal_set is used to register for process termination notifications.
	boost::asio::signal_set _signals;

	// Acceptor used to listen for incoming connections.
	tcp::acceptor _acceptor;

	boost::asio::io_context& _io_context;

	typedef shared_ptr<connection> proxy_conn_ptr;

	proxy_conn_ptr _new_proxy_conn;

	shared_ptr<http_proxy_handler> _new_proxy_handler;

	breakpoint_manager _breakpoint_manager;

	display_filter _display_filter;

	config _config;//������ʵ�ʶ�ȡ
};

}

