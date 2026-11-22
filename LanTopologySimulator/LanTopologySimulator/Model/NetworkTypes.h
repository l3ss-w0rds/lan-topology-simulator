#pragma once

enum class CableType {
    TwistedPair,  // Витая пара
    OpticalFiber, // Оптоволокно
    CoaxialCable, // Коаксиальный кабель
    ConsoleCable  // Консольный кабель
};

// Проверяем, может ли среда передавать Ethernet-кадры
inline bool supportsEthernet(CableType type)
{
    return type != CableType::ConsoleCable;
}