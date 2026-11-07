#pragma once

#include <QtWidgets/QMainWindow>
#include <QGraphicsScene> //Управление большим колвом 2Д объектов
#include <QtCore> //Для вывод времени 
#include <QGraphicsEllipseItem> //Эллипс
#include <QGraphicsTextItem> //Для текста на холсте
#include <QFont> //Для размера и жирности
#include <QGraphicsSceneMouseEvent> //События мышки на сцене
#include "DevicePropertiesDialog.h"
//#include <QComboBox> //Для выбора типа кабеля(менюшка)
#include "ui_LanTopologySimulator.h"
#include "NetworkDevice.h"
#include "NodeItem.h"

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

	void on_actionCable_triggered(); //нажатие кнопки КАБЕЛЬ на панели
	void on_actionDelete_triggered(); //нажатие кнопки КОРЗИНЫ на панели
	void on_actionCursor_triggered(); //нажатие кнопки КУРСОР на панели

protected:
	bool eventFilter(QObject* obj, QEvent* event) override; //Фильтр событий для мыши
	void keyPressEvent(QKeyEvent* event) override; //Обработка нажатий клавиш


private:
    Ui::LanTopologySimulatorClass ui;
	QGraphicsScene* m_scene; //Сцена для отображения объектов
	NodeItem* m_firstCableNode; //Узел, с которого тянем кабель

	void resetCableState(); //Сброс состояния кабеля
	void logDeviceCreated(const NetworkDevice* device);
	void handleCableConnection(NodeItem* clickedNode); // Логика 1клик и 2клик
	void deleteSelectedItems(); //Удаление выделенных объектов
};

