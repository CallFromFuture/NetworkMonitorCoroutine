#pragma once

namespace proxy_server {

	typedef enum {
		success,//����
		with_appendix,// �к�׺����Ҫ���зָ�
		failed,//������
		wait, //��������
		wait_chunked,//��������chunked��
		https_handshake //https���ְ�
	} integrity_status;

	typedef enum {
		not_begin,
		finished
	} handshake_status;

	typedef enum {
		respond_and_keep_alive,
		respond_and_close,
		respond_as_tunnel,
		respond_and_keep_reading,
		ignore
	} connection_behaviour;

	typedef enum {
		http,
		https,
		error,
		handshake
	} request_protocol;
}