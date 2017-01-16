#include <QtGui>
#include <QDebug>
#include <QString>
#include <QPalette>
#include <string.h>
#include "sae_register_dlg.h"
#include "helper.h"

#if 0
#define STAKE_TABLE_HASNOTBIKE(n)   	label_stakeNO##n->setPixmap(QPixmap(QString::fromUtf8(":/image/hasnotbike.png")))
#define STAKE_TABLE_HASBIKE(n)   		label_stakeNO##n->setPixmap(QPixmap(QString::fromUtf8(":/image/hasbike.png")))
#define STAKE_TABLE_NOTREGISTER(n)   	label_stakeNO##n->setPixmap(QPixmap(QString::fromUtf8(":/image/notregister.png")))
#endif

#define STAKE_TABLE_HASNOTBIKE1(id,fw,bl,para)   {\
                                            label_stakeNO##id->setStyleSheet("font: 75 13pt \"文泉驿正黑\"; border-width: 1px; border-style: solid; border-color: rgb(0, 0, 0); background-color: rgb(235, 165, 43); color: rgb(255, 255, 255); " );\
                                            label_stakeNO##id->setText(tr("%1\nF:%2\nB:%3\nP:%4\n无车").arg(id).arg(fw).arg(bl).arg(para));\
                                            label_stakeNO##id->setAlignment(Qt::AlignLeft);\
                                            label_stakeNO##id->setAlignment(Qt::AlignVCenter);\
                                            label_stakeNO##id->setFont(font);\
                                            label_stakeNO##id->resize(60,100);\
                                        }

#define STAKE_TABLE_HASBIKE1(id,fw,bl,para)  {\
                                            label_stakeNO##id->setStyleSheet("font: 75 13pt \"文泉驿正黑\"; border-width: 1px; border-style: solid; border-color: rgb(0, 0, 0); background-color: rgb(107, 184, 84); color: rgb(255, 255, 255);");\
                                            label_stakeNO##id->setText(tr("%1\nF:%2\nB:%3\nP:%4\n有车").arg(id).arg(fw).arg(bl).arg(para));\
                                            label_stakeNO##id->setAlignment(Qt::AlignLeft);\
                                            label_stakeNO##id->setAlignment(Qt::AlignVCenter);\
                                            label_stakeNO##id->resize(60,100);\
                                        }
#define STAKE_TABLE_NOTREGISTER1(id,fw,bl,para)  {\
                                            label_stakeNO##id->setStyleSheet("font: 75 13pt \"文泉驿正黑\"; border-width: 1px; border-style: solid; border-color: rgb(0, 0, 0); background-color: rgb(134, 134, 134); color: rgb(255, 255, 255);");\
                                            label_stakeNO##id->setText(tr("%1\nF:%2\nB:%3\nP:%4\n未注册").arg(id).arg(fw).arg(bl).arg(para));\
                                            label_stakeNO##id->setAlignment(Qt::AlignLeft);\
                                            label_stakeNO##id->setAlignment(Qt::AlignVCenter);\
                                            label_stakeNO##id->resize(60,100);\
                                        }

#define STAKE_TABLE_NOTREGISTER2(id,fw,bl,para)  {\
                                            label_stakeNO##id->setStyleSheet("font: 75 13pt \"文泉驿正黑\"; border-width: 1px; border-style: solid; border-color: rgb(0, 0, 0); background-color: rgb(134, 134, 134); color: rgb(0, 0, 0);");\
                                            label_stakeNO##id->setText(tr("%1\nF:%2\nB:%3\nP:%4\n未注册").arg(id).arg(fw).arg(bl).arg(para));\
                                            label_stakeNO##id->setAlignment(Qt::AlignLeft);\
                                            label_stakeNO##id->setAlignment(Qt::AlignVCenter);\
                                            label_stakeNO##id->resize(60,100);\
                                        }

/* 基本信息刷新时间  单位:秒 */
#define BINFO_REFRESH_PERIOD				(10)

extern lib_gui_t *g_gui;

