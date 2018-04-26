#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<qtableview.h>

const int PI = 3.1415926;
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
	maketableview();
	chord = new Chord();
	QTimer*timer = new QTimer(this);
	timer->start(1000);
	connect(timer, SIGNAL(timeout()), this, SLOT(update()));
	list<FingerNode>tables = chord->Gettable(chord->getlocalid());
	int n = 0;
	for (auto i = tables.begin(); i != tables.end(); i++,n++) {
		table->setItem(n, 0, new QStandardItem(QString::number(n)));
		table->setItem(n, 1, new QStandardItem(QString::fromStdString((*i).IP)));
		table->setItem(n, 2, new QStandardItem(QString::number((*i).ID)));
	}
	size = chord->getID().size();
}

void MainWindow::draw1(QPainter*paint, list<FingerNode>findlist) {//画线函数
	int rx = 550, ry = 270, r = 230;
	if (first == 1 || first == 2) {
		paint->setPen(QPen(Qt::green, 2, Qt::DashLine));
	}
	else {
		paint->setPen(QPen(Qt::gray, 2, Qt::DashLine));
	}
		for (auto i = findlist.begin(); i != findlist.end(); ) {
			auto t = i++;
			if (i == findlist.end()) {
				int px = (int)(r*cos(((*t).ID + 1) * 360 / 2048 * 3.1415926 / 180)) + rx;
				int py = (int)(r*sin(((*t).ID + 1) * 360 / 2048 * 3.1415926 / 180)) + ry;
				QPoint point(px, py);
				QPoint point2(px, py);
				QLine q(point, point2);
				paint->drawLine(q);
				break;
			}
			int px = (int)(r*cos(((*i).ID + 1) * 360 / 2048 * 3.1415926 / 180)) + rx;
			int py = (int)(r*sin(((*i).ID + 1) * 360 / 2048 * 3.1415926 / 180)) + ry;
			int pxt = (int)(r*cos(((*t).ID + 1) * 360 / 2048 * 3.1415926 / 180)) + rx;
			int pyt = (int)(r*sin(((*t).ID + 1) * 360 / 2048 * 3.1415926 / 180)) + ry;
			QPoint point(px, py);
			QPoint point2(pxt, pyt);
			QLine q(point, point2);
			paint->drawLine(q);
		}
	
}

