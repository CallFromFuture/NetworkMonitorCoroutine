
#include "proxy_handler.h"

#include "common_functions.h"
#include <algorithm>
#include <list>
using namespace common;

namespace proxy_server {

	



	http_proxy_handler::http_proxy_handler(
		breakpoint_manager& bp_mgr,
		display_filter& disp_fil,
		shared_ptr<client_unit> _client)
		: _breakpoint_manager(bp_mgr),
		_display_filter(disp_fil),
		_client(_client)
	{
	}

	connection_behaviour http_proxy_handler::handle_error(shared_ptr<string> result)//TODO
	{
		*result = "HTTP/1.1 400 Bad Request\r\n\r\n";

		return respond_and_close;
	}

	awaitable<connection_behaviour> http_proxy_handler::send_message(shared_ptr<string> msg, bool with_ssl, bool force_old_conn)
	{
		host.reset(new string(""));


		//��connect
		shared_ptr<string> _modified_data(new string(""));
		bool _keep_alive = _process_header(msg, _modified_data);//������host���޸�

		if (_modified_data->size() == 0) {
			co_return respond_error;
		}


		if (_update_id == -2) {//˵����һ��ȫ�µ����󣬲���ĳ��chunked�ĺ���
			if (_breakpoint_manager.check(*_modified_data)) {
				//�ϵ�
				//_modified_data �ڴ˴��ǿ��ܱ��޸ĵ�
				_update_id = co_await _display_filter.display_breakpoint_req(_modified_data);
				if (_update_id == -1) {//������ִ��
					_update_id = -2;//reset to default
					_client->disconnect();
					co_return connection_behaviour::ignore;
				}
			}
			else {//����ʾ 
				_update_id = _display_filter.display(_modified_data);
			}
		}
		else {//��ĳ�εĺ��������ֱ�Ӹ���
			_display_filter.update_display_req(_update_id, _modified_data);
		}

		connection_behaviour _behaviour;


		shared_ptr<string> _error_msg = make_shared<string>("");
		
		_behaviour = co_await _client->send_request(*host, *_modified_data,_error_msg, with_ssl, force_old_conn);

		if (_behaviour == respond_error) {
			shared_ptr<string> complete_error_msg = make_shared<string>("proxy_handler::send_message::_client::send_request ERROR : ");
			complete_error_msg->append(*_error_msg);
			_display_filter.update_display_error(_update_id, complete_error_msg);
			co_return _behaviour;
		}
		else {
			if (_keep_alive)
				co_return respond_and_keep_alive;
			else
				co_return respond_and_close;

		}



		
	}

	awaitable<connection_behaviour> http_proxy_handler::receive_message(shared_ptr<string>& rsp, bool with_ssl, bool chunked_body)
	{
		//��Ȼ��update_id
		//assert(update_id>=0)
		connection_behaviour _behaviour;
		_behaviour = co_await _client->receive_response(rsp);
		if (_behaviour == respond_error) {
			shared_ptr<string> complete_error_msg = make_shared<string>("proxy_handler::send_message::_client::send_request ERROR : ");
			complete_error_msg->append(*rsp);
			_display_filter.update_display_error(_update_id, complete_error_msg);
			co_return _behaviour;
		}

		if (!chunked_body && _breakpoint_manager.check(*rsp)) {
			//�ϵ�
			if (-1 == co_await _display_filter.display_breakpoint_rsp(_update_id, rsp)) {//result �ڴ˴��ǿ��ܱ��޸ĵ�
				//�������
				_update_id = -2;
				_client->disconnect();
				co_return connection_behaviour::ignore;//ignore Ҫ���client������
			}
			//else �������κ��£���Ϊdisplay_breakpoint_rsp �Ѿ�������˺�����ʾ����

		}
		else {//�ֶ�������ʾ
			_display_filter.update_display_rsp(_update_id, rsp);
		}

		
		if(_behaviour != keep_receiving_data){
			//reset _update_id
			_update_id = -2;
		}

		co_return _behaviour;
		
	}



	//=====================common functions=============================

	


	//return is keep_alive
	bool http_proxy_handler::_process_header(shared_ptr<string> data, shared_ptr<string> result) {
		bool _keep_alive = true;

		shared_ptr<string> header(new string(""));
		shared_ptr<string> body(new string(""));

		if (!split_request(data, header, body))
			return false;


		list<string> to_be_removed_header_list{//Сд
			"proxy-authenticate",
			"proxy-connection",
			"connection",
			//"transfer-encoding","te","trailers"//����ԭ��ת������Ҫͽ���鷳
			"upgrade",
			"accept-encoding"//TODO: ���gzip, br, deflate���ټӻ�ȥ��
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
					to_be_removed_header_list.emplace_back(t);
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
			auto list_iter = to_be_removed_header_list.begin();
			while (list_iter != to_be_removed_header_list.end()) {
				if (temp.find(*list_iter) == 0) {
					auto temp_iter = list_iter;
					list_iter++;
					to_be_removed_header_list.erase(temp_iter);
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

	



	

}