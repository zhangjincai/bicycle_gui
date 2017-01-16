#include <QtGui>
#include <QDebug>
#include <QKeyEvent>
#include <QString>
#include <QThreadPool>
#include <stdio.h>
#include "helper.h"
#include "lib_general.h"
#include "delay_return_bicycle_dlg.h"
#include "lib_lnt.h"



#define BINFO_RETURN_PERIOD						(25)
//#define BINFO_RETURN_FAIL_PERIOD				(20)
#define BINFO_REFRESH_PERIOD					(20)
#define WHETHER_DELAY_MAX_EMPTY_SAE_COUNT   	1 //(0.1)

#define ABLE_DELAY_RETUAN_BICYCLE				1
#define UNABLE_DELAY_RETUAN_BICYCLE				2
#define DELAY_RETUAN_BICYCLE_ING				3
#define LNT_HASNO_RENT_BICYCLE					0x22
#define LNT_DELAYED_BICYCLE					0x23
#define LNT_CHECK_DELAYED_BICYCLE                             0x24
#define LNT_DELAYED_BICYCLE_SUCESS                            0x25


extern lib_gui_t *g_gui;

DelayReturnBicycleDlg::DelayReturnBicycleDlg(QWidget *parent) : QDialog(parent)
{
	setupUi(this);
	setWindowFlags(Qt::FramelessWindowHint);	

	pushButton_DelayReturnBicycleStart->installEventFilter(this);
	pushButton_Quit->installEventFilter(this);


	pushButton_Quit->setStyleSheet("background-color: rgb(224, 239, 198)");
	pushButton_DelayReturnBicycleStart->setStyleSheet("background-color: rgb(224, 239, 198)");

	pushButton_Quit->setVisible(false);
	pushButton_DelayReturnBicycleStart->setVisible(false);
	m_ret = LIB_LNT_ERROR;
	m_stat = 0;
	timerFlag = 0;
	adminCardType = LNT_ADMIN_CARD_TYPE_NOT;
        delay_flag = UNABLE_DELAY_RETUAN_BICYCLE;
        SAVE_pid[8] = {0};
		

	__allWidgetClear();
		
	timerCount = BINFO_REFRESH_PERIOD;
	slots_timerDone();
	connect(&timer, SIGNAL(timeout()), this, SLOT(slots_timerDone()));
	timer.start(1000);

        connect(this, SIGNAL(signals_delayAccept()), this, SLOT(accept()));
	connect(this, SIGNAL(signals_delayReject()), this, SLOT(reject()));
	//connect(this, SIGNAL(signals_DelayReturnBicycleQuit()), this, SLOT(accept()));
	//connect(pushButton_DelayReturnBicycleStart, SIGNAL(clicked()), this, SLOT(slots_DelayReturnBicycleStart()));
	//connect(pushButton_Quit, SIGNAL(clicked()), this, SLOT(accept()));
}

DelayReturnBicycleDlg::~DelayReturnBicycleDlg(void)
{
	
}

void DelayReturnBicycleDlg::slots_DelayReturnBicycleStart(void)
{
	
}

void DelayReturnBicycleDlg::__rent_flag_to_bit(struct rent_info *rinfo, struct rent_flag *flag)
{
	if((rinfo == NULL) || (flag == NULL))
		return;


	flag->b0 = rinfo->rent_flag & 0x01;
	flag->b1 = (rinfo->rent_flag >> 1) & 0x01;
	flag->b2 = (rinfo->rent_flag >> 2) & 0x01;
	flag->b3 = (rinfo->rent_flag >> 3) & 0x01;
#if 0
	printf("0x1f & 0x1 = %d\n",0x1f & 0x1);
	printf("0x1f & 0x2 = %d\n",0x1f & 0x2);
	printf("0x1f & 0x4 = %d\n",0x1f & 0x4);
	printf("0x1f & 0x8 = %d\n",0x1f & 0x8);
#endif
}