void MainWindow::draw(QPainter*paint,list<FingerNode>IDlists) {//画点函数
	int rx = 550, ry = 270, r = 230;
	int t = 0;
	for (auto i = IDlists.begin(); i != IDlists.end(); i++,t++) {
		if ((*i).ID == 693) {
			paint->setPen(QPen(Qt::red, 2, Qt::DashLine));
		}
		else if(t>=size){
			paint->setPen(QPen(Qt::black, 2, Qt::DashLine));
		}
		else {
			paint->setPen(QPen(Qt::blue, 2, Qt::DashLine));
		}
		int px = (int)(r*cos(((*i).ID + 1) * 360 / 2048 * 3.1415926 / 180)) + rx;
		int py = (int)(r*sin(((*i).ID + 1) * 360 / 2048 * 3.1415926 / 180)) + ry;
		QPoint point(px, py);
		paint->drawEllipse(px-3,py-3, 3, 3);
		QString s = QString::number((*i).ID) +" IP:" +QString::fromStdString((*i).IP);
		paint->drawText(point, s);
	}
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::maketableview() {//创建tableview
	table = new QStandardItemModel();
	table->setHorizontalHeaderItem(0, new QStandardItem(QObject::tr("index")));
	table->setHorizontalHeaderItem(1, new QStandardItem(QObject::tr("IP")));
	table->setHorizontalHeaderItem(2, new QStandardItem(QObject::tr("ID")));
	ui->treeView->setModel(table);
	ui->treeView->setColumnWidth(0, 70);
	ui->treeView->setColumnWidth(1, 100);
	ui->treeView->setColumnWidth(2, 80);
}


void MainWindow::paintEvent(QPaintEvent *event)//绘图事件
{
	QPainter*paint = new QPainter(this);
	list<FingerNode>IDlist = chord->getID();
	draw(paint, IDlist);
	if (first == 1) {

		list<FingerNode>Findlist = chord->Findlist();
		draw1(paint, Findlist);
	}
	if (first == 2) {
		list<FingerNode>Joinlist = chord->GetJoinlist();
		draw1(paint, Joinlist);
	}
}


//插入节点
void MainWindow::on_pushButton_clicked()
{
    QString IP=ui->lineEdit->text();
	std::regex pattern("(\\d{1,3}.(\\d{1,3}).(\\d{1,3}).(\\d{1,3}))");
	std::smatch s;
	bool ret = std::regex_match(IP.toStdString(), pattern);
	if (ret) {
		chord->Join(IP.toStdString(), 1);
		int hash = chord->BKDHash(IP.toStdString()) % 2048;
		list<FingerNode>tables = chord->Gettable(hash);
		int n = 0;
		for (auto i = tables.begin(); i != tables.end(); i++) {
			table->setItem(n, 0, new QStandardItem(QString::number(n)));
			table->setItem(n, 1, new QStandardItem(QString::fromStdString((*i).IP)));
			table->setItem(n, 2, new QStandardItem(QString::number((*i).ID)));
			n++;
		}
		QString str = "IP: " + IP + " ID: " + QString::number(hash);
		ui->label_7->setText(str);
		first = 2;
		list<FingerNode>Joinlist = chord->GetJoinlist();
		QString s = "";
		for (auto i = Joinlist.begin(); i != Joinlist.end(); i++) {
			s += QString::number((*i).ID) + " IP:" + QString::fromStdString((*i).IP) + "-->";
		}
		s = s.left(s.size() - 3);
		ui->label_8->setText(s);
	}
	else {
		
		QString str = QString::fromLocal8Bit("无效的输入");
		ui->label_8->setText(str);
	}
}

//删除节点
void MainWindow::on_pushButton_2_clicked()
{
    QString delname=ui->lineEdit_2->text();
	if (chord->Leave(delname.toStdString()))
	{
		string IP = "127.0.0.1";
		int hash = chord->BKDHash(IP) % 2048;
		list<FingerNode>tables = chord->Gettable(hash);
		int n = 0;
		for (auto i = tables.begin(); i != tables.end(); i++) {
			table->setItem(n, 0, new QStandardItem(QString::number(n)));
			table->setItem(n, 1, new QStandardItem(QString::fromStdString((*i).IP)));
			table->setItem(n, 2, new QStandardItem(QString::number((*i).ID)));
			n++;
		}
		first = 0;
	}
	else {
		QString str = QString::fromLocal8Bit("没有该节点");
		ui->label_8->setText(str);
		ui->label_9->setText(" ");
		ui->label_7->setText(" ");
	}
}


//查找信息
void MainWindow::on_pushButton_3_clicked()
{
	QString key = ui->lineEdit_3->text();
	std::regex e("[a-zA-Z]+");

	bool ret = std::regex_match(key.toStdString(), e);
	if (ret) {
		bool find = chord->LookUp(key.toStdString());
		first = 1;
		if (find) {
			int hash = chord->BKDHashs(key.toStdString()) % 2048;
			int suc = chord->findsuccessor(hash, 0);
			list<FingerNode>findlist = chord->Findlist();
			auto t = findlist.back();
			list<store<string, string>>keystore = chord->Getstore(suc);
			bool notfind = true;
			for (auto i : keystore) {
				if (i.key == key.toStdString()) {
					QString value = QString::fromLocal8Bit(i.value.c_str());
					QString str = "key: " + key + "  value: " + value + "  K: " + QString::number(hash);
					ui->label_7->setText(str);
					str = QString::fromLocal8Bit("存储在节点ID:") + QString::number(t.ID) + " IP:" + QString::fromStdString(t.IP);
					ui->label_9->setText(str);
					break;
				}
			}
			QString s = " ";
			for (auto i = findlist.begin(); i != findlist.end(); i++) {
				s += QString::number((*i).ID) + " IP:" + QString::fromStdString((*i).IP) + "-->";
			}
			s = s.left(s.size() - 3);
			ui->label_8->setText(s);
		}
		else {
			QString str = QString::fromLocal8Bit("网络中没有该节点!!");
			ui->label_7->setText(str);
			ui->label_8->setText(str);
			ui->label_9->setText(" ");
		}
	}
	else {
		QString str = QString::fromLocal8Bit("无效的输入");
		ui->label_8->setText(str);
	}
}
//更新信息
void MainWindow::on_pushButton_4_clicked()
{
    QString upkey=ui->lineEdit_4->text();
	std::regex e("[a-zA-Z]+");
	bool ret = std::regex_match(upkey.toStdString(), e);
	if (ret) {
		QString new_value = ui->lineEdit_5->text();
			if (chord->Update(upkey.toLocal8Bit().toStdString(), new_value.toLocal8Bit().toStdString())) {
				first = 1;
				int K = chord->BKDHashs(upkey.toStdString()) % 2048;
				QString str = "key: " + upkey + "  value: " + new_value + "  K: " + QString::number(K);
				ui->label_7->setText(str);
				int succ = chord->findsuccessor(K, 0);
				list<FingerNode>findlist = chord->Findlist();
				auto t = findlist.back();
				str = QString::fromLocal8Bit("储存在节点:ID") + QString::number(succ) + " IP:" + QString::fromStdString(t.IP);
				ui->label_9->setText(str);
				QString s = "";
				for (auto i = findlist.begin(); i != findlist.end(); i++) {
					s += QString::number((*i).ID) + " IP:" + QString::fromStdString((*i).IP) + "-->";
				}
				s = s.left(s.size() - 3);
				ui->label_8->setText(s);
			}
			else {
				QString str = QString::fromLocal8Bit("网络中没有该节点");
				ui->label_7->setText(str);
				ui->label_8->setText(str);
				ui->label_9->setText(" ");
			}
		
	}
	else {
		QString str = QString::fromLocal8Bit("无效的输入");
		ui->label_8->setText(str);
	}
}
//插入信息
void MainWindow::on_pushButton_5_clicked()
{
	QString indkey=ui->lineEdit_6->text();
	std::regex e("[a-zA-Z]+");
	bool ret = std::regex_match(indkey.toStdString(), e);
	if (ret) {
		QString value = ui->lineEdit_7->text();
		chord->Insert(indkey.toLocal8Bit().toStdString(), value.toLocal8Bit().toStdString());
		first = 1;
		int K = chord->BKDHashs(indkey.toStdString()) % 2048;
		QString str = "key: " + indkey + "  value: " + value + " K: " + QString::number(K);
		ui->label_7->setText(str);
		list<FingerNode>findlist = chord->Findlist();
		int succ = chord->findsuccessor(K, 0);
		auto t = findlist.back();
		str = QString::fromLocal8Bit("储存在节点:ID") + QString::number(t.ID) + " IP:" + QString::fromStdString(t.IP);
		ui->label_9->setText(str);
		QString s = "";
		for (auto i = findlist.begin(); i != findlist.end(); i++) {
			s += QString::number((*i).ID) + " IP:" + QString::fromStdString((*i).IP) + "-->";
		}
		s = s.left(s.size() - 3);
		ui->label_8->setText(s);
	}
	else {
		QString str = QString::fromLocal8Bit("无效的输入");
		ui->label_8->setText(str);
	}
}


