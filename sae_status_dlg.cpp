#include <QtGui>
#include <QDebug>
#include <QString>
#include <string.h>
#include <QKeyEvent>
#include "sae_status_dlg.h"
#include "helper.h"

#define SHOW_MAX_LINE			(25)

extern lib_gui_t *g_gui;


SaeStatusDlg::SaeStatusDlg(QWidget *parent) : QDialog(parent)
{
	setupUi(this);
	FormHelper::__FormHideTitleBar(this);

	mRegister = 0;
	mTotal = 0;
	mPages = 0;
	mDiv = 0;
	mRema = 0;
	mShowPage = 0;
	
	memset(&sstat, 0, sizeof(struct saeStatus) * 65);

	label_saeStatShow->clear();
	FormHelper::__FormSetFont_wqy(18, QFont::Normal, label_saeStatShow);
	
	tableWidgetSet();
	__getPageConfig();

	pushButton_manualRefresh->installEventFilter(this);
	pushButton_loopBrowse->installEventFilter(this);
	
	connect(this, SIGNAL(signals_saeStatAccept()), this, SLOT(accept()));
	connect(this, SIGNAL(signals_saeStatReject()), this, SLOT(reject()));
	connect(pushButton_manualRefresh, SIGNAL(clicked()), this, SLOT(slots_manualRefresh()));
	connect(pushButton_loopBrowse, SIGNAL(clicked()), this, SLOT(slots_loopBrowse()));
}

SaeStatusDlg::~SaeStatusDlg(void)
{

}

int SaeStatusDlg::__getPageConfig(void)
{
	tableWidget->clearContents();  //清空内存，释放内存

	unsigned char i, j;
	memset(&sPageConfig, 0, sizeof(struct gui_stake_info_page_config));
	retval = lib_gui_get_stake_info_page_config(g_gui, &sPageConfig, 2000);  //获取锁在信息
	if(retval == LIB_GUI_EOK)
	{
		memset(&sstat, 0, sizeof(struct saeStatus) * 65);
		
		mTotal = sPageConfig.quantity;
		
		j = 1;
		for(i = 1; i <= mTotal; i++)
		{
			if(helper_chk_bit(sPageConfig.s_stat, i) == 1) //检查注册情况
			{
				mRegister++;
			
				sstat[j].id = i;
				memcpy(&(sstat[j].stat), &sPageConfig.status[i], 2);
				j++;
			}
		}

		if(mRegister > SHOW_MAX_LINE)
		{
			mDiv = mRegister  / SHOW_MAX_LINE;
			mRema = mRegister % SHOW_MAX_LINE;
			mPages = mDiv; 
			if(mRema > 0)
				mPages = mDiv + 1;
		}	
		else
			mPages = 1;

		slots_loopBrowse();
	}
	else if(retval == LIB_GUI_ETIMEOUT) //读取超时
	{
		label_saeStatShow->setText(tr("读取锁桩状态超时"));	
	}



	return retval;
}

void SaeStatusDlg::__setPageConfig(void)
{
	
}

void SaeStatusDlg::tableWidgetSet(void)
{
	/* 取消控件焦点 */
	tableWidget->setFocusPolicy(Qt::NoFocus);  //取消控件焦点
	
	/* 禁止编辑表格 */
	tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	
	/*设置表格为整行选中*/
	tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);   

	/* 去掉水平滚动条 */
	tableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	tableWidget->horizontalHeader()->setDefaultSectionSize(65);
	tableWidget->horizontalHeader()->setClickable(false);
	tableWidget->verticalHeader()->setDefaultSectionSize(24);
	tableWidget->setShowGrid(true);

  	tableWidget->setRowCount(25);     //设置行数为25
   // tableWidget->setColumnCount(5);   //设置列数为5  
	

	tableWidget->setColumnWidth(0, 55); //锁桩流水号
	tableWidget->setColumnWidth(1, 50); //锁舌收
	tableWidget->setColumnWidth(2, 50);  //锁舌身

	
}

void SaeStatusDlg::slots_manualRefresh(void)
{
	mDiv = 0;
	mRema = 0;
	mPages = 0;
	mShowPage = 0;
	mRegister = 0;
	mTotal = 0;
	
	__getPageConfig();
}