SaeRegisterDlg::SaeRegisterDlg(QWidget *parent) : QDialog(parent)
{
	setupUi(this);
	FormHelper::__FormHideTitleBar(this);

	refreshFlag = 1;
	
	__getPageConfig();

   	pa.setColor(QPalette::WindowText,Qt::white);
    	font.setBold(true);
	
	pushButton_manualRefresh->installEventFilter(this);

	connect(pushButton_manualRefresh, SIGNAL(clicked()), this, SLOT(slots_manualRefresh()));

	QString string;
	timerCount = BINFO_REFRESH_PERIOD;
	string.setNum(timerCount);
	label_refreshPeriod->setText(string);
	
	connect(&timer, SIGNAL(timeout()), this, SLOT(slots_timerDone()));
	timer.start(1000);  //定时1秒
}

SaeRegisterDlg::~SaeRegisterDlg(void)
{

}

void SaeRegisterDlg::slots_manualRefresh(void)
{
	refreshFlag = 0;
	
	timerCount = BINFO_REFRESH_PERIOD;

	if(__getPageConfig() > 0)
		__showConfigResult(DEV_W_CONFIG_RESULT_REFRESH);  //刷新成功

	refreshFlag = 1;
}

void SaeRegisterDlg::slots_timerDone(void)
{
	QString string;

	if(refreshFlag == 1) //手动刷新时候,不执行自动刷新
	{
		string.setNum(timerCount);
		label_refreshPeriod->setText(string);
		timerCount--;
		if(timerCount == 0)
			slots_manualRefresh();

		if(timerCount <= 0)
			timerCount = BINFO_REFRESH_PERIOD;

		if(timerCount == (BINFO_REFRESH_PERIOD / 2))
			label_saeRegisterShow->clear(); //清空信息显示
	}
}

void SaeRegisterDlg::__showConfigResult(const unsigned char result)
{
	FormHelper::__FormSetFont_wqy(20, QFont::Normal, label_saeRegisterShow);
	
	switch(result)
	{
		case DEV_R_CONFIG_RESULT_OK:
		{
			label_saeRegisterShow->setText(tr("读取锁桩签到信息成功"));	
		}
		break;

		case DEV_R_CONFIG_RESULT_TIMEOUT:
		{
			label_saeRegisterShow->setText(tr("读取锁桩签到信息超时"));	
		}
		break;

		case DEV_W_CONFIG_RESULT_REFRESH:
		{
			label_saeRegisterShow->setText(tr("刷新锁桩签到信息成功"));	
		}
		break;

		case DEV_R_SAE_STATUS_ERROR:
		{
			label_saeRegisterShow->setText(tr("读取对比信息失败"));	
		}
		break;
		
	}	
}


