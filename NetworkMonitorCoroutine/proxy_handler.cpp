
#include "proxy_handler.h"

#include "common_functions.h"
#include <algorithm>
#include <list>
using namespace common;

namespace proxy_tcp {

	



	http_proxy_handler::http_proxy_handler(
		breakpoint_manager& bp_mgr,
		display_filter& disp_fil,
		shared_ptr<client_unit> _client)
		: _breakpoint_manager(bp_mgr),
		_display_filter(disp_fil),
		_client(_client)
	{
		_session_info.reset();
	}

	connection_behaviour http_proxy_handler::handle_error(shared_ptr<string> result)//TODO
	{
		*result = "HTTP/1.1 400 Bad Request\r\n\r\n";

		_session_info->raw_rsp_data = make_shared<string>(*result);
		cout << *_session_info->new_data << endl;
		return respond_and_close;
	}

	awaitable<connection_behaviour> http_proxy_handler::send_message(shared_ptr<string> msg,
		bool with_ssl, bool force_old_conn,bool request_end)
	{
		host.reset(new string(""));

		
		//һ������connect method

		//TODO connection ��Ϊchunked body �Ͳ�Ҫ�ٴ��޸������keepalive ��־��
		bool _keep_alive = true;


		shared_ptr<string> _modified_data(new string(""));
		if (!_session_info) {//˵����һ��ȫ�µ�����
			
			_keep_alive = _process_header(msg, _modified_data);//������host���޸�

			if (_modified_data->size() == 0) {
				co_return respond_error;
			}

			_session_info = make_shared<session_info>();
			_session_info->new_data = _modified_data;
			_session_info->raw_req_data = _modified_data;
			_session_info->proxy_handler_ptr = shared_from_this();
			if (_breakpoint_manager.check(_session_info,true)) {//ͨ������ͷ������ �Ժ���˵body���°� TODO
				//�ϵ�
				_session_info->send_behaviour = intercept;
			}
			else {//����ʾ 
				_session_info->send_behaviour = pass;
				
			}
			_display_filter.display(_session_info);
		}
		else {//��ĳ�εĺ��������ֱ�Ӹ���
			_session_info->raw_req_data->append(*msg);
			_modified_data = msg;
			_session_info->new_data = _modified_data;
			_display_filter.update_display_req(_session_info);
			
		}

		

		if (request_end) {
			_display_filter.complete_req(_session_info);
		}

		connection_behaviour _behaviour = respond_and_keep_alive;

		shared_ptr<string> _error_msg = make_shared<string>("");

		if (_session_info->send_behaviour == intercept) {
			if (request_end) {
				size_t retry_count = 0;
				auto ex = co_await boost::asio::this_coro::executor;
				boost::asio::steady_timer timer(ex);
				while (retry_count < timeout 
					&& _session_info->send_behaviour == intercept) {
					timer.expires_after(std::chrono::milliseconds(1000));
					co_await timer.async_wait(use_awaitable);
				}
			}
		}
		

		switch (_session_info->send_behaviour) {
		case pass:
			_behaviour = co_await _client->send_request(*host, *_modified_data, _error_msg, with_ssl, force_old_conn);
			break;
		case pass_after_intercept:
			_behaviour = co_await _client->send_request(*host, *_session_info->raw_req_data, _error_msg, with_ssl, force_old_conn);
			_session_info->send_behaviour = pass;
			break;
		case drop:
			_session_info.reset();
			_client->disconnect();
			co_return connection_behaviour::ignore;
		case intercept://not end of request
			break;
			
		}
		

		if (_behaviour == respond_error) {
			shared_ptr<string> complete_error_msg = make_shared<string>("proxy_handler::send_message::_client::send_request ERROR : ");
			complete_error_msg->append(*_error_msg);
			_session_info->new_data = complete_error_msg;
			_display_filter.update_display_error(_session_info);
			co_return respond_error;
		}
		else {
			
			if (_keep_alive)
				co_return respond_and_keep_alive;
			else
				co_return respond_and_close;

		}



		
	}

	awaitable<connection_behaviour> http_proxy_handler::receive_message(
		shared_ptr<string>& rsp, bool with_ssl, bool chunked_body)
	{
		//��Ȼ��_session_info
		//assert(!_session_info)

		connection_behaviour _behaviour;
		_behaviour = co_await _client->receive_response(rsp);
		if (_behaviour == respond_error) {
			shared_ptr<string> complete_error_msg = make_shared<string>("proxy_handler::receive_message::_client::receive_response ERROR : ");
			complete_error_msg->append(*rsp);
			_session_info->new_data = complete_error_msg;
			_display_filter.update_display_error(_session_info);
			co_return respond_error;
		}

		_session_info->new_data = rsp;

		if (!chunked_body) {//response begin
			
			
			_session_info->raw_rsp_data = rsp;
			
			if (_breakpoint_manager.check(_session_info,false)) {
				_session_info->receive_behaviour = intercept;//�ϵ�
			}
			else {
				_session_info->receive_behaviour = pass;
			}
			
			_display_filter.display_rsp(_session_info);
		

		}
		else {
			_session_info->raw_rsp_data->append(*rsp);
			_display_filter.update_display_rsp(_session_info);
		}

		

		
		if (_behaviour != keep_receiving_data) {
			_display_filter.complete_rsp(_session_info);
		}

		if (_session_info->receive_behaviour == intercept) {
			if (_behaviour != keep_receiving_data) {
				size_t retry_count = 0;
				auto ex = co_await boost::asio::this_coro::executor;
				boost::asio::steady_timer timer(ex);
				while (retry_count < timeout
					&& _session_info->receive_behaviour == intercept) {
					timer.expires_after(std::chrono::milliseconds(1000));
					co_await timer.async_wait(use_awaitable);
				}
			}
		}


		switch (_session_info->receive_behaviour) {
		case pass:
			break;
		case pass_after_intercept:
			rsp = _session_info->raw_rsp_data;
			_session_info->receive_behaviour = pass;
			break;
		case drop:
			_session_info.reset();
			_client->disconnect();
			co_return connection_behaviour::ignore;
		case intercept://not end of response
			rsp.reset(new string(""));//TODO: connection ��rspΪ����д��
			break;
		}

		if(_behaviour != keep_receiving_data){
			
			_session_info.reset();
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
			//"upgrade",
			"accept-encoding"//TODO: ���gzip, br, deflate���ټӻ�ȥ��
			//"transfer-encoding","te","trailers"//����ԭ��ת������Ҫͽ���鷳
		};


		//find connection and host:

		shared_ptr<vector<string>> header_vec_ptr
			= string_split(*header, "\r\n");

		string _conn_value = get_header_value(header_vec_ptr, "proxy-connection");
		if(_conn_value.size()==0)
			_conn_value = get_header_value(header_vec_ptr, "connection");

		*host = get_header_value(header_vec_ptr, "host");


		//get to be removed field
		bool websocket_upgrade = false;
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
				else if (t == "upgrade") {
					if (string("websocket")
						!= get_header_value(header_vec_ptr, "upgrade")) {

						to_be_removed_header_list.emplace_back(t);
					}
					else {
						websocket_upgrade = true;
					}
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
			result->append("Connection: keep-alive");//Ĭ��keep-alive
		else
			result->append("Connection: close");

		if (websocket_upgrade) {
			result->append(", upgrade");
			_keep_alive = true;
		}

		result->append("\r\n");//header::connection end

		result->append("\r\n");//header end
		//append body
		result->append(*body);

		return _keep_alive;
	}

	



	

}