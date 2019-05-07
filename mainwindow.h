#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void changeUiColor();

    QPixmap genIcon();

    void preveiw();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void on_pushButton_clicked();

    void on_spinBox_valueChanged(int arg1);

    void on_spinBox_2_valueChanged(int arg1);

    void on_spinBox_3_valueChanged(int arg1);

private:
    Ui::MainWindow *ui;
    QString m_fontFamily;
    QList<quint32> m_fontChars;

    QColor m_color;
    QColor m_bgColor;
};

#endif // MAINWINDOW_H
