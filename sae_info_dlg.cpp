#include <QtGui>
#include <QDebug>
#include <QString>
#include <string.h>
#include <QKeyEvent>
#include "sae_info_dlg.h"
#include "helper.h"

#define SHOW_MAX_LINE			(25)

extern lib_gui_t *g_gui;

SaeInfoDlg::SaeInfoDlg(QWidget *parent) : QDialog(parent)
{
	setupUi(this);
	FormHelper::__FormHideTitleBar(this);

	mRegister = 0;
	mTotal = 0;
	mPages = 0;
	mDiv = 0;
	mRema = 0;
	mShowPage = 0;

	memset(&saeInfo, 0, sizeof(struct gui_sae_info) * 65);
	
	label_saeInfoShow->clear();
	FormHelper::__FormSetFont_wqy(18, QFont::Normal, label_saeInfoShow);
		
	tableWidgetSet();
	__getPageConfig();

	pushButton_manualRefresh->installEventFilter(this);
	pushButton_loopBrowse->installEventFilter(this);

	connect(this, SIGNAL(signals_saeInfoAccept()), this, SLOT(accept()));
	connect(this, SIGNAL(signals_saeInfoReject()), this, SLOT(reject()));
	connect(pushButton_manualRefresh, SIGNAL(clicked()), this, SLOT(slots_manualRefresh()));
	connect(pushButton_loopBrowse, SIGNAL(clicked()), this, SLOT(slots_loopBrowse()));
	
}

SaeInfoDlg::~SaeInfoDlg(void)
{

}

