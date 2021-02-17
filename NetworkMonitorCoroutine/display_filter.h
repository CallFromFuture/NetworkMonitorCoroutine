#pragma once

#include <string>
#include <memory>
#include <functional>




#include <boost/asio.hpp>

#ifdef QT_CORE_LIB
#include <qobject.h>
#include <mutex>
#include "../QTFrontend/SessionDataModel.h"
#endif

using namespace std;
namespace proxy_tcp{

	using boost::asio::ip::tcp;
	using boost::asio::awaitable;
	using boost::asio::co_spawn;
	using boost::asio::detached;
	using boost::asio::redirect_error;
	using boost::asio::use_awaitable;


#ifdef QT_CORE_LIB
class proxy_handler;
class display_filter : public QObject
{
	Q_OBJECT
#else
class display_filter
{

#endif

public:
#ifndef QT_CORE_LIB
	void display(shared_ptr<string> req_data, shared_ptr<string> rsp_data);
	void _temp_display(shared_ptr<string> data);
	//��ʼ��ʾ
	int display(shared_ptr<string> req_data);

	//��ʾrequest��ʱ���ǲ����д��
	void update_display_req(int id, shared_ptr<string> req_data);

	void update_display_rsp(int id, shared_ptr<string> rsp_data);
	//ֻ�ڸ���response��ʱ��˳����ʾ����
	void update_display_error(int id, shared_ptr<string> rsp_data);

	//����chunked data��ֻ�ڵ�һ�ε���breakpoint
	awaitable<int> display_breakpoint_req(shared_ptr<string> req_data);//����Ҫ��ʾ����
	awaitable<int> display_breakpoint_rsp(int update_id, shared_ptr<string> rsp_data);//����Ҫ��ʾ����
#else

	//shared_ptr<session_info> display(
	//	shared_ptr<string> req_data, shared_ptr<proxy_handler> _proxy_handler, bool breakpoint);

	void display(shared_ptr<session_info> _session_info);
	void update_display_req(shared_ptr<session_info> _session_info);
	void complete_req(shared_ptr<session_info> _session_info);

	void display_rsp(shared_ptr<session_info> _session_info);
	void update_display_rsp(shared_ptr<session_info> _session_info);
	void complete_rsp(shared_ptr<session_info> _session_info);

	void update_display_error(shared_ptr<session_info> _session_info);


#endif
	

	
#ifdef QT_CORE_LIB
private:
	

signals://signal
	//void filter_updated(string filter);

	void session_created(shared_ptr<session_info> _session_info);

	void session_req_updated(shared_ptr<session_info> _session_info);
	void session_req_completed(shared_ptr<session_info> _session_info);

	void session_rsp_begin(shared_ptr<session_info> _session_info);
	void session_rsp_updated(shared_ptr<session_info> _session_info);
	void session_rsp_completed(shared_ptr<session_info> _session_info);

	void session_error(shared_ptr<session_info> _session_info);

//private slots:
	//void session_passed(int update_id,bool is_req);//״̬��֪��Ӧ����bitmap����hashmap

#endif
};



}

 
