#include "lab8.h"
#include "ui_lab8.h"
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>

Lab8::Lab8(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Lab8)
{
    ui->setupUi(this);

    // Подключение кнопок к слотам
    connect(ui->action, &QAction::triggered, this, &Lab8::loadOriginalText);
    connect(ui->encodeBtn, &QPushButton::clicked, this, &Lab8::encodeText);
    connect(ui->decodeBtn, &QPushButton::clicked, this, &Lab8::decodeText);
}

Lab8::~Lab8()
{
    delete ui;
}

// Открытие и загрузка файла
void Lab8::loadOriginalText()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Выбрать файл", "", "Text Files (*.txt);;All Files (*)");
    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    QString text = in.readAll();
    file.close();

    // Вычисляем максимальное количество битов, которые можно скрыть
    int maxCapacity = 0;
    for (int i = 0; i < text.length(); i++) {
        if (text[i] == '.' || text[i] == '?' || text[i] == '!') {
            if (i + 1 < text.length() && text[i + 1] == ' ') {
                maxCapacity++;
            }
        }
    }

    // Выводим информацию в MessageBox
    QMessageBox::information(this, "Информация",
                             QString("Максимальная емкость контейнера: %1 бит (%2 байт)").arg(maxCapacity).arg(maxCapacity / 8));
    ui->OriginalFileText->setPlainText(text);
}

// Кодирование текста с сообщением
void Lab8::encodeText()
{
    QString containerText = ui->OriginalFileText->toPlainText();
    QString hiddenMessage = ui->openText->toPlainText();

    // Преобразуем скрытое сообщение в двоичный код
    QString binaryMessage;
    for (QChar ch : hiddenMessage) {
        binaryMessage.append(QString::number(ch.unicode(), 2).rightJustified(8, '0'));
    }
    binaryMessage.append("00000000"); // Маркер окончания

    // Проверяем, хватит ли места в контейнере
    int availableBits = 0;
    for (int i = 0; i < containerText.length(); i++) {
        if ((containerText[i] == '.' || containerText[i] == '?' || containerText[i] == '!') &&
            (i + 1 < containerText.length() && containerText[i + 1] == ' ')) {
            availableBits++;
        }
    }

    QMessageBox::information(this,
                             "Информация",
                             QString("Длина скрытого сообщения: %1 бит\n"
                                     "Максимальная вместимость контейнера: %2 бит")
                                 .arg(binaryMessage.length())
                                 .arg(availableBits));

    // Проверяем, хватит ли места в контейнере
    if (binaryMessage.length() > availableBits) {
        QMessageBox::warning(this,
                             "Ошибка",
                             "Недостаточно места в контейнере для скрытия сообщения!");
        return;
    }

    QString encodedText = processText(containerText, hiddenMessage);
    ui->ModifyText->setPlainText(encodedText);
}

// Декодирование текста и восстановление скрытого сообщения
void Lab8::decodeText()
{
    QString stegoText = ui->ModifyText->toPlainText();
    QString decodedMessage = extractMessage(stegoText);
    ui->DecodedText->setPlainText(decodedMessage);
}

QString Lab8::processText(const QString &text, const QString &hiddenMessage)
{
    QByteArray utf8Message = hiddenMessage.toUtf8();
    QString binaryMessage;

    for (char byte : utf8Message) {
        binaryMessage.append(QString::number(static_cast<unsigned char>(byte), 2).rightJustified(8, '0'));
    }

    binaryMessage.append("00000000"); // Маркер окончания
    qDebug() << "Исходное скрытое сообщение в битах:\t" << binaryMessage;

    QString result;
    int bitIndex = 0;

    for (int i = 0; i < text.length(); i++) {
        result.append(text[i]);

        if ((text[i] == '.' || text[i] == '?' || text[i] == '!') && i + 1 < text.length() && text[i + 1] == ' ') {
            if (bitIndex < binaryMessage.size()) {
                result.append(binaryMessage[bitIndex] == '0' ? " " : ""); // 0 — один пробел, 1 — два пробела
                bitIndex++;
            }
        }
    }

    return result;
}

QString Lab8::extractMessage(const QString &stegoText)
{
    QString binaryMessage;

    for (int i = 0; i < stegoText.length() - 1; i++) {  // -1, чтобы не выйти за границы строки
        if ((stegoText[i] == '.' || stegoText[i] == '?' || stegoText[i] == '!') && stegoText[i + 1] == ' ') {
            int spaceCount = 1;

            // Считаем пробелы после знака препинания
            int j = i + 2;
            while (j < stegoText.length() && stegoText[j] == ' ') {
                spaceCount++;
                j++;
            }

            binaryMessage.append(spaceCount == 1 ? '1' : '0');

            // Пропускаем уже обработанные пробелы
            i = j - 1;
        }
    }

    qDebug() << "Расшифрованное сообщение в битах:\t" << binaryMessage;

    // Декодируем двоичный код обратно в текст
    QByteArray utf8Message;
    for (int i = 0; i < binaryMessage.length(); i += 8) {
        QString byteStr = binaryMessage.mid(i, 8);

        if (byteStr == "00000000") break; // Останавливаемся при маркере окончания

        bool ok;
        char byte = byteStr.toInt(&ok, 2);
        if (ok) {
            utf8Message.append(byte);
        }
    }

    return QString::fromUtf8(utf8Message);
}
