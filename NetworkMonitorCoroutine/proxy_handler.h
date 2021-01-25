#pragma once
/*
* 
* �����м��
* ���ڴ���ϵ����ʾ
* 
* ��ȡ���Կͻ��˵�������ת��



*/


#include <memory>
#include <string>
using namespace std;

#include "connection.hpp"

#include "breakpoint_manager.h"
#include "display_filter.h"
#include "client_unit.h"

namespace proxy_server{

	using boost::asio::ip::tcp;
	using boost::asio::awaitable;
	using boost::asio::co_spawn;
	using boost::asio::detached;
	using boost::asio::use_awaitable;
	namespace this_coro = boost::asio::this_coro;

class http_proxy_handler
{
public:
	http_proxy_handler(const http_proxy_handler&) = delete;
	http_proxy_handler& operator=(const http_proxy_handler&) = delete;

	http_proxy_handler(breakpoint_manager& bp_mgr,
		display_filter& disp_fil, shared_ptr<client_unit> _client);

	//unified entrypoint
	awaitable<connection_behaviour> send_message(shared_ptr<string> msg,bool with_ssl,
		bool force_old_conn = false); //���һ����������chunked data����������Ӷ�ʧ��ֱ��ʧ��

	//receive ����Ҫָ���Ƿ�ʹ�þ����ӣ���Ϊ�Ͽ�ֱ��ʧ��
	awaitable<connection_behaviour> receive_message(shared_ptr<string>& rsp, 
		bool with_ssl, bool chunked_body = false);

	connection_behaviour handle_error(shared_ptr<string> result);


private:


	bool _process_header(shared_ptr<string> data, shared_ptr<string> result);

	breakpoint_manager& _breakpoint_manager;
	display_filter& _display_filter;
	shared_ptr<client_unit> _client;
	
	shared_ptr<string> host;


	int _update_id = -2;

};

}



//proxy_handler.cpp
