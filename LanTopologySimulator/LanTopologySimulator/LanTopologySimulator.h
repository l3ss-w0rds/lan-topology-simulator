#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_LanTopologySimulator.h"

class LanTopologySimulator : public QMainWindow
{
    Q_OBJECT

public:
    LanTopologySimulator(QWidget *parent = nullptr);
    ~LanTopologySimulator();

private:
    Ui::LanTopologySimulatorClass ui;
};

