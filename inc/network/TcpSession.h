#pragma once

#ifdef NETWORKASIO_EXPORTS
#define NETWORK_ASIO_API  __declspec(dllexport)
#else 
#define NETWORK_ASIO_API  __declspec(dllimport)
#endif

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <iostream>
#include <vector>
#include <deque>
#include "NetworkAsio/TcpMessage.h"
#include "Common/Mutex.h"

using boost::asio::ip::tcp;

namespace network_asio {

class TcpSession;

typedef std::deque<TcpMessage> TcpMessageQueue;

class SocketReaderSpi {
public:
	virtual void OnReceive(TcpSession *tcp_sock, char* buf, int len) = 0;
};

class SocketDissConnSpi {
public:
	virtual void OnDisconnect(TcpSession *tcp_sock) = 0;
};

class NETWORK_ASIO_API TcpSession
	: public boost::enable_shared_from_this<TcpSession>
{
public:
	TcpSession(boost::asio::io_service& io_service, 
		SocketReaderSpi* spi, SocketDissConnSpi* dis_conn_spi, bool is_server=false);
	~TcpSession();
	
	// �û����˽ӿڷ�����
	void Send(char* buf, int len);

	tcp::socket& socket() { return socket_; }
	void start();
	void close();

private:
	void handle_read_header(const boost::system::error_code& error);
	void handle_read_body(const boost::system::error_code& error);
	void handle_write(const boost::system::error_code& error);
	void handle_close();

	boost::asio::io_service &io_service_;
	// The socket used to communicate with the client.
	tcp::socket socket_;

	SocketReaderSpi *spi_;
	SocketDissConnSpi *disconn_spi_;

	TcpMessage read_message_; // �����������
	TcpMessageQueue write_message_;
	itstation::common::Mutex write_message_mutex_;

	// �Ƿ���˻��ǿͻ���
	bool is_server_;
	bool server_recv_data_; // ��������EXPIRES_TIME���Ƿ��յ�������,���û�յ�������������
	// �ͻ��˼�������<NUMBER_�����ͻ���ÿ��EXPIRES��������������������������NUMBER_�ζ�û�յ�����������������
	int client_ii_a; 

	//����ˣ����EXPIRES_TIME ���ղ������ݣ������ϸ�����
	//�ͻ��ˣ�ÿ��EXPIRES_TIME�����˷�һ����������������6���ղ������������ص���������������
	boost::asio::deadline_timer timer_;
	void OnTimer(const boost::system::error_code&);
};

typedef boost::shared_ptr<TcpSession> TcpSessionPtr;


}