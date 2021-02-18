#pragma once

namespace proxy_tcp {

	typedef enum {
		intact,//����
		//with_appendix,// �к�׺����Ҫ���зָ�
		broken,//������
		wait, //��������
		wait_chunked,//��chunked������������������
		chunked,//��chunked���������к���chunked��
		https_handshake, //https���ְ�
		websocket_intact//websocket
	} integrity_status;

	typedef enum {//deprecated
		not_begin,
		finished
	} handshake_status;

	typedef enum {
		respond_and_keep_alive,//��������
		respond_and_close,//
		keep_receiving_data,//������Զ�˶�����
		protocol_websocket,
		respond_error,
		ignore
	} connection_behaviour;

	typedef enum {
		http,
		//https,
		websocket,
		websocket_handshake,
		//websocket_with_ssl,
		unknown

	} connection_protocol;

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