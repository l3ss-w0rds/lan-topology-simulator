#pragma once
#include <QString>
#include <random>
#include <QDebug>

enum class DeviceType {
	PC,
	Server,
	Switch,
	Router
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

	//Сеттеры изменяют свойства объекта
	void setName(const QString& name) { m_name = name; }
	void setIP(const QString& ip) { m_ip = ip; }
	void setOnline(bool isOnline) { m_isOnline = isOnline; }

private:
	static QString generateRandomMac();

	QString m_name;
	QString m_ip;
	QString m_mac;
	DeviceType m_type;
	bool m_isOnline;

};

