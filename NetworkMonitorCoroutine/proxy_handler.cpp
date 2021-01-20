
#include "proxy_handler.h"

#include "common_functions.h"
#include <algorithm>
#include <list>
using namespace common;

namespace proxy_server {

	
	typedef enum {
		transparency_proxy,
		default_proxy
	}proxy_type;



	http_proxy_handler::http_proxy_handler(
		breakpoint_manager& bp_mgr,
		display_filter& disp_fil,
		shared_ptr<client_unit> _client)
		: _breakpoint_manager(bp_mgr),
		_display_filter(disp_fil),
		_client(_client)
	{
	}




	awaitable<connection_behaviour> http_proxy_handler::handle_request(
		shared_ptr<string> data, shared_ptr<string> result)
	{
		host.reset(new string(""));


		request_type _type = _get_request_type(*data);
		if (_type == _CONNECT) {
			*result = "HTTP/1.1 200 Connection Established\r\n\r\n";
			//_display_filter.display(data, result);
			co_return connection_behaviour::respond_as_tunnel;
		}

		//��connect
		shared_ptr<string> _modified_data(new string(""));
		bool _keep_alive = _process_header(data, _modified_data);

		if (_modified_data->size() == 0) {
			co_return handle_error(result);
		}

		if (_keep_alive)
			co_return co_await _handle_request(_modified_data, result, respond_and_keep_alive);
		else
			co_return co_await _handle_request(_modified_data, result, respond_and_close);



	}

	awaitable<connection_behaviour> http_proxy_handler::handle_request_as_tunnel(shared_ptr<string> data, shared_ptr<string> result)
	{

		//TODO:����data
		auto _behaviour = co_await _handle_request(data, result, respond_as_tunnel);
		if (_behaviour == ignore)
			co_return _behaviour;
		//TODO:����
		//TODO:д��result

		co_return _behaviour;
	}

	awaitable<connection_behaviour> http_proxy_handler::handle_handshake(shared_ptr<string> data, shared_ptr<string> result)
	{
		//_display_filter.display(data, result);
		cout << "DEBUG::handshake_refused" << endl;
		co_return connection_behaviour::ignore;
	}

	connection_behaviour http_proxy_handler::handle_error(shared_ptr<string> result)//TODO
	{
		*result = "HTTP/1.1 400 Bad Request\r\n\r\n";

		return respond_and_close;
	}






	awaitable<connection_behaviour> http_proxy_handler::_handle_request(shared_ptr<string> data, shared_ptr<string> result, connection_behaviour _behaviour)
	{

		int update_id = -2;
		if (_breakpoint_manager.check(*data)) {
			//_modified_data �ڴ˴��ǿ��ܱ��޸ĵ�
			update_id = co_await _display_filter.display_breakpoint_req(data);
			if (update_id == -1) {//������ִ��
				co_return connection_behaviour::ignore;
			}
		}
		else {
			update_id = _display_filter.display(data);
		}

		if (_behaviour == respond_as_tunnel) {//https
			co_await _client->send_request_ssl(*host,*data, result); //TODO: �˴������ж������Ƿ���� //TODO: async
		}
		else {//http
			co_await _client->send_request(*host ,*data, result);//����
		}


		if (_breakpoint_manager.check(*result)) {
			if (-1== co_await _display_filter.display_breakpoint_rsp(data, result, update_id)) {//result �ڴ˴��ǿ��ܱ��޸ĵ�
				//�������
				co_return connection_behaviour::ignore;
			}

			co_return _behaviour;

		}
		else {
			_display_filter.update_display(update_id, result);
		}

		co_return _behaviour;

	}

	request_type http_proxy_handler::_get_request_type(const string& data)
	{
		switch (data[0]) {
		case 'G'://get
			return request_type::_GET;
		case 'O'://options
			return request_type::_OPTIONS;
		case 'P'://post/put
			if (data[1] == 'O')
				return request_type::_POST;
			else
				return request_type::_PUT;
		case 'H'://head
			return request_type::_HEAD;
		case 'D'://delete
			return request_type::_DELETE;
		case 'T'://trace
			return request_type::_TRACE;
		case 'C'://connect
			return request_type::_CONNECT;
		default:
			return request_type::_GET;
		}


		return request_type::_GET;
	}


