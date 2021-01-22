#include "connection.hpp"
#include "proxy_handler.h"

#include <boost/thread.hpp>

namespace proxy_server {




connection::connection(tcp::socket socket,
	shared_ptr<http_proxy_handler> handler_ptr)
	: _socket(std::move(socket)),
	_request_handler(handler_ptr),
	_whole_request(new string(""))
{
	boost::asio::socket_base::keep_alive _ka(true);
	_socket.set_option(_ka);

}

connection::connection(boost::asio::io_context& _io_context,
	shared_ptr<http_proxy_handler> handler_ptr)
	: _socket(_io_context),
	_request_handler(handler_ptr),
	_whole_request(new string(""))
{
	boost::asio::socket_base::keep_alive _ka(true);
	_socket.set_option(_ka);

}



void connection::start()
{
	//��֤connection����
	auto self = this->shared_from_this();
	co_spawn(_socket.get_executor(),
		[self]() {
			return self->_waitable_loop();
		}, detached);

}



void connection::stop()
{
	if (_socket.is_open()) {
		boost::system::error_code ignored_ec;
		_socket.shutdown(tcp::socket::shutdown_both, ignored_ec);
		_socket.close();
	}
	
}





//test

awaitable<void> connection::_waitable_loop()
{
	/*
	* 
	* �����߼�
	* ������ϴ�ʣ�࣬�򲻶�ȡ
	* ��ȡ
	* ���������
	* ��Ϊwaitֱ��������ͷ
	* ��Ϊchunked��.....
	
	
	*/

	/*
	* 
	* ֻ�ж��Ƿ��η���
	* ��ν�����handler���ƣ��������Լ���һ���ж�
	
	*/

	cout << "executed by thread " << boost::this_thread::get_id() << endl;

	try
	{
		boost::system::error_code ec;

		bool _with_appendix = false;
		integrity_status _status= integrity_status::broken;

		while (_keep_alive) { //ѭ����д

			integrity_status last_status = _status;

			shared_ptr<string> res(new string(""));
			

			//���ϴ���һЩʣ���β�ͣ�����Ȳ��������ȼ�������ԣ������������Ϊ���һ�����Ķ�����
			if (!_with_appendix) {
				size_t bytes_transferred = co_await _socket.async_read_some(
					boost::asio::buffer(_buffer),
					boost::asio::redirect_error(use_awaitable, ec));
				if (ec) {
					if (ec != boost::asio::error::eof) {
						throw std::runtime_error("read failed");//TODO ��������
					}//TODO eof��ζ���޷�д����
					else {
						throw std::runtime_error("connection closed by peer");
					}
				}

				if (_is_tunnel_conn) {
					//TODO:ʾ��
					//TODO: ת�������õ�https��������
					//(temp_var,new_bytes_transferred) = https_decrypt_server.read(_buffer.data(), bytes_transferred);
					//_whole_request->append(temp_var, new_bytes_transferred);

					//DEBUG
					cout << _buffer.data() << endl;
					_keep_alive = false;
					continue;//DEBUG code(not complete)
				}
				else {
					_whole_request->append(_buffer.data(), bytes_transferred);
				}

			}

			

			


			//_whole_request ���ǽ������http����


			
			size_t split_pos = 0;
			if ((last_status == chunked) || (last_status == wait_chunked))
				_status = _chunked_integrity_check(_whole_request, split_pos);
			else
				_status = _http_integrity_check(_whole_request, split_pos);
			//appendix:[split_pos,_size)

			shared_ptr<string> remained_request;

			if (split_pos < _whole_request->size()) {
				remained_request.reset(new string(
					_whole_request->substr(split_pos, _whole_request->size() - split_pos)));
				_whole_request->resize(split_pos);//���ĩβ
				_with_appendix = true;
			}

			connection_behaviour _behaviour;
			switch (_status) {
			//case integrity_status::with_appendix://chunk_with_appendix�ȷָ��ٷ���
				
				
				//����û��break
			case integrity_status::chunked: //TODO:https��η���,ֻ������
				_behaviour = co_await _request_handler->
					send_message(_whole_request, _is_tunnel_conn,
						(last_status == chunked));//��һ����chunked����Ҫǿ��ʹ�þ�����

				break;
			case integrity_status::intact:
				if (_get_request_type(*_whole_request) == _CONNECT) {//connect method ��������ֱ�ӷ��أ�
					//DISPLAY IS NOT NECESSARY
					_is_tunnel_conn = true;
					*res = "HTTP/1.1 200 Connection Established\r\n\r\n";
					co_await boost::asio::async_write(_socket,
						boost::asio::buffer(*res),
						boost::asio::redirect_error(use_awaitable, ec));
					//cout << *_whole_request << endl;
					cout << "prepare for handshake" << endl;
					if (ec) {
						throw std::runtime_error("write failed");
					}
					continue;
				}
				_behaviour = co_await _request_handler->
					send_message(_whole_request, _is_tunnel_conn,
						(last_status == chunked));//��һ����chunked����Ҫǿ��ʹ�þ�����
				break;

			case integrity_status::broken:
				_behaviour = respond_error;
				break;

			case integrity_status::wait_chunked:
			case integrity_status::wait:
				continue;

				break;
			default:
				throw std::runtime_error("integrity check failed");
			}






			//reset to wait incoming data
			if (_with_appendix) {
				//��ʣһ������������
				_whole_request = remained_request;
			}
			else {
				_whole_request.reset(new string(""));
			}


			//�����Ƿ�����������ݣ��Ƿ�ֱ�ӳ�����
			switch (_behaviour) {
			case respond_and_close:
				_keep_alive = false;
				break;
			case respond_and_keep_alive:
				_keep_alive = true;
				break;
			case respond_error:
				_keep_alive = false;
				_request_handler->handle_error(res); //�ܿ죬����Ҫ�첽����
				co_await boost::asio::async_write(_socket, boost::asio::buffer(*res),
					boost::asio::redirect_error(use_awaitable, ec));
				if (ec) {
					throw std::runtime_error("write failed");
				}
				else {
					continue;//�Զ�������ѭ����
				}
				break;
			case ignore:
				_keep_alive = false;
				continue;//�Զ�������ѭ����


			case keep_receiving_data://send������Ӧ���ش�ֵ
				throw std::runtime_error("handler->send ERROR (presumably a bug)");
				break;
			}

			//�ܵ�����һ��û�г���
			if (_status == integrity_status::chunked) {
				continue;//����д�룬���������ݷ���
			}


			//ָ�����ֱ�ӱ��ˣ��˴���res����������
			_behaviour = co_await _request_handler->receive_message(res, _is_tunnel_conn);

			while (_behaviour == keep_receiving_data) {
				co_await boost::asio::async_write(_socket, boost::asio::buffer(*res),
					boost::asio::redirect_error(use_awaitable, ec));
				if (ec) {
					throw std::runtime_error("write failed");
				}
				res.reset(new string(""));
				_behaviour = co_await _request_handler->receive_message(res, _is_tunnel_conn);
			}


			switch (_behaviour) {
			case respond_error:
				_keep_alive = false;
				_request_handler->handle_error(res); //�ܿ죬����Ҫ�첽����
				co_await boost::asio::async_write(_socket, boost::asio::buffer(*res),
					boost::asio::redirect_error(use_awaitable, ec));
				if (ec) {
					throw std::runtime_error("write failed");
				}
				else {
					continue;//�Զ�������ѭ����
				}
				break;
			case ignore:
				_keep_alive = false;
				continue;//�Զ�������ѭ����


			case respond_and_close:
				throw std::runtime_error(
						"switch(_behaviour) ERROR when handle"
						"write back is close (presumably a bug)");
				break;
			case respond_and_keep_alive:
				//û���κ�����
				break;
			case keep_receiving_data://�˴���Ӧ�ô��ڴ�ֵ
				throw std::runtime_error("switch(_behaviour) ERROR when handle write back (presumably a bug)");
				break;
			}

			//��ʣһ��message ûд
			co_await boost::asio::async_write(_socket, boost::asio::buffer(*res),
				boost::asio::redirect_error(use_awaitable, ec));
			if (ec) {
				throw std::runtime_error("write failed");
			}
		}

	}
	catch (const std::exception& e)
	{
		cout << e.what() << endl;
		//stop();
	}

	stop();// Initiate graceful connection closure.
	co_return;
}






inline integrity_status connection::_https_integrity_check()
{
	return integrity_status::https_handshake;
}

}