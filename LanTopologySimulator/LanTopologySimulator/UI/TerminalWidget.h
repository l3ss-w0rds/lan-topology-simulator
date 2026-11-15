#pragma once
#include <QWidget>
#include <QStringList>

class QPlainTextEdit;
class QLineEdit;
class QLabel;
class NetworkDevice;

class TerminalWidget : public QWidget {
    Q_OBJECT
public:
    explicit TerminalWidget(NetworkDevice* device, QWidget* parent = nullptr);
    ~TerminalWidget() override = default;

protected:
    bool eventFilter(QObject* obj, QEvent* ev) override;

private slots:
    void handleCommand();   

private:
    void setupUI(); //
    void executeCommand(const QString& rawCommand);
    void print(const QString& text = QString());   
    void saveHistory();                            
    QString promptString() const;                  

    void cmdHelp();
    void cmdClear();
    void cmdIpconfig();
    void cmdPing(const QStringList& tokens);
    void cmdShowMacTable(const QStringList& tokens);
    void cmdArp(const QStringList& tokens);

private:
    NetworkDevice* m_device = nullptr;
    QPlainTextEdit* m_outputView = nullptr;   
    QLineEdit* m_inputEdit = nullptr;
    QLabel* m_promptLabel = nullptr;
    QStringList     m_history;
    int             m_historyPos = 0;
};