#pragma once

#include <QString>
#include <QByteArray>
#include <QtGlobal>

namespace EtherType {
    constexpr quint16 IPv4 = 0x0800;
    constexpr quint16 Arp = 0x0806;
    constexpr quint16 Test = 0x88B5; // Локальные тестовые кадры
}

inline QString ethernetBroadcastMac()
{
    return QStringLiteral("FF:FF:FF:FF:FF:FF");
}

struct EthernetFrame {
    QString sourceMac; // MAC отправителя на текущем участке
    QString destinationMac; // MAC получателя на текущем участке
    quint16 etherType = EtherType::Test; // Тип содержимого
    QByteArray payload; // Данные кадра
};