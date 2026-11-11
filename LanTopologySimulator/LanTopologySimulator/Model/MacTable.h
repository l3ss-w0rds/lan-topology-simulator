#pragma once
#include <QDebug>
#include <QDateTime>
#include <QMap>

enum class TypeEntry {
	Dynamic,  // Выучен автоматом, стареет, есть время
	Static,   // Задан вручну?
	Blackhole //Дропать трафик
};

struct MacEntry {
	int VlanID = 1; //1-4094
	QString macAddress; //Мак-адрес
	TypeEntry type = TypeEntry::Dynamic; //dinamic/static
	int portIndex = -1; //Номер порта коммутатора

	QDateTime timestamp; //Время добавления, для старения маков
	bool isValid = true;
};

class MacTable {
public: 
	explicit MacTable(int agingTimeSec = 300);

	void learn(const QString& mac, int portIndex, int VlanID = 1); //автоматически обучает мак, вызываетсә, если пришел кадр с неизвестным маком
	void addStatic(const QString& mac, int portIndex, int VlanID = 1); //Добавление статического мак адреса
	void addBlackhole(const QString& mac, int portIndex = -1, int VlanID = 1); //Добавление черной дыры
	int lookup(const QString& mac, int VlanID = 1); //Поиск мак адреса, возвращает номер порта или -1 если не найден
	void cleanExpired(); //Удаление устаревших маков
	void clear(); //Очистка таблицы маков
	void removeByPort(int portIndex); //Удаление всех маков по номеру порта

	QMap<QString, MacEntry> getEntries() const { return m_macTable;} //Возвращает копию таблицы маков
	int getSize() const {return m_macTable.size();} //Возвращает количество маков в таблице
	int getAgingTimeSec() const { return m_agingTimeSec; } //Возвращает время старения маков

private: 
	QString makeKey(const QString& mac, int VlanID) const; //Создание уник ключа VLAN:MAC


	QMap<QString, MacEntry> m_macTable; //Хэш таблица для хранения маков
	int m_agingTimeSec;
};