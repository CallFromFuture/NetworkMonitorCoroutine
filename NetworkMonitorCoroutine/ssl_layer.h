#pragma once
#include <memory>
#include <string>
using namespace std;

#include <boost/asio.hpp>

/*
* POOR PERFORMANCE WARNING
* ��ʵ�ֲ���socket ����boost ssl server ��ȡ����
* 
* �����ܳ�Ϊƿ�����滻����
* 
*/

namespace proxy_server {

	using boost::asio::ip::tcp;
	using boost::asio::awaitable;
	using boost::asio::co_spawn;
	using boost::asio::detached;
	using boost::asio::use_awaitable;
	namespace this_coro = boost::asio::this_coro;

class ssl_layer
{
public:
	ssl_layer(const ssl_layer&) = delete;
	ssl_layer& operator=(const ssl_layer&) = delete;

	ssl_layer(){}

	awaitable<bool> _do_handshake(const string& host);
	//shared_ptr<string> ssl_decrypt(const char * data, size_t length);

	awaitable<void> decrypt_append(shared_ptr<string> str,const char* data, size_t length);

	awaitable<shared_ptr<string>> ssl_encrypt(const string& data);

private:


};

}

