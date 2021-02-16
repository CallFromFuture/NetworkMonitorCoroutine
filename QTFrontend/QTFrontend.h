#pragma once

#include <QtWidgets/QMainWindow>
#include <QSortFilterProxyModel>

#include "ui_QTFrontend.h"

#include "SessionDataModel.h"

class QTFrontend : public QMainWindow
{
    Q_OBJECT

public:
    QTFrontend(QWidget *parent = Q_NULLPTR, display_filter* _disp = nullptr);






private slots:
    void display_full_info(const QModelIndex& index, const QModelIndex& prev);
    void update_displayed_info(size_t update_id);

private:
    Ui::QTFrontendClass ui;

    SessionDataModel _session_data;
    QSortFilterProxyModel _proxy_session_data;
    
    size_t _display_id=0;


    
    void _display_full_info(size_t display_id);
    

    

};