int SaeRegisterDlg::__getPageConfig(void)
{
    //__allWidgetClear();
    
    	char fw[8];
    	char bl[8] = {0};
	char para[8] = {0}; 

	memset(comp, 0, sizeof(gui_sae_comparison_status_t)*65);
    	retval = lib_gui_get_sae_comparison_status(g_gui, comp, 500);	
	if(retval != LIB_GUI_EOK)
	{
		__showConfigResult(DEV_R_SAE_STATUS_ERROR);
		return retval;
	}
		
	memset(&sPageConfig, 0, sizeof(struct gui_stake_info_page_config));
   	retval = lib_gui_get_stake_info_page_config(g_gui, &sPageConfig, 500);  //获取锁桩信息  (2000改成800)
	if(retval == LIB_GUI_EOK)
	{
		unsigned int i;
		unsigned int total = sPageConfig.quantity;
		unsigned int registered = 0;
		unsigned int not_registered = 0;
		unsigned int hasbike = 0;
		unsigned int hasnotbike = 0;  

		for(i = 1; i <= total; i++)
		{
			if(helper_chk_bit(sPageConfig.s_stat, i) == 1) //检查注册情况
			{
		               if(comp[i].fw==1)
		                   strcpy(fw,"1");
		               else
		                   strcpy(fw,"0");

		                if(comp[i].bl_total==1)
		                    strcpy(bl,"1");
		                else
		                    strcpy(bl,"0");

		                if(comp[i].bl_inc==1)
		                    strcat(bl,"1");
		                else
		                    strcat(bl,"0");

		                if(comp[i].bl_dec==1)
		                    strcat(bl,"1");
		                else
		                    strcat(bl,"0");

		                if(comp[i].bl_temp==1)
		                    strcat(bl,"1");
		                else
		                    strcat(bl,"0");

				 if(comp[i].stake_para ==1)
		                    strcpy(para,"1");
		                else
		                    strcpy(para,"0");
						
                		if(helper_chk_bit(sPageConfig.s_bicycle_exist_stat, i) == 1)  //检查车情况
				{
					hasbike++;
                    			__showTableHasBike(i,fw,bl,para); //有车
				}
				else
                		{
					hasnotbike++;
                   			 __showTableHasNotBike(i,fw,bl,para); //无车
				}

				registered++;
			}
            		else
			{
				not_registered++;
				__showTableNotRegister(i); //未注册
			}
		}

        __showSaeStatistics(total, registered, not_registered, hasbike, hasnotbike); 
		
		__showConfigResult(DEV_R_CONFIG_RESULT_OK);

	}
	else if(retval == LIB_GUI_ETIMEOUT) //读取超时
	{
		__showConfigResult(DEV_R_CONFIG_RESULT_TIMEOUT);
	}

	
	return retval;	
}
void SaeRegisterDlg::__showSaeStatistics(const unsigned int total, const unsigned int registered, const unsigned int not_registered, const unsigned int hasbike, const unsigned int hasnotbike)
{
	QString s_statistics;
	
    s_statistics.sprintf("此站点共%d个锁桩，已签到：%d个，未签到：%d个，有车：%d个，无车：%d个", total, registered, not_registered, hasbike, hasnotbike);
	
	label_saeStatistics->setText(s_statistics);	
}

bool SaeRegisterDlg::eventFilter(QObject *watched, QEvent *event)
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
	
	return QWidget::eventFilter(watched,event); 
}

