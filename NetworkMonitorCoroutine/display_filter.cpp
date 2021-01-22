#include "display_filter.h"

#include <cstdio>
#include <iostream>
using namespace std;

#include "common_functions.h"
using namespace common;

namespace proxy_server {

	//��Ҫ���̷���
	void display_filter::display(shared_ptr<string> req_data, shared_ptr<string> rsp_data)
	{
		//����
		cout << *req_data << endl;
		cout << "==============================" << endl;
		cout << *rsp_data << endl;

	}

	void display_filter::_temp_display(const shared_ptr<string>& data) {
		shared_ptr<string> header(new string(""));
		shared_ptr<string> body(new string(""));

		size_t _ignore_pos;
		auto _type = _chunked_integrity_check(data, _ignore_pos);
		if (_type == chunked || _type == intact) {
			cout << *data << endl;
			return;
		}


		cout << "[HEADER PART]\n";
		if (!split_request(data, header, body)) {
			cout << "header integrity compromised" << endl;
			return;
		}
		cout << *header << "\n\n[BODY PART]\n";
		string _content_type = get_header_value(header, "content-type");

		if (_content_type.find("text") == string::npos) {//16����
			cout << memory2hex_string(body)->substr(0,333)<< "...\n\n";
		}
		else {
			cout << *body << "\n\n";
		}
	}

	int display_filter::display(const shared_ptr<string>& req_data) //return update_id
	{
		cout << "[Request Data]\n";

		_temp_display(req_data);

		cout << "==============================" << endl;
		
		return 0;
	}






	awaitable<int> display_filter::display_breakpoint_req(shared_ptr<string> req_data)
	{

		co_return 0;
	}

	void display_filter::update_display_req(int id, const shared_ptr<string>& req_data)
	{
		cout << "[Request Data](supplementary)\n";
		_temp_display(req_data);
	}

	void display_filter::update_display_rsp(int id, const shared_ptr<string>& rsp_data)
	{
		cout << "[Response Data]\n";
		_temp_display(rsp_data);
	}

	void display_filter::update_display_error(int id, const shared_ptr<string>& rsp_data)
	{
		cout << "[ERROR]\n";
		cout << *rsp_data << endl;
	}


	awaitable<int> display_filter::display_breakpoint_rsp(int update_id,
		shared_ptr<string> rsp_data)//���¾���ʾ
	{
		co_return 0;
	}



}