void DelayReturnBicycleDlg::slots_timerDone(void)
{
	static int ret = LNT_MOC_ERROR_CARD_OP_FAIL;
	QString memberShow;

	if(timerFlag == 0)
	{
		__allWidgetClear();
		
		ret = __getPageConfig();
		if(ret == LNT_MOC_ERROR_CARD_NOTAG)
		{
			memberShow.sprintf("提示:请在%02d秒内,把卡放置到指定刷卡区域", timerCount);
			label_DelayReturnBicycleShow->setText(memberShow);	
		}
		
		if(timerCount == 0)
		{
			label_DelayReturnBicycleShow->setText(tr("提示:会员信息查询超时,自动返回上一级界面, 按[退出]键直接返回"));
			Helper::__Sleep(1);
			timerFlag = 1;
			emit signals_delayReject();
		}

		if(ret != LNT_MOC_ERROR_CARD_NOTAG)
		{
			timerCount = BINFO_RETURN_PERIOD;
			timerFlag = 1;
		}
	}


	if(timerFlag == 1)
	{
        if(ret == LNT_MOC_ERROR_NORMAL)
                memberShow.sprintf("提示:会员信息查询成功,%02d秒后自动返回上一级界面, 按[退出]键直接返回", timerCount);
		else
                        memberShow.sprintf("提示:会员信息查询失败,请退卡, %02d秒后自动返回上一级界面, 按[退出]键直接返回", timerCount);

		if(ret == LNT_MOC_ERROR_M1)
                        memberShow.sprintf("提示:不支持该类型卡操作,请退卡, %02d秒后自动返回上一级界面, 按[退出]键直接返回", timerCount);

		if(ret == LNT_MOC_ERROR_CARD_OP_FAIL)
                        memberShow.sprintf("提示:会员信息查询失败,请退卡, %02d秒后自动返回上一级界面, 按[退出]键直接返回", timerCount);
			
		if(ret == LNT_MOC_ERROR_LILIN_NOT_MEMBER_CARD)
                        memberShow.sprintf("提示:不支持非会员卡操作,请退卡, %02d秒后自动返回上一级界面, 按[退出]键直接返回", timerCount);

		if(ret == LNT_MOC_ERROR_CARD_NAMELIST)
                        memberShow.sprintf("提示:该卡为黑名单卡,请退卡, %02d秒后自动返回上一级界面, 按[退出]键直接返回", timerCount);

		if(ret == LNT_HASNO_RENT_BICYCLE)
                        memberShow.sprintf("提示:该卡未租车,无法执行延时还车操作,请退卡, %02d秒后自动返回上一级界面, 按[退出]键直接返回", timerCount);

		if(ret == LNT_DELAYED_BICYCLE)
                        memberShow.sprintf("提示:该卡已执行过延迟还车操作,请退卡, %02d秒后自动返回上一级界面, 按[退出]键直接返回", timerCount);
		if(ret == LNT_MOC_ERROR_UNKNOW)
                        memberShow.sprintf("提示:该卡租还车标志位异常,请退卡, %02d秒后自动返回上一级界面, 按[退出]键直接返回", timerCount);
        if(ret == LNT_CHECK_DELAYED_BICYCLE)
            memberShow.sprintf("提示:正在检查目前站点的空桩情况,请耐心等待......");

        if((timerCount < 20) && (delay_flag == UNABLE_DELAY_RETUAN_BICYCLE) && (ret != LNT_HASNO_RENT_BICYCLE))
            memberShow.sprintf("提示:目前该站点不满足延时还车条件,请退卡, %02d秒后自动返回上一级界面, 按[退出]键直接返回", timerCount);

        if((timerCount < 20) && (delay_flag == LNT_DELAYED_BICYCLE_SUCESS))
            memberShow.sprintf("提示:执行延时还车成功,请退卡, %02d秒后自动返回上一级界面, 按[退出]键直接返回", timerCount);

        label_DelayReturnBicycleShow->setText(memberShow);

		if(timerCount == 0)
		{
			timerFlag = 2;
			emit signals_delayReject();
		}
#if 1
                //printf("delay_flag:%d, timerFlag:%d\n",delay_flag, timerFlag);
                if((delay_flag != ABLE_DELAY_RETUAN_BICYCLE) && (timerFlag == 1) && (ret != LNT_HASNO_RENT_BICYCLE)) //没有执行过延时还车才允许延时还车
                {   //必须加上(ret != LNT_HASNO_RENT_BICYCLE),不然后面读线程会退出!!!
                    if(timerCount == 20)
                    {
                         //memberShow.sprintf("提示:正在检查目前站点的空桩情况,请耐心等待......");
                         //label_DelayReturnBicycleShow->setText(memberShow);

                        #if 1
                          unsigned int hasnotbike = 0;
                          float percent = 0;
                          QString string;

                          /* 响应延迟还车 ACK */
                          gui_delay_return_bicycle_t bicyle;
                          memset(&bicyle, 0, sizeof(bicyle));
                          lib_gui_delay_return_bicyle_ack(g_gui, &bicyle, 300);

                          fprintf(stderr, "\n\n*********************Delay Return Bicyle**********************\n\n");
                          fprintf(stderr, "quantity:%d\n", bicyle.quantity);
                          fprintf(stderr, "register_count:%d\n", bicyle.register_count);
                          fprintf(stderr, "bicycle_online_count:%d\n", bicyle.bicycle_online_count);
                          fprintf(stderr, "\n\n*********************************************************\n\n");

                          hasnotbike = bicyle.register_count - bicyle.bicycle_online_count;   //空桩数
                          if(bicyle.register_count != 0) //解决除数为0时进程死掉的问题 add by zjc at 2016-10-27
                          	percent = 100 * hasnotbike / bicyle.register_count; //空桩占注册桩的百分比

                          qDebug("----------empty sae:%d",hasnotbike);
                          qDebug("----------empty sae percent:%5.2f%",percent);
                          qDebug("----------limit:%5.2f%",(float)(100 * WHETHER_DELAY_MAX_EMPTY_SAE_COUNT));

                          string.sprintf("%d 个", hasnotbike),
                          label_emptySae->setText(string);

                          if(hasnotbike == 0)	//延时还车条件:空桩数为0
                          //if(percent < (float)(100 * WHETHER_DELAY_MAX_EMPTY_SAE_COUNT))	//延时还车条件
                          {
                                  delay_flag = ABLE_DELAY_RETUAN_BICYCLE;

                                  //修改延迟还车标志位
                                  if(ret != LNT_DELAYED_BICYCLE) //add by zjc at 2016-10-28
                                  {
									label_whetherRetrunDelay->setText("允许");
									memberShow.sprintf("提示:正在执行延时还车处理,请耐心等待......");
                                  	label_DelayReturnBicycleShow->setText(memberShow);
                                  	do_delay_return_bicycle();
								  }
                          }
                          else
                          {
                                  delay_flag = UNABLE_DELAY_RETUAN_BICYCLE;
                                  label_whetherRetrunDelay->setText("不允许");
                          }
                        #endif
                    }
            }
#endif
	}

	timerCount--;
}


