#pragma once

#include <QtWidgets/QMainWindow>
#include <QSortFilterProxyModel>

#include "ui_QTFrontend.h"

#include "SessionDataModel.h"

using namespace proxy_tcp;

#include <map>
using namespace std;


struct breakpoint_filter {
    map<string, string> header_filter;

    string raw_custom_header_filter;
    string raw_host_filter;

    bool enable_breakpoint = false;
};

struct config {
    breakpoint_filter req_filter;
    breakpoint_filter rsp_filter;

    int column_width[sizeof(_table_header_name)/sizeof(const char*)] = {
    20,200,35,45,200,40,70
    };

    bool ssl_decrypt = true;
};


class QTFrontend : public QMainWindow
{
    Q_OBJECT

public:
    QTFrontend(QWidget *parent = Q_NULLPTR, display_filter* _disp = nullptr);






private slots:
    void display_full_info(const QModelIndex& index, const QModelIndex& prev);
    void update_displayed_info(size_t update_id);

    void pass_session();
    void drop_session();
    void _debug_function();

private:
    Ui::QTFrontendClass ui;

    SessionDataModel _session_data;
    QSortFilterProxyModel _proxy_session_data;
    
    shared_ptr<session_info> _display_session_ptr;

    size_t _display_id=0;

    config _config;

    bool last_breakpoint_req_checked=true;

    bool is_req_intercepted = true;

    
    void _display_full_info(size_t display_id);
    void _activate_breakpoint_box(bool active);
    void _activate_editor(bool active, bool is_req);

    void _load_config_from_file(string path="config.dat");

    void _toggle_breakpoint_config();

    void _set_config();//write ui's data to _config
    void _update_config();

    void _save_config();

};