void SaeRegisterDlg::__showTableNotRegister(const unsigned int idx)
{
	switch(idx)
	{
        case 1:
            STAKE_TABLE_NOTREGISTER1(1,"0","0000", "0");
			break;

		case 2:
            STAKE_TABLE_NOTREGISTER1(2,"0","0000", "0");
			break;

        case 3:
            STAKE_TABLE_NOTREGISTER1(3,"0","0000", "0");
            break;

		case 4:
            STAKE_TABLE_NOTREGISTER1(4,"0","0000", "0");
			break;

		case 5:
            STAKE_TABLE_NOTREGISTER1(5,"0","0000", "0");
			break;

		case 6:
            STAKE_TABLE_NOTREGISTER1(6,"0","0000", "0");
			break;

		case 7:
            STAKE_TABLE_NOTREGISTER1(7,"0","0000", "0");
			break;
			
		case 8:
            STAKE_TABLE_NOTREGISTER1(8,"0","0000", "0");
			break;

		case 9:
            STAKE_TABLE_NOTREGISTER1(9,"0","0000", "0");
			break;

		case 10:
            STAKE_TABLE_NOTREGISTER1(10,"0","0000", "0");
			break;

		case 11:
            STAKE_TABLE_NOTREGISTER1(11,"0","0000", "0");
			break;

		case 12:
            STAKE_TABLE_NOTREGISTER1(12,"0","0000", "0");
			break;

		case 13:
            STAKE_TABLE_NOTREGISTER1(13,"0","0000", "0");
			break;

		case 14:
            STAKE_TABLE_NOTREGISTER1(14,"0","0000", "0");
			break;

		case 15:
            STAKE_TABLE_NOTREGISTER1(15,"0","0000", "0");
			break;

		case 16:
            STAKE_TABLE_NOTREGISTER1(16,"0","0000", "0");
			break;

		case 17:
            STAKE_TABLE_NOTREGISTER1(17,"0","0000", "0");
			break;
			
		case 18:
            STAKE_TABLE_NOTREGISTER1(18,"0","0000", "0");
			break;

		case 19:
            STAKE_TABLE_NOTREGISTER1(19,"0","0000", "0");
			break;

		case 20:
            STAKE_TABLE_NOTREGISTER1(20,"0","0000", "0");
			break;		

		case 21:
            STAKE_TABLE_NOTREGISTER1(21,"0","0000", "0");
			break;

		case 22:
            STAKE_TABLE_NOTREGISTER1(22,"0","0000", "0");
			break;

		case 23:
            STAKE_TABLE_NOTREGISTER1(23,"0","0000", "0");
			break;

		case 24:
            STAKE_TABLE_NOTREGISTER1(24,"0","0000", "0");
			break;

		case 25:
            STAKE_TABLE_NOTREGISTER1(25,"0","0000", "0");
			break;

		case 26:
            STAKE_TABLE_NOTREGISTER1(26,"0","0000", "0");
			break;

		case 27:
            STAKE_TABLE_NOTREGISTER1(27,"0","0000", "0");
			break;
			
		case 28:
            STAKE_TABLE_NOTREGISTER1(28,"0","0000", "0");
			break;

		case 29:
            STAKE_TABLE_NOTREGISTER1(29,"0","0000", "0");
			break;

		case 30:
            STAKE_TABLE_NOTREGISTER1(30,"0","0000", "0");
			break;

		case 31:
            STAKE_TABLE_NOTREGISTER1(31,"0","0000", "0");
			break;

		case 32:
            STAKE_TABLE_NOTREGISTER1(32,"0","0000", "0");
			break;

		case 33:
            STAKE_TABLE_NOTREGISTER1(33,"0","0000", "0");
			break;

		case 34:
            STAKE_TABLE_NOTREGISTER1(34,"0","0000", "0");
			break;

		case 35:
            STAKE_TABLE_NOTREGISTER1(35,"0","0000", "0");
			break;

		case 36:
            STAKE_TABLE_NOTREGISTER1(36,"0","0000", "0");
			break;

		case 37:
            STAKE_TABLE_NOTREGISTER1(37,"0","0000", "0");
			break;
			
        case 38:
            STAKE_TABLE_NOTREGISTER1(38,"0","0000", "0");
			break;

		case 39:
            STAKE_TABLE_NOTREGISTER1(39,"0","0000", "0");
			break;

		case 40:
            STAKE_TABLE_NOTREGISTER1(40,"0","0000", "0");
			break;

		case 41:
            STAKE_TABLE_NOTREGISTER1(41,"0","0000", "0");
			break;

		case 42:
            STAKE_TABLE_NOTREGISTER1(42,"0","0000", "0");
			break;

		case 43:
            STAKE_TABLE_NOTREGISTER1(43,"0","0000", "0");
			break;

		case 44:
            STAKE_TABLE_NOTREGISTER1(44,"0","0000", "0");
			break;

		case 45:
            STAKE_TABLE_NOTREGISTER1(45,"0","0000", "0");
			break;

		case 46:
            STAKE_TABLE_NOTREGISTER1(46,"0","0000", "0");
			break;

		case 47:
            STAKE_TABLE_NOTREGISTER1(47,"0","0000", "0");
			break;
			
		case 48:
            STAKE_TABLE_NOTREGISTER1(48,"0","0000", "0");
			break;

		case 49:
            STAKE_TABLE_NOTREGISTER1(49,"0","0000", "0");
			break;

		case 50:
            STAKE_TABLE_NOTREGISTER1(50,"0","0000", "0");
			break;
			
		case 51:
            STAKE_TABLE_NOTREGISTER1(51,"0","0000", "0");
			break;

		case 52:
            STAKE_TABLE_NOTREGISTER1(52,"0","0000", "0");
			break;

		case 53:
            STAKE_TABLE_NOTREGISTER1(53,"0","0000", "0");
			break;

		case 54:
            STAKE_TABLE_NOTREGISTER1(54,"0","0000", "0");
			break;

		case 55:
            STAKE_TABLE_NOTREGISTER1(55,"0","0000", "0");
			break;

		case 56:
            STAKE_TABLE_NOTREGISTER1(56,"0","0000", "0");
			break;

		case 57:
            STAKE_TABLE_NOTREGISTER1(57,"0","0000", "0");
			break;
			
		case 58:
            STAKE_TABLE_NOTREGISTER1(58,"0","0000", "0");
			break;

		case 59:
            STAKE_TABLE_NOTREGISTER1(59,"0","0000", "0");
			break;

		case 60:
            STAKE_TABLE_NOTREGISTER1(60,"0","0000", "0");
			break;	

		case 61:
            STAKE_TABLE_NOTREGISTER1(61,"0","0000", "0");
			break;

		case 62:
            STAKE_TABLE_NOTREGISTER1(62,"0","0000", "0");
			break;

		case 63:
            STAKE_TABLE_NOTREGISTER1(63,"0","0000", "0");
			break;

		case 64:
            STAKE_TABLE_NOTREGISTER1(64,"0","0000", "0");
			break;
	}
}

