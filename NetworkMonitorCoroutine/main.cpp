/*
* main.cpp
* 
* Main Entry point
* 
* Distributed under the Boost Software License, Version 1.0.
(See accompanying file ./License/LICENSE_1_0.txt or copy at
https://www.boost.org/LICENSE_1_0.txt)

*/


#include <iostream>
#include <cstdio>
using namespace std;

#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <boost/bind/bind.hpp>
#include <boost/array.hpp>

#include <boost/lexical_cast.hpp>


using boost::asio::ip::tcp;

//#include "TestServer.h"
#include "proxy_server.h"

/*δ��ɵĲ���:
* 
* logging ��
* 
* 
* 
* certificate_manager::
create_ca
auto_trust_ca
* 
* 
*/

//TODO: websocket support
/*
websocket_integrity_check
*/

/*
������ڵ�
��Ϊ�������ĺ�̨������Ϣѭ��
���Ե�������
*/

int main(int argc, char* argv[])
{
    try
    {
        // Check command line arguments.
        if (argc != 4)
        {
            std::cerr << "Usage: proxy_server <address> <port> <threads>\n";
            std::cerr << "  For IPv4, try:\n";
            std::cerr << "    proxy_server 0.0.0.0 5559 16\n";
            std::cerr << "  For IPv6, try:\n";
            std::cerr << "    proxy_server 0::0 5559 16\n";
            return 1;
        }
        
        // Initialise the server.
        std::size_t num_threads = boost::lexical_cast<std::size_t>(argv[3]);
        proxy_tcp::proxy_server srv(argv[1], argv[2], num_threads);
        printf("listening on %s:%s\n", argv[1], argv[2]);

        // Run the server until stopped.
        srv.start();
    }
    catch (std::exception& e)
    {
        std::cerr << "exception: " << e.what() << "\n";
    }
    

	return 0;
}