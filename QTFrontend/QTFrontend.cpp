#include "QTFrontend.h"
#include "qimage.h"


static int column_width[] = {
    20,100,35,45,200,40,70
};

QTFrontend::QTFrontend(QWidget *parent,display_filter* _disp)
    : QMainWindow(parent),
    _session_data(nullptr,_disp)
{
    ui.setupUi(this);

    //table settings start

    _proxy_session_data.setSourceModel(&_session_data);

    ui.table_session->setModel(&_proxy_session_data);
    //ui.table_session->setSortingEnabled(true);
    ui.table_session->sortByColumn(0, Qt::AscendingOrder);

    //ui.table_session->setSelectionBehavior(QAbstractItemView::SelectRows);

    //ui.table_session->verticalHeader()->setHidden(true);
    //ui.table_session->verticalHeader()->setDefaultSectionSize(15);

    //ui.table_session->horizontalHeader()->setHighlightSections(false);

    for (int i = 0; i < sizeof(column_width) / sizeof(int); i++) {
        ui.table_session->setColumnWidth(i, column_width[i]); //TODO : config
    }


    connect(ui.table_session->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &QTFrontend::display_full_info);
    connect(&_session_data, &SessionDataModel::info_updated, this, &QTFrontend::update_displayed_info);

    ui.table_session->show();
    //table settings end

    ui.scrollArea->setVisible(true);
    
}

void QTFrontend::display_full_info(const QModelIndex& index, const QModelIndex& prev) {
    
    _display_id = _proxy_session_data.data(_proxy_session_data.index(index.row(), 0)).toInt();// ��ȡʵ��λ��
    _display_full_info(_display_id);
}

void QTFrontend::update_displayed_info(size_t update_id)
{
    if (_display_id == update_id) {
        _display_full_info(_display_id);
    }
}

//TODO: ���ӱ����ļ�����
void QTFrontend::_display_full_info(size_t display_id)
{

    
    //��Զ·�ˣ�����������
    auto req = _session_data.get_raw_req_data(display_id);
    auto rsp = _session_data.get_raw_rsp_data(display_id);
    if (req)
        ui.plaintext_req_text->setPlainText(QString::fromStdString(*req));
    else
        ui.plaintext_req_text->setPlainText(QString());

    if (!rsp) {
        ui.plaintext_rsp_text->setPlainText(QString());
        return;
    }

    ui.plaintext_rsp_text->setPlainText(QString::fromStdString(*rsp));

    try
    {
        if (_session_data.get_content_type(display_id).find("image") == 0) {//image
            size_t header_end_pos = rsp->find("\r\n\r\n");

            if (header_end_pos != string::npos) {
                int length = rsp->size() - header_end_pos - 4;
                const uchar* image_bytes = (const uchar*)(rsp->c_str() + header_end_pos + 4);
                QImage img;
                if (!img.loadFromData(image_bytes, length))
                    throw -1;

                ui.label_image->setPixmap(QPixmap::fromImage(img));//TODO: gif/webm/... support
                ui.scrollArea->setWidgetResizable(false);
                ui.label_image->adjustSize();
                ui.scrollAreaWidgetContents->adjustSize();

                
                
            }
            else {
                throw - 1;
            }

        }else {
            throw -1;

        }

    }
    catch (int error_code)
    {
        if (error_code == -1) {
            ui.scrollArea->setWidgetResizable(true);
            ui.label_image->setText("Cannot resolve it as image");
        }

    }
    

}
