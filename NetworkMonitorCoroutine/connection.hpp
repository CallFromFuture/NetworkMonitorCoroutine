#pragma once

/*
* �������ӣ��Ͽ��Զ�����
* ����https������ȷ�����������Ժ�Ż�ת����handler
* 
*/


#include <memory>
#include <array>
#include <string>
#include <iostream>
using namespace std;


#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/bind/bind.hpp>
using namespace boost::asio::ip;

#include "common_functions.h"
using namespace common;

#include "connection_enums.h"
#include "certificate_manager.h"

namespace proxy_tcp{

	using boost::asio::ip::tcp;
	using boost::asio::awaitable;
	using boost::asio::co_spawn;
	using boost::asio::detached;
	using boost::asio::use_awaitable;

	namespace this_coro = boost::asio::this_coro;

	typedef boost::asio::ssl::stream<tcp::socket> ssl_stream;

class http_proxy_handler;

class connection : public enable_shared_from_this<connection>
{
public:
	//noncopyable, please use shared_ptr
	connection(const connection&) = delete;
	connection& operator=(const connection&) = delete;

	
	//explicit connection(tcp::socket socket,
	//	shared_ptr<http_proxy_handler> handler_ptr);

	//�ɸ�����io_context����������
	explicit connection(boost::asio::io_context& _io_context,
		shared_ptr<http_proxy_handler> handler_ptr, shared_ptr<certificate_manager> cert_mgr);

	void start();
	void stop();



	~connection() {
		if (_ssl_stream_ptr)
			_socket = nullptr;//���������ͷ�
		else if (_socket)
			delete _socket;
		//cout << "lost connection" << endl;
	}
	tcp::socket& socket(){ return *_socket; }

	
	void set_replay_mode(shared_ptr<string> raw_req_data,bool is_tunnel_conn) {
		skip_socket_rw = true;
		_whole_request = raw_req_data;
		_is_tunnel_conn = is_tunnel_conn;
	}

private:
	bool skip_socket_rw = false;
	boost::asio::io_context& _io_context;
	awaitable<void> _waitable_loop();

	awaitable<void> _async_read(bool with_ssl);
	awaitable<void> _async_write(const string& data, bool with_ssl);

	//ssl_layer _ssl_layer;

	//shared_ptr<tcp::socket> _socket;
	tcp::socket* _socket;//��������
	
	bool _keep_alive = true;

	shared_ptr<http_proxy_handler> _request_handler;

	array<char, 8192> _buffer;


	shared_ptr<string> _whole_request;
	

	bool _is_tunnel_conn = false;

	string host;

	shared_ptr<ssl_stream> _ssl_stream_ptr;
	boost::asio::ssl::context _ssl_context;

	shared_ptr<certificate_manager> _cert_mgr;

	connection_protocol _conn_protocol = http;

};




}// namespace proxy_tcp