int SaeInfoDlg::__getPageConfig(void)
{
	tableWidget->clearContents();  //����ڴ棬�ͷ��ڴ�

	unsigned char i, j;
	memset(&allInfoConfig, 0, sizeof(struct gui_stake_all_info_page_config));
	retval = lib_gui_get_stake_all_info_page_config(g_gui, &allInfoConfig, 3000);  //��ȡ������Ϣ
	if(retval == LIB_GUI_EOK)
	{
		memset(&saeInfo, 0, sizeof(struct gui_sae_info) * 65);
		
		mTotal = allInfoConfig.quantity;

		j = 1;
		for(i = 1; i <= mTotal; i++)
		{
			if(allInfoConfig.info[i].healthy_flag == 1)  //�ж���׮�Ƿ�ǩ��
			{
				mRegister++;
				memcpy(&saeInfo[j], &(allInfoConfig.info[i]), sizeof(struct gui_sae_info));
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
	else if(retval == LIB_GUI_ETIMEOUT) //��ȡ��ʱ
	{
		label_saeInfoShow->setText(tr("��ȡ��׮��Ϣ��ʱ"));	
	}

	return retval;	
}

void SaeInfoDlg::__setPageConfig(void)
{


}

void SaeInfoDlg::tableWidgetSet(void)
{
	/* ȡ���ؼ����� */
	tableWidget->setFocusPolicy(Qt::NoFocus);  //ȡ���ؼ�����
	
	/* ��ֹ�༭��� */
	tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	
	/*���ñ��Ϊ����ѡ��*/
	tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);   

	/* ȥ��ˮƽ������ */
	tableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	tableWidget->horizontalHeader()->setDefaultSectionSize(72);
	tableWidget->horizontalHeader()->setClickable(false);
	tableWidget->verticalHeader()->setDefaultSectionSize(24);
	tableWidget->setShowGrid(true);
	
	tableWidget->setRowCount(25);    //��������

	tableWidget->setColumnWidth(0, 55); //��׮���
	tableWidget->setColumnWidth(4, 67);  //��׮��ˮ��
	tableWidget->setColumnWidth(10, 160);  //ǩ��ʱ��
}

void SaeInfoDlg::slots_manualRefresh(void)
{
	mDiv = 0;
	mRema = 0;
	mPages = 0;
	mShowPage = 0;
	mRegister = 0;
	mTotal = 0;
	
	__getPageConfig();
}

void SaeInfoDlg::slots_loopBrowse(void)
{
	QString stringShow, string;
	unsigned char i, j = 0;
	unsigned char iN = 1;
	
	tableWidget->clearContents();  //����ڴ棬�ͷ��ڴ�
	
	mShowPage++;
	if(mShowPage > mPages)
		mShowPage = 1;

	stringShow.sprintf("%d����׮,  ��ǩ��%d��, �� [%d] ҳ, ����ʾ�� [%d] ҳ", mTotal, mRegister, mPages, mShowPage);
	label_saeInfoShow->setText(stringShow);	

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
		QTableWidgetItem *item_id = new QTableWidgetItem();
		QTableWidgetItem *item_ver = new QTableWidgetItem();
		QTableWidgetItem *item_lock_ver = new QTableWidgetItem();
		QTableWidgetItem *item_psam = new QTableWidgetItem();
		QTableWidgetItem *item_sn = new QTableWidgetItem();
		QTableWidgetItem *item_total_blk_ver = new QTableWidgetItem();
		QTableWidgetItem *item_inc_blk_ver = new QTableWidgetItem();
		QTableWidgetItem *item_dec_blk_ver = new QTableWidgetItem();
		QTableWidgetItem *item_temp_blk_ver = new QTableWidgetItem();
		QTableWidgetItem *item_stake_para = new QTableWidgetItem();
		QTableWidgetItem *item_reg_time = new QTableWidgetItem();
		QTableWidgetItem *item_phy_sn = new QTableWidgetItem();
		
		/* ��׮��� */
		string.sprintf("%d", saeInfo[i].id);
		item_id->setText(string);

		/* ��׮�汾�� */
		Helper::__CharToQString((char *)saeInfo[i].s_ver, string);
		item_ver->setText(string);

		/* ���ذ汾�� */
		Helper::__CharToQString((char *)saeInfo[i].s_lock_ver, string);
		item_lock_ver->setText(string);

		/* PSAM���� */
		Helper::__CharToQString((char *)saeInfo[i].s_psam, string);
		item_psam->setText(string);

		/* ��׮��ˮ�� */
		Helper::__CharToQString((char *)saeInfo[i].s_sn, string);
		item_sn->setText(string);

		/* ���������� */
		Helper::__CharToQString((char *)saeInfo[i].s_total_bl_ver, string);
		item_total_blk_ver->setText(string);

		/* ���������� */
		Helper::__CharToQString((char *)saeInfo[i].s_inc_bl_ver, string);
		item_inc_blk_ver->setText(string);
				
		/* ���������� */
		Helper::__CharToQString((char *)saeInfo[i].s_dec_bl_ver, string);
		item_dec_blk_ver->setText(string);
				
		/* ��ʱ������ */
		Helper::__CharToQString((char *)saeInfo[i].s_temp_bl_ver, string);
		item_temp_blk_ver->setText(string);

		/* ��׮�����汾 */
		Helper::__CharToQString((char *)saeInfo[i].s_stake_para, string);
		item_stake_para->setText(string);

					
		/* ǩ��ʱ�� */
		QDateTime regDateTime = QDateTime::fromTime_t(saeInfo[i].s_reg_time);
		string = regDateTime.toString("yyyy-MM-dd hh:mm:ss");
		item_reg_time->setText(string);

#if 0
		/* ��׮ʱ�� */
		Helper::__CharToQString((char *)saeInfo[i].s_time, string);
		QString s_string;
		s_string.append("20");
		s_string.append(string.left(6));
		s_string.append(" ");
		s_string.append(string.mid(6, 2));
		s_string.append(":");
		s_string.append(string.mid(8, 2));
		s_string.append(":");
		s_string.append(string.mid(10, 2));
		item_time->setText(s_string);
#endif

		/* ��׮������ */
		Helper::__CharToQString((char *)saeInfo[i].s_phy_sn, string);
		item_phy_sn->setText(string);
		
		tableWidget->setItem(j, 0, item_id);
		tableWidget->setItem(j, 1, item_ver);
		tableWidget->setItem(j, 2, item_lock_ver);
		tableWidget->setItem(j, 3, item_psam);
		tableWidget->setItem(j, 4, item_sn);
		tableWidget->setItem(j, 5, item_total_blk_ver);
		tableWidget->setItem(j, 6, item_inc_blk_ver);
		tableWidget->setItem(j, 7, item_dec_blk_ver);
		tableWidget->setItem(j, 8, item_temp_blk_ver);
		tableWidget->setItem(j, 9, item_stake_para);
		tableWidget->setItem(j, 10, item_reg_time);
		tableWidget->setItem(j, 11, item_phy_sn);
		
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
		tableWidget->item(j, 11)->setTextAlignment(Qt::AlignHCenter);	
		
		j++;
		if(j == SHOW_MAX_LINE)
			break;
	}
}

void SaeInfoDlg::keyPressEvent(QKeyEvent *e)
{
	if(e->key() == Qt::Key_Escape)  
		emit signals_saeInfoAccept();
}

bool SaeInfoDlg::eventFilter(QObject *watched, QEvent *event)
{
	if(watched == pushButton_manualRefresh)
	{
		if(event->type()==QEvent::FocusIn) 
		{
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
			pushButton_loopBrowse->setStyleSheet("background-color: rgb(85, 170, 255)");
		}
		else if (event->type()==QEvent::FocusOut)
		{
			pushButton_loopBrowse->setStyleSheet("background-color: rgb(255, 255, 255)");
		}
	}

	 return QWidget::eventFilter(watched,event);
}