	//return is keep_alive
	bool http_proxy_handler::_process_header(shared_ptr<string> data, shared_ptr<string> result) {
		bool _keep_alive = true;

		shared_ptr<string> header(new string(""));
		shared_ptr<string> body(new string(""));

		if (!split_request(data, header, body))
			return false;


		list<string> to_be_remove_header_list{//Сд
			"proxy-authenticate",
			"proxy-connection",
			"connection",
			//"transfer-encoding","te","trailers"//����ԭ��ת������Ҫͽ���鷳
			"upgrade"
		};


		//find connection and host:

		shared_ptr<vector<string>> header_vec_ptr
			= string_split(*header, "\r\n");

		string _conn_value = get_header_value(header_vec_ptr, "proxy-connection");
		if(_conn_value.size()==0)
			_conn_value = get_header_value(header_vec_ptr, "connection");

		*host = get_header_value(header_vec_ptr, "host");


		//get to be removed field

		if (_conn_value.size() != 0) {
			auto temp_vec_ptr = string_split(_conn_value, ",");

			for (auto e : *temp_vec_ptr) {
				
				string t = string_trim(e);
				if (t.size() == 0)
					continue;
				if (t == "close") {
					_keep_alive = false;
				}
				else if (t == "keep-alive") {
					_keep_alive = true;
				}
				else {
					to_be_remove_header_list.emplace_back(t);
				}
			}
		}
		




		//first process the first line, remove the host
		string first_line = (*header_vec_ptr)[0];

		size_t pos1 = first_line.find("://");
		if (pos1 != string::npos) {//��͸������
			size_t pos0 = first_line.find(" ");
			size_t pos2 = first_line.find("/", pos1 + 3);
			(*header_vec_ptr)[0] = first_line.substr(0, pos0 + 1) +
				first_line.substr(pos2, first_line.size() - pos2);
		}
		//͸������ʲô��������



		for (auto header : *header_vec_ptr) {//�������ͷ��
			string temp;
			temp.resize(header.size());
			transform(header.begin(), header.end(), temp.begin(), ::tolower);
			bool skip = false;
			auto list_iter = to_be_remove_header_list.begin();
			while (list_iter != to_be_remove_header_list.end()) {
				if (temp.find(*list_iter) == 0) {
					auto temp_iter = list_iter;
					list_iter++;
					to_be_remove_header_list.erase(temp_iter);
					skip = true;
					break;
				}
				else {
					list_iter++;
				}
			}
			if (!skip) {
				result->append(header);
				result->append("\r\n");
			}

		}

		//��ӵ���һ����ͷ��

		if (CLIENT_UNIT_KEEP_ALIVE)
			result->append("Connection: keep-alive\r\n");//Ĭ��keep-alive
		else
			result->append("Connection: close\r\n");


		result->append("\r\n");//header end
		//append body
		result->append(*body);
		return _keep_alive;
	}

	/*

	//return is keep_alive
	bool http_proxy_handler::_process_header(shared_ptr<string> data, shared_ptr<string> result)
	{
		bool _keep_alive = true;
		size_t header_end_pos = data->find("\r\n\r\n");

		if (header_end_pos == string::npos)
			return false;

		string headers = data->substr(0, header_end_pos);
		shared_ptr<vector<string>> header_vec_ptr = string_split(headers, "\r\n");

		list<string> to_be_remove_header_list{//Сд
			"proxy-authenticate",
			"proxy-connection",
			"connection",
			//"transfer-encoding","te","trailers"//����ԭ��ת������Ҫͽ���鷳
			"upgrade"

		};

		//proxy_type _proxy_type = default_proxy;

		//find connection:

		string _conn_field("");
		for (auto header : *header_vec_ptr) {

			string temp;
			temp.resize(header.size());
			transform(header.begin(), header.end(), temp.begin(), ::tolower);

			if (temp.find("proxy-connection:") == 0) {//��ͨ����
				_conn_field = temp.substr(17, temp.size() - 17);//potential performance loss
				//_proxy_type = default_proxy;
				//break;
			}
			else if (temp.find("connection:") == 0) {//͸������
				_conn_field = temp.substr(11, temp.size() - 11);
				//_proxy_type = transparency_proxy;
				//break;
			}
			else if (temp.find("host:") == 0) {
				*host = string_trim(temp.substr(5, temp.size() - 5));
			}
		}

		//get to be removed field
		auto temp_vec_ptr = string_split(_conn_field, ",");

		for (auto e : *temp_vec_ptr) {
			
			string t = string_trim(e);
			if (t == "close") {
				_keep_alive = false;
			}
			else if (t == "keep-alive") {
				_keep_alive = true;
			}
			else {
				to_be_remove_header_list.emplace_back(t);
			}
		}




		//first process the first line, remove the host
		string first_line = (*header_vec_ptr)[0];
		
		size_t pos1 = first_line.find("://");
		if (pos1 != string::npos) {//��͸������
			size_t pos0 = first_line.find(" ");
			size_t pos2 = first_line.find("/", pos1 + 3);
			(*header_vec_ptr)[0] = first_line.substr(0, pos0 + 1) +
				first_line.substr(pos2, first_line.size() - pos2);
		}
		//͸������ʲô��������
		


		for (auto header : *header_vec_ptr) {//�������ͷ��
			string temp;
			temp.resize(header.size());
			transform(header.begin(), header.end(), temp.begin(), ::tolower);
			bool skip = false;
			auto list_iter = to_be_remove_header_list.begin();
			while (list_iter != to_be_remove_header_list.end()) {
				if (temp.find(*list_iter) == 0) {
					auto temp_iter = list_iter;
					list_iter++;
					to_be_remove_header_list.erase(temp_iter);
					skip = true;
					break;
				}
				else {
					list_iter++;
				}
			}
			if (!skip) {
				result->append(header);
				result->append("\r\n");
			}

		}

		//��ӵ���һ����ͷ��

		if (CLIENT_UNIT_KEEP_ALIVE)
			result->append("Connection: keep-alive\r\n");//Ĭ��keep-alive
		else
			result->append("Connection: close\r\n");
		
		
		result->append("\r\n");//header end
		//append body
		result->append(data->substr(header_end_pos + 4,
			data->size() - (header_end_pos + 4)));

		return _keep_alive;
	}

	*/



	

}