int DelayReturnBicycleDlg::__getPageConfig(void)
{
	int retval = LIB_LNT_ERROR;
	m_stat = LNT_MOC_ERROR_CARD_OP_FAIL;
	QString string;
	
	memset(&lnt_qry_ticket, 0, sizeof(lib_lnt_qry_ticket_info_ack_t));
	memset(&lnt_rent_req, 0, sizeof(lib_lnt_rent_info_req_t));
	memset(&lnt_rent_ack, 0, sizeof(lib_lnt_rent_info_ack_t));
	memset(&rf, 0, sizeof(struct rent_flag));
	
	m_ret = lib_lnt_qry_ticket_info(&lnt_qry_ticket, &m_stat, 300);
	if((m_ret > 0) && (m_stat == LNT_MOC_ERROR_CARD_NAMELIST))
	{
		return LNT_MOC_ERROR_CARD_NAMELIST;
	}
	else if((m_ret > 0) && (m_stat == LNT_MOC_ERROR_NORMAL))
	{
		 //printf("lib_lnt_qry_ticket_info success\n");
		 pmflag_t pf;

		 /* 检查一下是否管理卡 */
		 adminCardType = lib_lnt_is_admin_card();
                 if((adminCardType == LNT_ADMIN_CARD_TYPE_RD) || (adminCardType == LNT_ADMIN_CARD_TYPE_RW))
		 	goto TODO;

		if(lnt_qry_ticket.ticket.pursetype == 0x01)  //不支持M1卡
			return LNT_MOC_ERROR_CARD_OP_FAIL;  
			
		memcpy(&pf, &(lnt_qry_ticket.ticket.pmflag), 1);
		if((lnt_qry_ticket.ticket.pmflag != 0xFF) && (pf.pflag != 1)) //会员卡
		{
			return LNT_MOC_ERROR_LILIN_NOT_MEMBER_CARD;  
		}
			
TODO:		

		
		memcpy(SAVE_pid, lnt_qry_ticket.ticket.pid, 8); //保存卡的物理卡号
		
		lnt_rent_req.id = 0x68;
		m_ret = lib_lnt_qry_rent_info(&lnt_rent_req, &lnt_rent_ack, &m_stat, 300);
		if((m_ret > 0) && (m_stat == LNT_MOC_ERROR_NORMAL))
		{
			//printf("lib_lnt_qry_rent_info success\n");
			//lib_printf_v2("RENT ACK:", (unsigned char *)&lnt_rent_ack, sizeof(lnt_rent_ack), 16);
			
			/* 租还车标志 */
			__rent_flag_to_bit(&(lnt_rent_ack.rent), &rf);

                        printf("**************rent flag(bit0~bit3): %d,%d,%d,%d************\n",rf.b0,rf.b1,rf.b2,rf.b3);

                        /* 租还车标志 */
                        __rent_flag_to_bit(&(lnt_rent_ack.rent), &rf);

						if((rf.b0 == 0) && (rf.b1 == 0) && (rf.b2 == 0) && (rf.b3 == 1))
						{
							label_rentFlag->setText(tr("普通租车"));
						}
						else if((rf.b0 == 0) && (rf.b1 == 0) && (rf.b2 == 1) && (rf.b3 == 1))
						{
							label_rentFlag->setText(tr("租车高峰期租车"));
						}
						else if((rf.b0 == 0) && (rf.b1 == 1) && (rf.b2 == 0) && (rf.b3 == 1))
						{
							label_rentFlag->setText(tr("还车高峰期租车"));
						}
						else if((rf.b0 == 0) && (rf.b1 == 0) && (rf.b2 == 1) && (rf.b3 == 0))
						{
							label_rentFlag->setText(tr("还车高峰期还车"));
							retval = LNT_HASNO_RENT_BICYCLE;
						}
						else if((rf.b0 == 0) && (rf.b1 == 1) && (rf.b2 == 0) && (rf.b3 == 0))
						{
							label_rentFlag->setText(tr("租车高峰期还车"));
							retval = LNT_HASNO_RENT_BICYCLE;
						}
						else if((rf.b0 == 0) && (rf.b1 == 0) && (rf.b2 == 0) && (rf.b3 == 0))
						{
							label_rentFlag->setText(tr("普通还车"));
							retval = LNT_HASNO_RENT_BICYCLE;
						}
						else if(rf.b0 == 1)
                        {
                                label_rentFlag->setText(tr("延迟还车"));
								retval = LNT_DELAYED_BICYCLE; //add by zjc at 2016-10-28
                        }
						else
						{
							label_rentFlag->setText(tr("租还车标志异常"));
							retval = LNT_MOC_ERROR_UNKNOW; 
						}
						
					#if 0
                        if((rf.b0 == 0) && (rf.b1 == 0) && (rf.b2 == 0) && (rf.b3 == 0))
                        {
                                label_rentFlag->setText(tr("已还车"));

                                retval = LNT_HASNO_RENT_BICYCLE;//
                        }
                        else if((rf.b0 == 1) && (rf.b1 == 0) && (rf.b2 == 0) && (rf.b3 == 0))
                        {
                                label_rentFlag->setText(tr("普通已租车"));
                        }
                        else if((rf.b0 == 1) && (rf.b1 == 1) && (rf.b2 == 0) && (rf.b3 == 0))
                        {
                                label_rentFlag->setText(tr("高峰已租车"));
                        }
                        else if(rf.b1 == 1)
                        {
                                label_rentFlag->setText(tr("延迟还车"));
								retval = LNT_DELAYED_BICYCLE; //add by zjc at 2016-10-28
                        }
					#endif

			/* 票种标志 */
			string.clear();
			switch(lnt_rent_ack.rent.ticket_flag)
			{
				case TICKET_FLAG_ORDINARG:
					label_ticketType->setText(tr("普通卡"));
					string.sprintf("%d", LNT_NTOHS(lnt_rent_ack.rent.s_un.acc.accum));
					break;

				case TICKET_FLAG_TIME:
					label_ticketType->setText(tr("次卡"));
					string.sprintf("%d(次)", LNT_NTOHS(lnt_rent_ack.rent.s_un.acc.accum));
					break;

				case TICKET_FLAG_DAY:
					label_ticketType->setText(tr("天卡"));
					string.sprintf("%d(天)", LNT_NTOHS(lnt_rent_ack.rent.s_un.acc.accum));
					break;

				case TICKET_FLAG_MON:
					label_ticketType->setText(tr("月卡"));
					string.sprintf("%d(月)", LNT_NTOHS(lnt_rent_ack.rent.s_un.acc.accum));
					break;
					
				case TICKET_FLAG_YEAR:
					label_ticketType->setText(tr("年卡"));
					string.sprintf("%d(年)", LNT_NTOHS(lnt_rent_ack.rent.s_un.acc.accum));
					break;
					
				case TICKET_FLAG_SPECIAL:
					label_ticketType->setText(tr("专车卡"));
					string.sprintf("%d", LNT_NTOHS(lnt_rent_ack.rent.s_un.acc.accum));
					break;		
					
				case TICKET_FLAG_OLD:
					label_ticketType->setText(tr("原客户卡"));
					string.sprintf("%d(次)", LNT_NTOHS(lnt_rent_ack.rent.s_un.acc.accum));
					break;	

				case TICKET_FLAG_ADMIN:
					label_ticketType->setText(tr("管理卡"));
					string.sprintf("%d", LNT_NTOHS(lnt_rent_ack.rent.s_un.acc.accum));
					break;	
			}


			if(lnt_rent_ack.rent.ticket_flag == TICKET_FLAG_ADMIN)	//如果是管理卡
			{
				switch(adminCardType)
				{
					case LNT_ADMIN_CARD_TYPE_RD:
					{
						label_ticketType->setText(tr("普通管理卡"));
					}
					break;

					case LNT_ADMIN_CARD_TYPE_RW:
					{
						label_ticketType->setText(tr("超级管理卡"));
					}
					break;

					default:
					break;
				}
			}
			

			/* 免费时长 */
			int free_time = lnt_rent_ack.rent.free_time * 15;
			string.clear();
			string.sprintf("%d分钟", free_time);
			label_freeTime->setText(string);


			/* 当次交易日期时间 */
			unsigned int cur_deal_date = 0;
			memcpy(&cur_deal_date, lnt_rent_ack.rent.cur_deal_date, 4);
			if(cur_deal_date != 0)
			{
				string.sprintf("20%02d-%02d-%02d %02d:%02d", lib_hex_to_bcd(lnt_rent_ack.rent.cur_deal_date[0]),
				lib_hex_to_bcd(lnt_rent_ack.rent.cur_deal_date[1]),  lib_hex_to_bcd(lnt_rent_ack.rent.cur_deal_date[2]),
				lib_hex_to_bcd(lnt_rent_ack.rent.cur_deal_date[3]), lib_hex_to_bcd(lnt_rent_ack.rent.cur_deal_date[4]));
				label_curDealDate->setText(string);
			}
			else
			{
				label_curDealDate->setText(tr("0000-00-00 00:00"));
			}		
						//|| (retval == LNT_DELAYED_BICYCLE) 已经延时还车的无需进行延时还车操作 add by zjc at 2016-10-28
                        if((retval == LNT_HASNO_RENT_BICYCLE) || (retval == LNT_DELAYED_BICYCLE))    //未租车的无需执行延时还车操作
                        {
                            label_whetherRetrunDelay->setText("不允许");  //允许延时还车与否
                            label_emptySae->setText("未统计");            //空桩数量

                            return retval;
                        }

                        label_whetherRetrunDelay->setText("等待判断");  //允许延时还车与否
                        label_emptySae->setText("正在统计");            //空桩数量

                        /* 请求延迟还车 REQ */
                        lib_gui_delay_return_bicyle_req(g_gui, 300);

                        retval = LNT_CHECK_DELAYED_BICYCLE;
		}
		else
			retval = m_stat;
	}
	else
		retval = m_stat;

	
	return retval;
}

