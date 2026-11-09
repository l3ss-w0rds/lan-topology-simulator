#pragma once
#include <QDialog>
#include "NetworkDevice.h"
#include <QDialogButtonBox>            
#include <QTableWidgetItem>            
#include <QHeaderView>                 


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
    void loadPortsTable(); // Наполняет таблицу портов реальными данными из модели

private:
    Ui::DevicePropertiesDialog* ui; // Доступ ко всем элементам вашей UI-формы
    NetworkDevice* m_device;
};