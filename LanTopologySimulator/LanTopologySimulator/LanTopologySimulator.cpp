#include "LanTopologySimulator.h"
#include <QGraphicsEllipseItem> //Эллипс
#include "NetworkDevice.h"

LanTopologySimulator::LanTopologySimulator(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
	m_scene = new QGraphicsScene(this); //Создаем сцену для отобр объектов(белый лист)
	ui.canvasView->setScene(m_scene); //Устанавливаем сцену в виджет canvasView
}

LanTopologySimulator::~LanTopologySimulator()
{}

void LanTopologySimulator::on_btnPC_clicked()
{
	static int pcNumber = 0; //Счетчик ПК
	pcNumber++;

	QString name = QString("PC%1").arg(pcNumber); //Имя ПК
	QString ip = QString("192.168.0.%1").arg(10+pcNumber); //IP адрес ПК)

	NetworkDevice* device = new NetworkDevice(name, ip, DeviceType::PC); //Создаем объект ПК
	//Создаем объект ПК и добавляем его на сцену
	QGraphicsEllipseItem* pc = m_scene->addEllipse(0, 0, 40, 40); //Эллипс размером 40x40
	pc->setFlag(QGraphicsItem::ItemIsMovable); //объект перемещается
	pc->setFlag(QGraphicsItem::ItemIsSelectable); //объект выделяется

	logDeviceCreated(device);

}
void LanTopologySimulator::on_btnServer_clicked()
{
	static int serverNumber = 0; //Счетчик серверов
	serverNumber++;

	QString name = QString("Server%1").arg(serverNumber);
	QString ip = QString("192.168.0.%1").arg(20 + serverNumber);

	NetworkDevice* device = new NetworkDevice(name, ip, DeviceType::Server);

	//Создаем объект сервера и добавляем его на сцену
	QGraphicsRectItem* server = m_scene->addRect(0, 0, 40, 60); //Прямоугольник размером 40x60
	server->setFlag(QGraphicsItem::ItemIsMovable); //объект перемещается
	server->setFlag(QGraphicsItem::ItemIsSelectable); //объект выделяется

	logDeviceCreated(device);


}
void LanTopologySimulator::on_btnSwitch_clicked()
{
	static int switchNumber = 0; //Счетчик коммутаторов
	switchNumber++;

	QString name = QString("Switch%1").arg(switchNumber);
	QString ip = QString("192.168.0.%1").arg(30 + switchNumber);

	NetworkDevice* device = new NetworkDevice(name, ip, DeviceType::Switch);

	//Создаем объект коммутатора и добавляем его на сцену
	QGraphicsRectItem* sw = m_scene->addRect(0, 0, 80, 40); //Прямоугольник размером 80x40
	sw->setFlag(QGraphicsItem::ItemIsMovable); //объект перемещается
	sw->setFlag(QGraphicsItem::ItemIsSelectable); //объект выделяется
	logDeviceCreated(device);

}
void LanTopologySimulator::on_btnRouter_clicked()
{
	static int routerNumber = 0; //Счетчик маршрутизаторов
	routerNumber++;

	QString name = QString("Router%1").arg(routerNumber);
	QString ip = QString("192.168.0.%1").arg(40 + routerNumber);

	NetworkDevice* device = new NetworkDevice(name, ip, DeviceType::Router);

	//Создаем объект маршрутизатора и добавляем его на сцену
	QGraphicsRectItem* router = m_scene->addRect(0, 0, 60, 40); //Прямоугольник размером 60x40
	router->setFlag(QGraphicsItem::ItemIsMovable); //объект перемещается
	router->setFlag(QGraphicsItem::ItemIsSelectable); //объект выделяется
	logDeviceCreated(device);

}




void LanTopologySimulator::logDeviceCreated(const NetworkDevice* device) {

	QString timeStr = QTime::currentTime().toString("hh:mm:ss");

	QString logLine = QString("[%1] [СЕТЬ] Создан %2 | IP: %3 | MAC: %4")
		.arg(timeStr)
		.arg(device->getName())
		.arg(device->getIP())
		.arg(device->getMac());


	ui.logTextEdit->appendPlainText(logLine);

}


