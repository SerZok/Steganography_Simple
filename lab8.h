#ifndef LAB8_H
#define LAB8_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class Lab8; }
QT_END_NAMESPACE

class Lab8 : public QMainWindow
{
    Q_OBJECT

public:
    Lab8(QWidget *parent = nullptr);
    ~Lab8();

private slots:
    void loadOriginalText();
    void encodeText();
    void decodeText();

private:
    Ui::Lab8 *ui;
    QString processText(const QString &text, const QString &hiddenMessage);
    QString extractMessage(const QString &stegoText);
};

#endif // LAB8_H
