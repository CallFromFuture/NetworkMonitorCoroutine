#pragma once

namespace proxy_server {

	typedef enum {
		success,//����
		failed,//������
		wait, //��������
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
		ignore
	} connection_behaviour;

	typedef enum {
		http,
		https,
		error,
		handshake
	} request_protocol;
}