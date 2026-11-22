#pragma once

#include <QVector>
#include <QString>

#include "NetworkDevice.h"
#include "EthernetFrame.h"

struct EthernetArrival {
    NetworkDevice* device = nullptr; // Кто получил кадр
    int portIndex = -1; // На какой интерфейс пришёл
    EthernetFrame frame;
};

struct EthernetResult {
    QVector<EthernetArrival> arrivals;
    QString error;
};

class EthernetSimulator {
public:
    static EthernetResult transmit(
        NetworkDevice* sender,
        int outputPort,
        const EthernetFrame& frame); // Передача кадра через сеть
};