void DelayReturnBicycleDlg::do_delay_return_bicycle()
{
	lib_lnt_config_t config;
	int ret = -1;

    QString memberShow;

	if(delay_flag == UNABLE_DELAY_RETUAN_BICYCLE || delay_flag == DELAY_RETUAN_BICYCLE_ING)
	{
		return;
	}

	delay_flag = DELAY_RETUAN_BICYCLE_ING;

	// 初始化结构体
	memset(&lnt_rent,0,sizeof(lib_lnt_rent_info_t));
	lnt_rent.id = 0x68;
	lnt_rent.length = sizeof(lnt_rent.rent);
	memcpy(&lnt_rent.rent,&lnt_rent_ack.rent,sizeof(rent_info));
	lnt_rent.rent.app_locker = 0x00;
	//lnt_rent.rent.rent_flag = 9;
	lnt_rent.rent.rent_flag = lnt_rent_ack.rent.rent_flag | 0x01; //最高两位原样写入,最低位为满桩标志 2016-12-02


	// 设置租还车信息
	m_ret = lib_lnt_set_rent_info(&lnt_rent, &m_stat, 300);
	if((m_ret <= 0) || (m_stat != LNT_MOC_ERROR_NORMAL))
	{
		label_whetherRetrunDelay->setText("延迟还车失败");

        memberShow.sprintf("提示:执行延时还车失败,请退卡, %02d秒后自动返回上一级界面, 按[退出]键直接返回", timerCount);
        label_DelayReturnBicycleShow->setText(memberShow);

		
	#if 1
		//岭南通模块串口初始化
		memset(&config,0,sizeof(lib_lnt_config_t));
		strcpy((char *)config.tty,"/dev/ttyO1");
		config.baudrate = 115200;
	
		ret = lnt_init(&config);
		if(ret < 0){
			fprintf(stderr,"lnt_init failed,ret = %d\n",ret);
		}
	#endif
	
		return;
	}

	
	// 消费
	memset(&lib_lnt_consume_req,0,sizeof(lib_lnt_consume_req));
	lib_lnt_consume_req.amount = 0;
	lib_lnt_consume_req.fee = 0;
	lib_lnt_utils_time_bcd_yymmddhhMMss(lib_lnt_consume_req.time);
	memcpy(lib_lnt_consume_req.phyiscalid, SAVE_pid, 8);
	lib_lnt_consume_req.aty = LNT_ATY_RETURN; //收押金  0x17
	//lib_printf("consume",(unsigned char *)&lib_lnt_consume_req,sizeof(lib_lnt_consume_req));
	m_ret = lib_lnt_consume(&lib_lnt_consume_req, &m_stat, 500);
	if(!(m_ret > 0) || !(m_stat == LNT_MOC_ERROR_NORMAL))
	{
		label_whetherRetrunDelay->setText("延迟还车失败");

        memberShow.sprintf("提示:执行延时还车失败,请退卡, %02d秒后自动返回上一级界面, 按[退出]键直接返回", timerCount);
        label_DelayReturnBicycleShow->setText(memberShow);

		
	#if 1
		//岭南通模块串口初始化
		memset(&config,0,sizeof(lib_lnt_config_t));
		strcpy((char *)config.tty,"/dev/ttyO1");
		config.baudrate = 115200;
	
		ret = lnt_init(&config);
		if(ret < 0){
			fprintf(stderr,"lnt_init failed,ret = %d\n",ret);
		}
	#endif
		
		return;
	}

	// 获取交易信息	
	m_ret = lib_lnt_get_trade_record(&lib_lnt_trade_record_ack, &m_stat, 300);
	if(!(m_ret > 0) || !(m_stat == LNT_MOC_ERROR_NORMAL))
	{
		label_whetherRetrunDelay->setText("延迟还车失败");

                memberShow.sprintf("提示:执行延时还车失败,请退卡, %02d秒后自动返回上一级界面, 按[退出]键直接返回", timerCount);
                label_DelayReturnBicycleShow->setText(memberShow);
		return;
	}

        delay_flag = LNT_DELAYED_BICYCLE_SUCESS;
	label_whetherRetrunDelay->setText("延迟还车成功");
}


