#pragma once

#include <QtWidgets/QMainWindow>
#include <QGraphicsScene> //Управление большим колвом 2Д объектов
#include <QtCore> //Для вывод времени 
#include "ui_LanTopologySimulator.h"
#include "NetworkDevice.h"

class LanTopologySimulator : public QMainWindow
{
    Q_OBJECT

public:
    LanTopologySimulator(QWidget *parent = nullptr);
    ~LanTopologySimulator();
private slots:
	void on_btnPC_clicked();
	void on_btnServer_clicked();
	void on_btnRouter_clicked();
	void on_btnSwitch_clicked();
private:
    Ui::LanTopologySimulatorClass ui;
	QGraphicsScene* m_scene; //Сцена для отображения объектов
	void logDeviceCreated(const NetworkDevice* device);
};

