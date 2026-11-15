#pragma once
#include <QDialog>
#include "NetworkDevice.h"
#include "TerminalWidget.h"
#include <QDialogButtonBox>            
#include <QTableWidgetItem>            
#include <QHeaderView>
#include <QVBoxLayout>
#include <QComboBox>
#include <QMessageBox>



// Связываем с формой из Qt Designer
namespace Ui {
    class DevicePropertiesDialog;
}

class NetworkDevice;

class DevicePropertiesDialog : public QDialog {
    Q_OBJECT
public:
    explicit DevicePropertiesDialog(NetworkDevice* device, QWidget* parent = nullptr);
    ~DevicePropertiesDialog() override;


private:
    void loadData();       // Загружает имя, IP и MAC в форму
    void loadPortsTable(); // Наполняет таблицу портов реальными данными из черновика
    void loadSelectedInterface(int index); //Загружает данные выбранные из черновика в соотв. поля ввода
    void storeSelectedInterface(); //Сохраняем текущее значение из полей ввода обратно в черновик для выбранного порта
    void saveConfiguration(); //Применяет черновик к реальному устройству

private:
    Ui::DevicePropertiesDialog* ui; 
    NetworkDevice* m_device =nullptr;
    QVector<NetworkPort> m_draft;
    int m_selectedPort = -1;
};