void DelayReturnBicycleDlg::keyPressEvent(QKeyEvent *e)
{
	if(e->key() == Qt::Key_Escape)
	{
		emit signals_delayAccept();
	}
}

bool DelayReturnBicycleDlg::eventFilter(QObject *watched, QEvent *event)
{
	if(watched == pushButton_DelayReturnBicycleStart)
	{
		if(event->type()==QEvent::FocusIn) 
		{
			pushButton_DelayReturnBicycleStart->setStyleSheet("background-color: rgb(89, 182, 75)");
		}
		else if (event->type()==QEvent::FocusOut)
		{
			pushButton_DelayReturnBicycleStart->setStyleSheet("background-color: rgb(224, 239, 198)");
		}
	}	

	if(watched == pushButton_Quit)
	{
		if(event->type()==QEvent::FocusIn) 
		{
			pushButton_Quit->setStyleSheet("background-color: rgb(89, 182, 75)");
		}
		else if (event->type()==QEvent::FocusOut)
		{
			pushButton_Quit->setStyleSheet("background-color: rgb(224, 239, 198)");
		}
	}	

	return QWidget::eventFilter(watched,event); 
}

void DelayReturnBicycleDlg::__allWidgetClear(void)
{
	label_DelayReturnBicycleShow->clear();
	
	label_whetherRetrunDelay->clear();
	label_freeTime->clear();
	label_curDealDate->clear();
	label_rentFlag->clear();
	label_ticketType->clear();
	label_emptySae->clear();
}



