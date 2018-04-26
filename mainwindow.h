#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include<QMainWindow>
#include<QPainter>
#include<qtimer.h>
#include<qtextcodec.h>
#include<qstandarditemmodel.h>
#include"Chord.h"
#include<regex>
namespace Ui {
class MainWindow;
}
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
	Chord * chord;
	QStandardItemModel * table;
	void maketableview();
    explicit MainWindow(QWidget *parent = 0);
	~MainWindow();
public slots:
    void on_pushButton_clicked();//�½ڵ����
    void on_pushButton_2_clicked();//�ڵ��뿪
    void on_pushButton_3_clicked();//������Ϣ
    void on_pushButton_4_clicked();//������Ϣ
    void on_pushButton_5_clicked();//������Ϣ
	void MainWindow::paintEvent(QPaintEvent *event);
private:
    Ui::MainWindow *ui;
	int first = 0;
	int size;
	int circlesize;
	void draw(QPainter*paint,list<FingerNode>);
	void draw1(QPainter*paint, list<FingerNode>);
};

#endif // MAINWINDOW_H