void SaeStatusDlg::slots_loopBrowse(void)
{

	QString stringShow, string;
	unsigned char i, j = 0;
	unsigned char iN = 1;
	gui_sae_status_t status;
	
	tableWidget->clearContents();  //清空内存，释放内存
	
	mShowPage++;
	if(mShowPage > mPages)
		mShowPage = 1;

	stringShow.sprintf("%d个锁桩,  已签到%d个, 共 [%d] 页, 现显示第 [%d] 页", mTotal, mRegister, mPages, mShowPage);
	label_saeStatShow->setText(stringShow);	

	switch(mShowPage)
	{
		case 1:
			iN = 1;
			break;

		case 2:
			iN = SHOW_MAX_LINE + 1;
			break;

		case 3:
			iN = SHOW_MAX_LINE * 2 + 1;
			break;
	}

	for(i = iN; i <= mRegister; i++)
	{
		memset(&status, 0, sizeof(gui_sae_status_t));
		memcpy(&status, &(sstat[i].stat), sizeof(gui_sae_status_t));
				
		QTableWidgetItem *item_id = new QTableWidgetItem();
		QTableWidgetItem *item_in = new QTableWidgetItem();
		QTableWidgetItem *item_out = new QTableWidgetItem();
		QTableWidgetItem *item_bike = new QTableWidgetItem();
		QTableWidgetItem *item_op = new QTableWidgetItem();
		QTableWidgetItem *item_rc531 = new QTableWidgetItem();
		QTableWidgetItem *item_lock = new QTableWidgetItem();
		QTableWidgetItem *item_time = new QTableWidgetItem();
		QTableWidgetItem *item_volt = new QTableWidgetItem();
		QTableWidgetItem *item_rtc = new QTableWidgetItem();
		QTableWidgetItem *item_can = new QTableWidgetItem();

		/* 0:正常, 1:异常 */

		/* 锁桩编号 */
		string.sprintf("%d", sstat[i].id);
		item_id->setText(string);

		/* 锁舌收 */
		if(status.in == 0)
			item_in->setText("是");
		else
			item_in->setText("否");

		/* 锁舌伸 */
		if(status.out == 0)
			item_out->setText("是");
		else
			item_out->setText("否");

		if(status.bike == 1)  //有车
			item_bike->setText("是");
		else
			item_bike->setText("否");

		if(status.op == 0)
			item_op->setText("正常");
		else
			item_op->setText("异常");

		if(status.rc531 == 0)
			item_rc531->setText("正常");
		else
			item_rc531->setText("异常");

		if(status.lock == 0)
			item_lock->setText("正常");
		else
			item_lock->setText("异常");

		if(status.time == 0)
			item_time->setText("是");
		else
			item_time->setText("否");

		if(status.volt == 0)
			item_volt->setText("正常");
		else
			item_volt->setText("异常");

		if(status.rtc == 0)
			item_rtc->setText("正常");
		else
			item_rtc->setText("异常");
				
		if(status.can == 0)
			item_can->setText("正常");
		else
			item_can->setText("异常");

		tableWidget->setItem(j, 0, item_id);
		tableWidget->setItem(j, 1, item_in);
		tableWidget->setItem(j, 2, item_out);
		tableWidget->setItem(j, 3, item_bike);
		tableWidget->setItem(j, 4, item_op);
		tableWidget->setItem(j, 5, item_rc531);
		tableWidget->setItem(j, 6, item_lock);
		tableWidget->setItem(j, 7, item_time);
		tableWidget->setItem(j, 8, item_volt);
		tableWidget->setItem(j, 9, item_rtc);
		tableWidget->setItem(j, 10, item_can);
				
		tableWidget->item(j, 0)->setTextAlignment(Qt::AlignHCenter);
		tableWidget->item(j, 1)->setTextAlignment(Qt::AlignHCenter);
		tableWidget->item(j, 2)->setTextAlignment(Qt::AlignHCenter);
		tableWidget->item(j, 3)->setTextAlignment(Qt::AlignHCenter);
		tableWidget->item(j, 4)->setTextAlignment(Qt::AlignHCenter);
		tableWidget->item(j, 5)->setTextAlignment(Qt::AlignHCenter);
		tableWidget->item(j, 6)->setTextAlignment(Qt::AlignHCenter);
		tableWidget->item(j, 7)->setTextAlignment(Qt::AlignHCenter);
		tableWidget->item(j, 8)->setTextAlignment(Qt::AlignHCenter);
		tableWidget->item(j, 9)->setTextAlignment(Qt::AlignHCenter);
		tableWidget->item(j, 10)->setTextAlignment(Qt::AlignHCenter);

		j++;
		if(j == SHOW_MAX_LINE)
			break;
	}
}

void SaeStatusDlg::keyPressEvent(QKeyEvent *e)
{
	if(e->key() == Qt::Key_Escape)  
		emit signals_saeStatAccept();
}

bool SaeStatusDlg::eventFilter(QObject *watched, QEvent *event)
{
	if(watched == pushButton_manualRefresh)
	{
		if(event->type()==QEvent::FocusIn) 
		{
			//pushButton_manualRefresh->setStyleSheet("background-color: rgb(0, 85, 255)");
			pushButton_manualRefresh->setStyleSheet("background-color: rgb(85, 170, 255)");
		}
		else if (event->type()==QEvent::FocusOut)
		{
			pushButton_manualRefresh->setStyleSheet("background-color: rgb(255, 255, 255)");
		}
	}

	if(watched == pushButton_loopBrowse)
	{
		if(event->type()==QEvent::FocusIn) 
		{
			//pushButton_loopBrowse->setStyleSheet("background-color: rgb(0, 85, 255)");
			pushButton_loopBrowse->setStyleSheet("background-color: rgb(85, 170, 255)");
		}
		else if (event->type()==QEvent::FocusOut)
		{
			pushButton_loopBrowse->setStyleSheet("background-color: rgb(255, 255, 255)");
		}
	}

	 return QWidget::eventFilter(watched,event);
}




