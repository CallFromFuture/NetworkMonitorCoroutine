#pragma once

#include <string>
#include <memory>
#include <functional>

using namespace std;


#include <boost/asio.hpp>

namespace proxy_server{

	using boost::asio::ip::tcp;
	using boost::asio::awaitable;
	using boost::asio::co_spawn;
	using boost::asio::detached;
	using boost::asio::redirect_error;
	using boost::asio::use_awaitable;



class display_filter
{
public:
	void display(shared_ptr<string> req_data, shared_ptr<string> rsp_data);

	void _temp_display(shared_ptr<const string> data);

	int display(shared_ptr<const string> req_data);

	awaitable<int> display_breakpoint_req(shared_ptr<string> req_data);//����Ҫ��ʾ����
	void update_display_req(int id, shared_ptr<const string> req_data);
	void update_display_rsp(int id, shared_ptr<const string> rsp_data);
	void update_display_error(int id, shared_ptr<const string> rsp_data);
	awaitable<int> display_breakpoint_rsp(int update_id, shared_ptr<string> rsp_data);

	//����Ҫ��ʾ����
};

}