void SaeRegisterDlg::__showTableHasNotBike(const unsigned int idx,char *fw, char *bl, char *para)
{
	switch(idx)
	{
		case 1:
            STAKE_TABLE_HASNOTBIKE1(1,fw,bl,para);
			break;

		case 2:
            STAKE_TABLE_HASNOTBIKE1(2,fw,bl,para);
			break;

		case 3:
            STAKE_TABLE_HASNOTBIKE1(3,fw,bl,para);
			break;

		case 4:
            STAKE_TABLE_HASNOTBIKE1(4,fw,bl,para);
			break;

		case 5:
            STAKE_TABLE_HASNOTBIKE1(5,fw,bl,para);
			break;

		case 6:
            STAKE_TABLE_HASNOTBIKE1(6,fw,bl,para);
			break;

		case 7:
            STAKE_TABLE_HASNOTBIKE1(7,fw,bl,para);
			break;
			
		case 8:
            STAKE_TABLE_HASNOTBIKE1(8,fw,bl,para);
			break;

		case 9:
            STAKE_TABLE_HASNOTBIKE1(9,fw,bl,para);
			break;

		case 10:
            STAKE_TABLE_HASNOTBIKE1(10,fw,bl,para);
			break;

		case 11:
            STAKE_TABLE_HASNOTBIKE1(11,fw,bl,para);
			break;

		case 12:
            STAKE_TABLE_HASNOTBIKE1(12,fw,bl,para);
			break;

		case 13:
            STAKE_TABLE_HASNOTBIKE1(13,fw,bl,para);
			break;

		case 14:
            STAKE_TABLE_HASNOTBIKE1(14,fw,bl,para);
			break;

		case 15:
            STAKE_TABLE_HASNOTBIKE1(15,fw,bl,para);
			break;

		case 16:
            STAKE_TABLE_HASNOTBIKE1(16,fw,bl,para);
			break;

		case 17:
            STAKE_TABLE_HASNOTBIKE1(17,fw,bl,para);
			break;
			
		case 18:
            STAKE_TABLE_HASNOTBIKE1(18,fw,bl,para);
			break;

		case 19:
            STAKE_TABLE_HASNOTBIKE1(19,fw,bl,para);
			break;

		case 20:
            STAKE_TABLE_HASNOTBIKE1(20,fw,bl,para);
			break;		

		case 21:
            STAKE_TABLE_HASNOTBIKE1(21,fw,bl,para);
			break;

		case 22:
            STAKE_TABLE_HASNOTBIKE1(22,fw,bl,para);
			break;

		case 23:
            STAKE_TABLE_HASNOTBIKE1(23,fw,bl,para);
			break;

		case 24:
            STAKE_TABLE_HASNOTBIKE1(24,fw,bl,para);
			break;

		case 25:
            STAKE_TABLE_HASNOTBIKE1(25,fw,bl,para);
			break;

		case 26:
            STAKE_TABLE_HASNOTBIKE1(26,fw,bl,para);
			break;

		case 27:
            STAKE_TABLE_HASNOTBIKE1(27,fw,bl,para);
			break;
			
		case 28:
            STAKE_TABLE_HASNOTBIKE1(28,fw,bl,para);
			break;

		case 29:
            STAKE_TABLE_HASNOTBIKE1(29,fw,bl,para);
			break;

		case 30:
            STAKE_TABLE_HASNOTBIKE1(30,fw,bl,para);
			break;

		case 31:
            STAKE_TABLE_HASNOTBIKE1(31,fw,bl,para);
			break;

		case 32:
            STAKE_TABLE_HASNOTBIKE1(32,fw,bl,para);
			break;

		case 33:
            STAKE_TABLE_HASNOTBIKE1(33,fw,bl,para);
			break;

		case 34:
            STAKE_TABLE_HASNOTBIKE1(34,fw,bl,para);
			break;

		case 35:
            STAKE_TABLE_HASNOTBIKE1(35,fw,bl,para);
			break;

		case 36:
            STAKE_TABLE_HASNOTBIKE1(36,fw,bl,para);
			break;

		case 37:
            STAKE_TABLE_HASNOTBIKE1(37,fw,bl,para);
			break;
			
		case 38:
            STAKE_TABLE_HASNOTBIKE1(38,fw,bl,para);
			break;

		case 39:
            STAKE_TABLE_HASNOTBIKE1(39,fw,bl,para);
			break;

		case 40:
            STAKE_TABLE_HASNOTBIKE1(40,fw,bl,para);
			break;

		case 41:
            STAKE_TABLE_HASNOTBIKE1(41,fw,bl,para);
			break;

		case 42:
            STAKE_TABLE_HASNOTBIKE1(42,fw,bl,para);
			break;

		case 43:
            STAKE_TABLE_HASNOTBIKE1(43,fw,bl,para);
			break;

		case 44:
            STAKE_TABLE_HASNOTBIKE1(44,fw,bl,para);
			break;

		case 45:
            STAKE_TABLE_HASNOTBIKE1(45,fw,bl,para);
			break;

		case 46:
            STAKE_TABLE_HASNOTBIKE1(46,fw,bl,para);
			break;

		case 47:
            STAKE_TABLE_HASNOTBIKE1(47,fw,bl,para);
			break;
			
		case 48:
            STAKE_TABLE_HASNOTBIKE1(48,fw,bl,para);
			break;

		case 49:
            STAKE_TABLE_HASNOTBIKE1(49,fw,bl,para);
			break;

		case 50:
            STAKE_TABLE_HASNOTBIKE1(50,fw,bl,para);
			break;
			
		case 51:
            STAKE_TABLE_HASNOTBIKE1(51,fw,bl,para);
			break;

		case 52:
            STAKE_TABLE_HASNOTBIKE1(52,fw,bl,para);
			break;

		case 53:
            STAKE_TABLE_HASNOTBIKE1(53,fw,bl,para);
			break;

		case 54:
            STAKE_TABLE_HASNOTBIKE1(54,fw,bl,para);
			break;

		case 55:
            STAKE_TABLE_HASNOTBIKE1(55,fw,bl,para);
			break;

		case 56:
            STAKE_TABLE_HASNOTBIKE1(56,fw,bl,para);
			break;

		case 57:
            STAKE_TABLE_HASNOTBIKE1(57,fw,bl,para);
			break;
			
		case 58:
            STAKE_TABLE_HASNOTBIKE1(58,fw,bl,para);
			break;

		case 59:
            STAKE_TABLE_HASNOTBIKE1(59,fw,bl,para);
			break;

		case 60:
            STAKE_TABLE_HASNOTBIKE1(60,fw,bl,para);
			break;	
			
		case 61:
            STAKE_TABLE_HASNOTBIKE1(61,fw,bl,para);
			break;

		case 62:
            STAKE_TABLE_HASNOTBIKE1(62,fw,bl,para);
			break;

		case 63:
            STAKE_TABLE_HASNOTBIKE1(63,fw,bl,para);
			break;

		case 64:
            STAKE_TABLE_HASNOTBIKE1(64,fw,bl,para);
			break;

	}
}

