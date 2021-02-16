#pragma once

#include <string>
#include <memory>
#include <functional>




#include <boost/asio.hpp>

#ifdef QT_CORE_LIB
#include <qobject.h>
#include <mutex>
//#include "../QTFrontend/SessionDataModel.h"
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
#endif
	


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

	
#ifdef QT_CORE_LIB
private:
	mutex update_id_lock;
	size_t update_id = 0;

	//SessionDataModel* _session_data_model;
	inline size_t get_temp_id() {//potential performance loss
		update_id_lock.lock();
		size_t temp_id = update_id;
		update_id++;
		update_id_lock.unlock();
		return temp_id;
	}

signals://signal
	void filter_updated(string filter);

	void session_created(shared_ptr<string> req_data, int update_id);
	void session_created_breakpoint(shared_ptr<string> req_data, int update_id);

	void session_req_updated(shared_ptr<string> req_data, int update_id);//�����Ͽ��Ըģ��ȴ�����chunked data����

	void session_rsp_updated(shared_ptr<string> rsp_data, int update_id);
	void session_rsp_updated_breakpoint(shared_ptr<string> rsp_data,int update_id);

	void session_error(shared_ptr<string> err_msg, int update_id);

//private slots://slot
	//void session_passed(int update_id,bool is_req);//״̬��֪��Ӧ����bitmap����hashmap

#endif
};

}

 
