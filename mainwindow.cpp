#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "QtAwesome/QtAwesome.h"

#include <QFileDialog>
#include <QColorDialog>
#include <QToolButton>
#include <QMessageBox>
#include <list>
#include <string>

#define TTF_FONT_PARSER_IMPLEMENTATION
#include "ttfParser.h"

#include <chrono>
#include <thread>

struct TTFRes{
    uint8_t condition_variable = 0;
    std::string family;
    std::list<uint32_t> fonts;
};

void font_parsed(void* args, void* _font_data, int error) {
    if (error) {
        (*(TTFRes*)args).condition_variable = error;
        //printf("Unable to parse font\n");
    }
    else {
        TTFFontParser::FontData* font_data = (TTFFontParser::FontData*)_font_data;
        //printf("Font %s parsed\n", font_data->full_font_name.c_str());
        //printf("Number of glyphs: %d\n", font_data->glyphs.size());
        (*(TTFRes*)args).condition_variable = 1;
        (*(TTFRes*)args).family = font_data->full_font_name;

        for(int i=0;i<font_data->glyphs.size();++i){
            auto glyphs = font_data->glyphs.at(i);
            auto c = glyphs.character;
            (*(TTFRes*)args).fonts.push_back(c);
        }
    }
}

bool parseTTF(const QString &path,QString& family,QList<quint32>& fontNames) {
    uint8_t condition_variable = 0;
    TTFRes ttfres;

    TTFFontParser::FontData font_data;
    int8_t error = TTFFontParser::parse_file(path.toStdString().c_str(), &font_data, &font_parsed, &ttfres);

    family = QString::fromStdString(ttfres.family);
    fontNames = QList<quint32>::fromStdList(ttfres.fonts);
//    while (!condition_variable) {
//        std::this_thread::sleep_for(std::chrono::milliseconds(1));
//    }

    return ttfres.condition_variable == 1;
}

const QString STYLE("border:none;border-radius:%1px;background-color:rgb(%2,%3,%4,%5);");


void MainWindow::changeUiColor()
{
    ui->color->setStyleSheet(QString("border:1px solid #eee;border-radius:4px;background-color:rgb(%1,%2,%3)")
                             .arg(QString::number(m_color.red()))
                             .arg(QString::number(m_color.green()))
                             .arg(QString::number(m_color.blue())));

    ui->bgcolor->setStyleSheet(QString("border:1px solid #eee;border-radius:4px;background-color:rgba(%1,%2,%3,%4)")
                               .arg(QString::number(m_bgColor.red()))
                               .arg(QString::number(m_bgColor.green()))
                               .arg(QString::number(m_bgColor.blue()))
                               .arg(QString::number(m_bgColor.alpha()))
                               );

}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == ui->color && event->type() == QEvent::MouseButtonPress){
        auto color = QColorDialog::getColor(m_color,this);
        if(color.isValid()){
            m_color = color;
            changeUiColor();
            preveiw();
        }
        return true;
    }else if(watched == ui->bgcolor && event->type() == QEvent::MouseButtonPress){
        auto color = QColorDialog::getColor(m_color,this,"",QColorDialog::ShowAlphaChannel);
        if(color.isValid()){
            m_bgColor = color;
            changeUiColor();
            preveiw();
        }
        return true;
    }
    return QMainWindow::eventFilter(watched,event);
}

void MainWindow::preveiw()
{
    auto pixmap = genIcon();
    QString s = STYLE.arg(ui->spinBox_2->value())
            .arg(QString::number(m_bgColor.red()))
            .arg(QString::number(m_bgColor.green()))
            .arg(QString::number(m_bgColor.blue()))
            .arg(QString::number(m_bgColor.alpha()));

    ui->lb_preview_128->setStyleSheet(s);
    ui->lb_preview_128->setPixmap(pixmap);

    ui->lb_preview_64->setStyleSheet(s);
    ui->lb_preview_64->setPixmap(pixmap);

    ui->lb_preview_32->setStyleSheet(s);
    ui->lb_preview_32->setPixmap(pixmap);

    ui->lb_preview_16->setStyleSheet(s);
    ui->lb_preview_16->setPixmap(pixmap);
}