void SaeRegisterDlg::__showTableHasBike(const unsigned int idx, char *fw, char *bl,  char *para)
{
	switch(idx)
	{
        case 1:
            STAKE_TABLE_HASBIKE1(1,fw,bl,para);
            break;

        case 2:
            STAKE_TABLE_HASBIKE1(2,fw,bl,para);
            break;

        case 3:
            STAKE_TABLE_HASBIKE1(3,fw,bl,para);
            break;

        case 4:
            STAKE_TABLE_HASBIKE1(4,fw,bl,para);
            break;

        case 5:
            STAKE_TABLE_HASBIKE1(5,fw,bl,para);
            break;

        case 6:
            STAKE_TABLE_HASBIKE1(6,fw,bl,para);
            break;

        case 7:
            STAKE_TABLE_HASBIKE1(7,fw,bl,para);
            break;

        case 8:
            STAKE_TABLE_HASBIKE1(8,fw,bl,para);
            break;

        case 9:
            STAKE_TABLE_HASBIKE1(9,fw,bl,para);
            break;

        case 10:
            STAKE_TABLE_HASBIKE1(10,fw,bl,para);
            break;

        case 11:
            STAKE_TABLE_HASBIKE1(11,fw,bl,para);
            break;

        case 12:
            STAKE_TABLE_HASBIKE1(12,fw,bl,para);
            break;

        case 13:
            STAKE_TABLE_HASBIKE1(13,fw,bl,para);
            break;

        case 14:
            STAKE_TABLE_HASBIKE1(14,fw,bl,para);
            break;

        case 15:
            STAKE_TABLE_HASBIKE1(15,fw,bl,para);
            break;

        case 16:
            STAKE_TABLE_HASBIKE1(16,fw,bl,para);
            break;

        case 17:
            STAKE_TABLE_HASBIKE1(17,fw,bl,para);
            break;

        case 18:
            STAKE_TABLE_HASBIKE1(18,fw,bl,para);
            break;

        case 19:
            STAKE_TABLE_HASBIKE1(19,fw,bl,para);
            break;

        case 20:
            STAKE_TABLE_HASBIKE1(20,fw,bl,para);
            break;

        case 21:
            STAKE_TABLE_HASBIKE1(21,fw,bl,para);
            break;

        case 22:
            STAKE_TABLE_HASBIKE1(22,fw,bl,para);
            break;

        case 23:
            STAKE_TABLE_HASBIKE1(23,fw,bl,para);
            break;

        case 24:
            STAKE_TABLE_HASBIKE1(24,fw,bl,para);
            break;

        case 25:
            STAKE_TABLE_HASBIKE1(25,fw,bl,para);
            break;

        case 26:
            STAKE_TABLE_HASBIKE1(26,fw,bl,para);
            break;

        case 27:
            STAKE_TABLE_HASBIKE1(27,fw,bl,para);
            break;

        case 28:
            STAKE_TABLE_HASBIKE1(28,fw,bl,para);
            break;

        case 29:
            STAKE_TABLE_HASBIKE1(29,fw,bl,para);
            break;

        case 30:
            STAKE_TABLE_HASBIKE1(30,fw,bl,para);
            break;

        case 31:
            STAKE_TABLE_HASBIKE1(31,fw,bl,para);
            break;

        case 32:
            STAKE_TABLE_HASBIKE1(32,fw,bl,para);
            break;

        case 33:
            STAKE_TABLE_HASBIKE1(33,fw,bl,para);
            break;

        case 34:
            STAKE_TABLE_HASBIKE1(34,fw,bl,para);
            break;

        case 35:
            STAKE_TABLE_HASBIKE1(35,fw,bl,para);
            break;

        case 36:
            STAKE_TABLE_HASBIKE1(36,fw,bl,para);
            break;

        case 37:
            STAKE_TABLE_HASBIKE1(37,fw,bl,para);
            break;

        case 38:
            STAKE_TABLE_HASBIKE1(38,fw,bl,para);
            break;

        case 39:
            STAKE_TABLE_HASBIKE1(39,fw,bl,para);
            break;

        case 40:
            STAKE_TABLE_HASBIKE1(40,fw,bl,para);
            break;

        case 41:
            STAKE_TABLE_HASBIKE1(41,fw,bl,para);
            break;

        case 42:
            STAKE_TABLE_HASBIKE1(42,fw,bl,para);
            break;

        case 43:
            STAKE_TABLE_HASBIKE1(43,fw,bl,para);
            break;

        case 44:
            STAKE_TABLE_HASBIKE1(44,fw,bl,para);
            break;

        case 45:
            STAKE_TABLE_HASBIKE1(45,fw,bl,para);
            break;

        case 46:
            STAKE_TABLE_HASBIKE1(46,fw,bl,para);
            break;

        case 47:
            STAKE_TABLE_HASBIKE1(47,fw,bl,para);
            break;

        case 48:
            STAKE_TABLE_HASBIKE1(48,fw,bl,para);
            break;

        case 49:
            STAKE_TABLE_HASBIKE1(49,fw,bl,para);
            break;

        case 50:
            STAKE_TABLE_HASBIKE1(50,fw,bl,para);
            break;

        case 51:
            STAKE_TABLE_HASBIKE1(51,fw,bl,para);
            break;

        case 52:
            STAKE_TABLE_HASBIKE1(52,fw,bl,para);
            break;

        case 53:
            STAKE_TABLE_HASBIKE1(53,fw,bl,para);
            break;

        case 54:
            STAKE_TABLE_HASBIKE1(54,fw,bl,para);
            break;

        case 55:
            STAKE_TABLE_HASBIKE1(55,fw,bl,para);
            break;

        case 56:
            STAKE_TABLE_HASBIKE1(56,fw,bl,para);
            break;

        case 57:
            STAKE_TABLE_HASBIKE1(57,fw,bl,para);
            break;

        case 58:
            STAKE_TABLE_HASBIKE1(58,fw,bl,para);
            break;

        case 59:
            STAKE_TABLE_HASBIKE1(59,fw,bl,para);
            break;

        case 60:
            STAKE_TABLE_HASBIKE1(60,fw,bl,para);
            break;

        case 61:
            STAKE_TABLE_HASBIKE1(61,fw,bl,para);
            break;

        case 62:
            STAKE_TABLE_HASBIKE1(62,fw,bl,para);
            break;

        case 63:
            STAKE_TABLE_HASBIKE1(63,fw,bl,para);
            break;

        case 64:
            STAKE_TABLE_HASBIKE1(64,fw,bl,para);
            break;
        }
}

