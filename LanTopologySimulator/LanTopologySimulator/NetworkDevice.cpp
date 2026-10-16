#include "networkdevice.h"


NetworkDevice::NetworkDevice(const QString& name, const QString& ip, DeviceType type)
	: m_name(name), 
	m_ip(ip), 
	m_mac(generateRandomMac()), 
	m_type(type), 
	m_isOnline(true)
{
}
QString NetworkDevice::generateRandomMac() {
	std::random_device rd; //Источник энтропии
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> dis(0, 255);

	uint8_t mac[6];
	for (int i = 0; i < 6; ++i) {
		mac[i] = static_cast<uint8_t>(dis(gen));
	}
	mac[0] &= 0xFC; // Устанавливаем первый бит в 0 (1байт четный), а второй не 2,6,10....
	//mac[0] |= 0x02; // Устанавливаем второй бит в 1


	QString macStr = QString("%1:%2:%3:%4:%5:%6")
		.arg(mac[0], 2, 16, QChar('0'))
		.arg(mac[1], 2, 16, QChar('0'))
		.arg(mac[2], 2, 16, QChar('0'))
		.arg(mac[3], 2, 16, QChar('0'))
		.arg(mac[4], 2, 16, QChar('0'))
		.arg(mac[5], 2, 16, QChar('0'))
		.toUpper();
	qDebug() << "Generated MAC: " << macStr;
	return macStr;
}





	/*
	for (int i = 0; i < 11; i++) {
		int rn = dis(gen);
		while (i ==0 && (rn % 2 != 0)) {
			rn = dis(gen);
		}
		while (i == 1 && (rn == 2 || rn == 6 || rn == 10 || rn == 14)) { rn = dis(gen); }

		macBytes[i] = rn;

		for (int i = 1; i < 12; i+=2) {
			macBytes += ":" + std::hex + std::to_string(macBytes[i]);
		}

		return macBytes;
	}*/

