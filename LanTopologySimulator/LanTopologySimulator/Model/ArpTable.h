#pragma once

#pragma once
#include <QString>
#include <QVector>
#include <QDateTime>

enum class ArpEntryType {
    Dynamic, // Запись получена автоматически через ARP-ответ
    Static   // Задана вручную администратором
};

struct ArpEntry {
    QString ip; // IPv4-адрес
    QString mac;// Соответствующий MAC-адрес
    ArpEntryType type = ArpEntryType::Dynamic;
    QDateTime timestamp; // Время жизни записи (старение кэша)
};

class ArpTable {
public:
    explicit ArpTable(int ttlSeconds = 120); //ARP-кэш живет 2 мин
    ~ArpTable() = default;

    void addEntry(const QString& ip, const QString& mac, ArpEntryType type = ArpEntryType::Dynamic); // Добавить или обновить запись в кэше
    QString lookupMac(const QString& ip) const; // Поиск MAC-адреса по IP
    bool hasEntry(const QString& ip) const; // Проверка наличия записи
    void cleanExpired(); // Очистка устаревших записей
    void clear(); // Полная очистка кэша

    // Получение всех записей (для команды arp -a и интерфейса)
    const QVector<ArpEntry>& getEntries() const { return m_entries; }
    int size() const { return m_entries.size(); }
private:
    QVector<ArpEntry> m_entries;
    int m_ttlSeconds; // Время жизни динамических записей в секундах
};