void SaeRegisterDlg::__allWidgetClear(void)
{
    label_stakeNO1->clear();
    label_stakeNO2->clear();
    label_stakeNO3->clear();
    label_stakeNO4->clear();
    label_stakeNO5->clear();
    label_stakeNO6->clear();
    label_stakeNO7->clear();
    label_stakeNO8->clear();
    label_stakeNO9->clear();

    label_stakeNO11->clear();
    label_stakeNO12->clear();
    label_stakeNO13->clear();
    label_stakeNO14->clear();
    label_stakeNO15->clear();
    label_stakeNO16->clear();
    label_stakeNO17->clear();
    label_stakeNO18->clear();
    label_stakeNO19->clear();

    label_stakeNO21->clear();
    label_stakeNO22->clear();
    label_stakeNO23->clear();
    label_stakeNO24->clear();
    label_stakeNO25->clear();
    label_stakeNO26->clear();
    label_stakeNO27->clear();
    label_stakeNO28->clear();
    label_stakeNO29->clear();

    label_stakeNO31->clear();
    label_stakeNO32->clear();
    label_stakeNO33->clear();
    label_stakeNO34->clear();
    label_stakeNO35->clear();
    label_stakeNO36->clear();
    label_stakeNO37->clear();
    label_stakeNO38->clear();
    label_stakeNO39->clear();

    label_stakeNO41->clear();
    label_stakeNO42->clear();
    label_stakeNO43->clear();
    label_stakeNO44->clear();
    label_stakeNO45->clear();
    label_stakeNO46->clear();
    label_stakeNO47->clear();
    label_stakeNO48->clear();
    label_stakeNO49->clear();

    label_stakeNO51->clear();
    label_stakeNO52->clear();
    label_stakeNO53->clear();
    label_stakeNO54->clear();
    label_stakeNO55->clear();
    label_stakeNO56->clear();
    label_stakeNO57->clear();
    label_stakeNO58->clear();
    label_stakeNO59->clear();

    label_stakeNO61->clear();
    label_stakeNO62->clear();
    label_stakeNO63->clear();
    label_stakeNO64->clear();

}














			
