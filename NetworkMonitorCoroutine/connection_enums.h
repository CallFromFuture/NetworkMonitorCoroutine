#pragma once

namespace proxy_tcp {

	typedef enum {
		intact,//����
		//with_appendix,// �к�׺����Ҫ���зָ�
		broken,//������
		wait, //��������
		wait_chunked,//��chunked������������������
		chunked,//��chunked���������к���chunked��
		https_handshake //https���ְ�
	} integrity_status;

	typedef enum {//deprecated
		not_begin,
		finished
	} handshake_status;

	typedef enum {
		respond_and_keep_alive,//��������
		respond_and_close,//
		keep_receiving_data,//������Զ�˶�����
		respond_error,
		ignore
	} connection_behaviour;

	typedef enum {//deprecated
		http,
		https,
		error,
		handshake
	} request_protocol;

	typedef enum {
		_OPTIONS,
		_HEAD,
		_GET,
		_POST,
		_PUT,
		_DELETE,
		_TRACE,
		_CONNECT //https ��������
	} request_type;

}