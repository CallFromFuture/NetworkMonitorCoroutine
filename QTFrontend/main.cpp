#include "QTFrontend.h"
#include <QtWidgets/QApplication>

#include "../NetworkMonitorCoroutine/proxy_server.h"

#include <memory>

#include <boost/thread.hpp>

#include <QMetaType>

//TODO:
/*
* save_session
* 
*/


//TODO ������������ĳЩhost��meta-type ��ʾ��/�ϵ���
//TODO stream truncated
//TODO ǰ��˽�����ӳ���Щ���������Լ���webui,����Զ�̵�����

//TODO ���õĸ���ͷ�İ취

//TODO ɾ������session�������עδ���鿴��session


int main(int argc, char *argv[])
{
    qRegisterMetaType<QVector<int>>("QVector<int>");
    qRegisterMetaType<shared_ptr<session_info>>("shared_ptr<session_info>");
    
    qRegisterMetaType<size_t>("size_t");
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QApplication a(argc, argv);
    




    //proxy_tcp::proxy_server _backend_proxy_server();
    //boost::thread backend_thread(boost::bind(
    //    &proxy_tcp::proxy_server::start, &_backend_proxy_server));

 

    QTFrontend w(nullptr);

    if (argc > 1) {
        if (string(argv[1]) == "--debug") {
            AllocConsole();
            freopen("CON", "w", stdout);
        }
    }

    w.show();

    a.exec();
    //backend_thread.join();
    return 0;
}