QPixmap MainWindow::genIcon()
{
    auto txt = ui->lb_char->text();
    bool isOk = false;
    int v = txt.toInt(&isOk,16);

    QVariantMap var;
    var["color"] = m_color;
    qreal factor = (qreal)ui->spinBox_3->value()/100;
    var["scale-factor"] = factor;
    auto icon = QtAwesome::getSingleton()->icon(v,var);
    int size = ui->spinBox->value();
    icon.actualSize(QSize(size,size));

    auto pixmap = icon.pixmap(QSize(size,size));
    return pixmap;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->gridLayout->setAlignment(Qt::AlignLeft|Qt::AlignTop);

    auto btn = new QToolButton(this);
    btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    btn->setText(QStringLiteral("导入字体"));
    btn->setIcon(QIcon(":/import.png"));
    ui->mainToolBar->addWidget(btn);

    ui->color->installEventFilter(this);
    ui->bgcolor->installEventFilter(this);

    ui->pushButton->setEnabled(false);

    auto lbState = new QLabel(this);
    lbState->setStyleSheet("color: rgb(100,100,100);");
    lbState->setOpenExternalLinks(true);
    lbState->setText(QStringLiteral("  字符集制作网站:<a href='https://www.iconfont.cn/'>阿里巴巴图标</a> <a href='https://glyphter.com/'>glyphter</a> "));
    ui->statusBar->addWidget(lbState);

    m_color = QColor::fromRgb(0,0,0);
    m_bgColor = QColor::fromRgb(240,240,240);

    changeUiColor();

    connect(btn,&QToolButton::clicked,this,[=](){

        QString path = QFileDialog::getOpenFileName(this,"Select font file",QString(),"TTF *.ttf");

        if(path.isEmpty())
            return;

        parseTTF(path,m_fontFamily,m_fontChars);

        if(m_fontChars.size() == 0 || m_fontFamily.isEmpty()){
            QMessageBox::warning(this,"Error","Import icon file error.");
            return;
        }

        QtAwesome::getSingleton()->initIconFont(path);

        ui->lb_family->setText(m_fontFamily);

        //清理
        int count = ui->gridLayout->count();
        for(int i=count-1;i>=0;--i){
            auto w = ui->gridLayout->itemAt(i)->widget();
            ui->gridLayout->removeWidget(w);
            w->deleteLater();
        }

        foreach(quint32 v,m_fontChars){
            auto w = new QToolButton(this);

            w->setStyleSheet("border:none;border-radius:3px;background-color:rgb(245,245,245);");
            w->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
            w->setIconSize(QSize(32,32));
            w->setFixedSize(50,50);
            auto icon = QtAwesome::getSingleton()->icon(v);
            icon.actualSize(QSize(32,32));
            w->setIcon(icon);
            w->setText("0x"+QString::number(v,16));
            int total = ui->gridLayout->count();

            int row = total/6;
            int coloum = total%6;
            ui->gridLayout->addWidget(w,row,coloum);

            connect(w,&QToolButton::clicked,[=](){
                ui->lb_family->setText( m_fontFamily );
                ui->lb_char->setText(w->text());

                preveiw();
            });
        }

        ui->pushButton->setEnabled(true);
        //btn->setEnabled(false);
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    auto pixmap = genIcon();

    //TODO:保存图片
    int size = ui->spinBox->value();
    int radius = ui->spinBox_2->value();
    QImage image_res(size,size,QImage::Format_ARGB32);

    image_res.fill(qRgba(255,255,255,0));
    //image_res.fill(Qt::white);
    QPainter painter;
    painter.begin(&image_res);

    //画图
    painter.setClipping(true);
    painter.setBackgroundMode(Qt::TransparentMode);
    painter.setBackground(QBrush(Qt::transparent));

    QPainterPath clipPath;
    clipPath.addRoundedRect(0, 0, size, size, radius, radius);
    painter.setClipPath(clipPath);
    painter.setBrush(QBrush(m_bgColor));
    painter.setPen(m_bgColor);
    painter.drawRect(0,0,size,size);
    painter.drawPixmap(0,0,size,size,pixmap);

    painter.end();

    auto path = QFileDialog::getSaveFileName(this,"Select Save image path",QString(),"Png *.png");

    image_res.save(path,"PNG");
}

void MainWindow::on_spinBox_valueChanged(int arg1)
{
    preveiw();
}

void MainWindow::on_spinBox_2_valueChanged(int arg1)
{
    preveiw();
}

void MainWindow::on_spinBox_3_valueChanged(int arg1)
{
    preveiw();
}
