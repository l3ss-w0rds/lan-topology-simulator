#pragma once
#include <QString>
#include <random>
#include <QDebug>
#include <QVector>

enum class DeviceType {
	PC,
	Server,
	Switch,
	Router
};
struct NetworkPort {
	QString name; //имя интерфейса
	bool isOccupied = false; // занят или свободен
	class NetworkDevice* connectedDevice = nullptr; //к кому подключен
	QString connectedPortName = ""; //имя порта соседа
};

class NetworkDevice {
public:
	NetworkDevice(const QString& name, const QString& ip, DeviceType type);
	virtual ~NetworkDevice() = default;

	//Геттеры читают свойства объекта
	QString getName() const {return m_name; }
	QString getIP() const { return m_ip; }
	QString getMac() const { return m_mac; }
	DeviceType getType() const { return m_type; }
	bool getOnline() const { return m_isOnline; }
	const QVector<NetworkPort>& getPorts() const { return m_ports; }

	QVector<NetworkPort> getFreePorts() const; //Возврощает список свободных портов устройства
	bool hasFreePorts() const; //Проверка на наличие свободных портов
	bool connectPort(const QString& myPortName, NetworkDevice* otherDevice, const QString& otherPortName); //занять порт подклчением
	void disconnectPort(const QString& myPortName); //освободить порт

	//Сеттеры изменяют свойства объекта
	void setName(const QString& name) { m_name = name; }
	void setIP(const QString& ip) { m_ip = ip; }
	void setOnline(bool isOnline) { m_isOnline = isOnline; }

	bool isConnectedTo(const NetworkDevice* other) const; //Проверка соединения с другим устройством
	void connectTo(NetworkDevice* other); //Установить соединение с другим устройством
	void disconnectedFrom(NetworkDevice* other); //Разорвать соединение с другим устройством

private:
	static QString generateRandomMac();
	void initPorts(); //Инициализация портов устройства

	QString m_name;
	QString m_ip;
	QString m_mac;
	DeviceType m_type;
	bool m_isOnline;
	//QList <NetworkDevice*> m_connectedDevices; //Список подключенных устройств
	QVector<NetworkPort> m_ports; //Список портов устройства

};

