#include <QtGui>
#include <QDebug>
#include "nearby_site_info_query_dlg.h"
#include "helper.h"
#include "lib_general.h"
#include "lib_gui.h"
#include "lib_wireless.h"


#define BINFO_REFRESH_PERIOD					(25)
#define BINFO_RETURN_PERIOD						(20)
#define BINFO_RETURN_SUCCESS_PERIOD			(25)  //查找超时
#define BINFO_RETURN_FAIL_PERIOD				(10)
#define BINFO_EHANDLE_PERIOD					(10)


#define ECS_QUIT_ON								1
#define ECS_QUIT_OFF								2


#define T_TAB									0x09
#define N_ENTER									0x0a

enum NEARBY_SITE_INFO_FMT
{
        RIF_SITE_NAME = 64, //网点名称
        RIF_DISTANCE = 16, //距离
        RIF_BIKES = 16, //自行车数
        RIF_STACKS = 16 //总桩数
};

enum RET_ERROR
{
    RET_ERROR_NORMAL = 0,   //正常
    RET_ERROR_CARD_NOTAG = 1,  //没有卡
    RET_ERROR_NOVIP = 2,  //非会员
    RET_ERROR_DP_INIT = 3,	//押金初始化
    RET_ERROR_OP_FAIL = 4, //操作失败
    RET_ERROR_OP_SUCCESS = 5,  //操作成功
    RET_ERROR_OP_LOOP = 6, //轮询
    RET_ERROR_LNT_BAD = 7, //岭南通读卡器坏 i
    RET_ERROR_NETWORK_DISCONNT = 8, //网络不通
    RET_ERROR_LNT_NOT_REG = 9, //读卡器未注册
    RET_ERROR_USED_TIMES_TOO_MUCH = 10,  //使用次数太多
    RET_ERROR_TIME_INTERVAL_TOO_SHORT = 11 //时间简单太短
};

struct riq_condition
{
	unsigned char last_pid[8]; //最后一次使用的物理卡号
	unsigned int used_times; //使用次数
	time_t last_time;  //最后一次使用时间
};

#define RIQ_COND_LAST_TIME		(10)  //分钟




#define RIQ_SN(n)							label_SN##n
#define RIQ_SITENAME(n)							label_siteName##n
#define RIQ_DISTTANCE(n)							label_distance##n
#define RIQ_BIKES(n)							label_bikes##n
#define RIQ_STACKS(n)						label_stacks##n

#define RIQ_CLR_SN(n)							label_SN##n->clear()
#define RIQ_CLR_SITENAME(n)						label_siteName##n->clear()
#define RIQ_CLR_DISTTANCE(n)						label_distance##n->clear()
#define RIQ_CLR_BIKES(n)						label_bikes##n->clear()
#define RIQ_CLR_STACKS(n)                                           label_stacks##n->clear()

#define RIQ_SET_SN(n, text)						label_SN##n->setText(text)		
#define RIQ_SET_SITENAME(n, text)                                         label_siteName##n->setText(text)
#define RIQ_SET_DISTTANCE(n, text)					label_distance##n->setText(text)
#define RIQ_SET_BIKES(n, text)					label_bikes##n->setText(text)
#define RIQ_SET_STACKS(n, text)                                     label_stacks##n->setText(text)


extern lib_gui_t *g_gui;
extern lib_wl_t *g_wl;

#if 0
#define RIQ_COND_IDX_MAX    					2
static int g_riq_cond_idx = 0;
static struct riq_condition g_riq_cond[RIQ_COND_IDX_MAX];
#endif

static struct riq_condition g_riq_cond;


NearbySiteInfoQueryDlg::NearbySiteInfoQueryDlg(QWidget *parent) : QDialog(parent)
{
	setupUi(this);
	setWindowFlags(Qt::FramelessWindowHint);	

        memset(&nearby_site_info_qry_req, 0, sizeof(gui_nearby_site_info_qry_req_t));
        memset(&nearby_site_info_qry_ack, 0, sizeof(gui_nearby_site_info_qry_ack_t));
	
	m_ret = LIB_LNT_ERROR;
	m_stat = 0;
	timerFlag = 0;
	esc_quit_key = ECS_QUIT_ON;  //允许使用ESC退出键
	handleReslut = 0;
	now_time_sec = 0;
	
	__allWidgetClear();
	
	timerCount = BINFO_REFRESH_PERIOD;
	//slots_timerDone();
#if 0
	QString string;
	string.sprintf("提示:请在%02d秒内,把卡放置到指定刷卡区域上", timerCount + 1);
        label_nearbySiteInfoQryShow->setText(string);
#endif
	connect(&timer, SIGNAL(timeout()), this, SLOT(slots_timerDone()));
	timer.start(1000);

        connect(this, SIGNAL(signals_nearbySiteInfoAccept()), this, SLOT(accept()));
        connect(this, SIGNAL(signals_nearbySiteInfoReject()), this, SLOT(reject()));
}


NearbySiteInfoQueryDlg::~NearbySiteInfoQueryDlg(void)
{
	
}

void NearbySiteInfoQueryDlg::__showConfigResult(const unsigned char result)
{
        FormHelper::__FormSetFont_wqy(20, QFont::Normal, label_nearbySiteInfoQryShow);
	
	switch(result)
	{
		case DEV_R_CONFIG_RESULT_OK:
		{
                        label_nearbySiteInfoQryShow->setText(tr("提示:附近网点信息查询成功"));
		}
		break;

		case DEV_R_CONFIG_RESULT_ERROR:
		{
                        label_nearbySiteInfoQryShow->setText(tr("提示:附近网点信息查询失败"));
		}
		break;
	}	
}

int NearbySiteInfoQueryDlg::__parseNearbySiteInfoQry(gui_nearby_site_info_qry_ack_t *nearby_site_info_ack, gui_nearby_site_info_fmt_t nearby_site_info_fmt[])
{
        if((nearby_site_info_ack == NULL) || (nearby_site_info_fmt == NULL))
			return -1;

		int begin = 0;
		int end = 0;	
		int cnt = 0;
		int size = 0;
		int i = 0;
        int m_CurrInfoNum = 0;
		int info_len = 0;
		
        printf("----info num:%d, size:%d-----\n",nearby_site_info_ack->item, nearby_site_info_ack->length);

        for(i = 0; i < nearby_site_info_ack->length; i++)
        {
            //0x09:字段间的分隔符(tab键), 0x10:记录间的分隔符(换行键)
            if((nearby_site_info_ack->info[i] == 0x09) || (nearby_site_info_ack->info[i] == 0x0a))
            {
                end = i;
                size = end - begin;//一个字段的长度
                switch(cnt)//将各个字段拷到结构体对应的成员中去
                {
                    case 0://网点名称
                        if(size >= 64)
                        {
                                size = 64;
                        }
                        if(m_CurrInfoNum == 0)
                                memcpy(nearby_site_info_fmt[m_CurrInfoNum].siteName, &nearby_site_info_ack->info[begin],size);
                        else
                                memcpy(nearby_site_info_fmt[m_CurrInfoNum].siteName, &nearby_site_info_ack->info[begin+1],size);

                        nearby_site_info_fmt[m_CurrInfoNum].siteName[size] = '\0';
                        break;

                    case 1://距离
                        if(size >= 16)
                        {
                                size = 16;
                        }
                        memcpy(nearby_site_info_fmt[m_CurrInfoNum].distance,&nearby_site_info_ack->info[begin+1],size);
                        nearby_site_info_fmt[m_CurrInfoNum].distance[size] = '\0';
                        break;

                    case 2://自行车数
                        if(size >= 16)
                        {
                                size = 16;
                        }
                        memcpy(nearby_site_info_fmt[m_CurrInfoNum].bikes, &nearby_site_info_ack->info[begin+1],size);
                        nearby_site_info_fmt[m_CurrInfoNum].bikes[size] = '\0';
                        break;

                    case 3://总桩数
                        if(size >= 16)
                        {
                                size = 16;
                        }
                        memcpy(nearby_site_info_fmt[m_CurrInfoNum].stacks, &nearby_site_info_ack->info[begin+1],size);
                        nearby_site_info_fmt[m_CurrInfoNum].stacks[size] = '\0';
                        break;

                    default:
                            cnt = 0;
                    break;
                }

                begin = end;
                cnt++;//同一条记录里的第几个字段
            }

            if(nearby_site_info_ack->info[i] == 0x0a)
            {
                    #if 1
                    //打印显示每一条附近网点信息
                    printf("\n---------RecordNum:[%d]---------\n",m_CurrInfoNum);
                    printf("siteName:%s\n",nearby_site_info_fmt[m_CurrInfoNum].siteName);
                    printf("distance:%s\n",nearby_site_info_fmt[m_CurrInfoNum].distance);
                    printf("bikes:%s\n",nearby_site_info_fmt[m_CurrInfoNum].bikes);
                    printf("stacks:%s\n",nearby_site_info_fmt[m_CurrInfoNum].stacks);
                    #endif

                    m_CurrInfoNum++;//第几条记录
                    cnt = 0;
            }
			
            if(m_CurrInfoNum >= nearby_site_info_ack->item)
                    return m_CurrInfoNum;
	}

    return m_CurrInfoNum;
}

int NearbySiteInfoQueryDlg::__explainNearbySiteInfoAtoi(char *dest, char *src)
{
	if((dest == NULL) || (src == NULL))
		return -1;
		
        int dist = atoi(src);


        sprintf(dest, "%d", dist);

	return 0;
}

int NearbySiteInfoQueryDlg::__setNearbySiteInfo(void)
{
	int ret = -1;
	char s_str[32] = {0};
	QTextCodec *codec = QTextCodec::codecForLocale();
    QString s_gbk;

	printf("------------site info:\n%s\n", nearby_site_info_qry_ack.info);
	lib_printf("------------siteNo", nearby_site_info_qry_ack.siteNo, 2);
	printf("------------item:%d\n", nearby_site_info_qry_ack.item);
	printf("------------length:%d\n", nearby_site_info_qry_ack.length);
	//printf("------------strlen:%d\n",strlen((const char *)nearby_site_info_qry_ack.info));
	
    ret = __parseNearbySiteInfoQry(&nearby_site_info_qry_ack, nearby_site_info_fmt);
	if(ret > 0)
	{
		/*****1******/
		//RIQ_SET_SN(1, "1");
        s_gbk = codec->toUnicode((char *)nearby_site_info_fmt[0].siteName);  //显示中文,GBK
        RIQ_SET_SITENAME(1, s_gbk);
				
		s_gbk.clear();
        memset(s_str, 0, 32);
        __explainNearbySiteInfoAtoi(s_str, (char *)nearby_site_info_fmt[0].distance);
        s_gbk = codec->toUnicode(s_str);
        RIQ_SET_DISTTANCE(1, s_gbk);

		s_gbk.clear();
		memset(s_str, 0, 32);
        __explainNearbySiteInfoAtoi(s_str, (char *)nearby_site_info_fmt[0].bikes);
		s_gbk = codec->toUnicode(s_str); 
                RIQ_SET_BIKES(1, s_gbk);

		s_gbk.clear();
        memset(s_str, 0, 32);
        __explainNearbySiteInfoAtoi(s_str, (char *)nearby_site_info_fmt[0].stacks);
        s_gbk = codec->toUnicode(s_str);
        RIQ_SET_STACKS(1, s_gbk);

		if(ret <= 1)
			goto Done;

		/*****2******/
		//RIQ_SET_SN(2, "2");
        s_gbk = codec->toUnicode((char *)nearby_site_info_fmt[1].siteName);  //显示中文,GBK
        RIQ_SET_SITENAME(2, s_gbk);
				
		s_gbk.clear();
        memset(s_str, 0, 32);
        __explainNearbySiteInfoAtoi(s_str, (char *)nearby_site_info_fmt[1].distance);
        s_gbk = codec->toUnicode(s_str);
        RIQ_SET_DISTTANCE(2, s_gbk);

		s_gbk.clear();
		memset(s_str, 0, 32);
        __explainNearbySiteInfoAtoi(s_str, (char *)nearby_site_info_fmt[1].bikes);
		s_gbk = codec->toUnicode(s_str); 
                RIQ_SET_BIKES(2, s_gbk);

		s_gbk.clear();
        memset(s_str, 0, 32);
        __explainNearbySiteInfoAtoi(s_str, (char *)nearby_site_info_fmt[1].stacks);
        s_gbk = codec->toUnicode(s_str);
        RIQ_SET_STACKS(2, s_gbk);

		if(ret <= 2)
			goto Done;

		/*****3******/
		//RIQ_SET_SN(3, "3");
        s_gbk = codec->toUnicode((char *)nearby_site_info_fmt[2].siteName);  //显示中文,GBK
        RIQ_SET_SITENAME(3, s_gbk);
				
		s_gbk.clear();
        memset(s_str, 0, 32);
        __explainNearbySiteInfoAtoi(s_str, (char *)nearby_site_info_fmt[2].distance);
        s_gbk = codec->toUnicode(s_str);
        RIQ_SET_DISTTANCE(3, s_gbk);

		s_gbk.clear();
		memset(s_str, 0, 32);
        __explainNearbySiteInfoAtoi(s_str, (char *)nearby_site_info_fmt[2].bikes);
		s_gbk = codec->toUnicode(s_str); 
                RIQ_SET_BIKES(3, s_gbk);

		s_gbk.clear();
        memset(s_str, 0, 32);
        __explainNearbySiteInfoAtoi(s_str, (char *)nearby_site_info_fmt[2].stacks);
        s_gbk = codec->toUnicode(s_str);
        RIQ_SET_STACKS(3, s_gbk);

		if(ret <= 3)
			goto Done;

		/*****4******/
		//RIQ_SET_SN(4, "4");
        s_gbk = codec->toUnicode((char *)nearby_site_info_fmt[3].siteName);  //显示中文,GBK
        RIQ_SET_SITENAME(4, s_gbk);
				
		s_gbk.clear();
        memset(s_str, 0, 32);
        __explainNearbySiteInfoAtoi(s_str, (char *)nearby_site_info_fmt[3].distance);
        s_gbk = codec->toUnicode(s_str);
        RIQ_SET_DISTTANCE(4, s_gbk);

		s_gbk.clear();
		memset(s_str, 0, 32);
        __explainNearbySiteInfoAtoi(s_str, (char *)nearby_site_info_fmt[3].bikes);
		s_gbk = codec->toUnicode(s_str); 
                RIQ_SET_BIKES(4, s_gbk);

		s_gbk.clear();
        memset(s_str, 0, 32);
        __explainNearbySiteInfoAtoi(s_str, (char *)nearby_site_info_fmt[3].stacks);
        s_gbk = codec->toUnicode(s_str);
        RIQ_SET_STACKS(4, s_gbk);

		if(ret <= 4)
			goto Done;	

		/*****5******/
		//RIQ_SET_SN(5, "5");
        s_gbk = codec->toUnicode((char *)nearby_site_info_fmt[4].siteName);  //显示中文,GBK
        RIQ_SET_SITENAME(5, s_gbk);
				
		s_gbk.clear();
        memset(s_str, 0, 32);
        __explainNearbySiteInfoAtoi(s_str, (char *)nearby_site_info_fmt[4].distance);
        s_gbk = codec->toUnicode(s_str);
        RIQ_SET_DISTTANCE(5, s_gbk);

		s_gbk.clear();
		memset(s_str, 0, 32);
        __explainNearbySiteInfoAtoi(s_str, (char *)nearby_site_info_fmt[4].bikes);
		s_gbk = codec->toUnicode(s_str); 
                RIQ_SET_BIKES(5, s_gbk);

		s_gbk.clear();
        memset(s_str, 0, 32);
        __explainNearbySiteInfoAtoi(s_str, (char *)nearby_site_info_fmt[4].stacks);
        s_gbk = codec->toUnicode(s_str);
        RIQ_SET_STACKS(5, s_gbk);

		if(ret <= 5)
			goto Done;	

		/*****6******/
		//RIQ_SET_SN(6, "6");
        s_gbk = codec->toUnicode((char *)nearby_site_info_fmt[5].siteName);  //显示中文,GBK
        RIQ_SET_SITENAME(6, s_gbk);
				
		s_gbk.clear();
        memset(s_str, 0, 32);
        __explainNearbySiteInfoAtoi(s_str, (char *)nearby_site_info_fmt[5].distance);
        s_gbk = codec->toUnicode(s_str);
        RIQ_SET_DISTTANCE(6, s_gbk);

		s_gbk.clear();
		memset(s_str, 0, 32);
        __explainNearbySiteInfoAtoi(s_str, (char *)nearby_site_info_fmt[5].bikes);
		s_gbk = codec->toUnicode(s_str); 
                RIQ_SET_BIKES(6, s_gbk);

		s_gbk.clear();
        memset(s_str, 0, 32);
        __explainNearbySiteInfoAtoi(s_str, (char *)nearby_site_info_fmt[5].stacks);
        s_gbk = codec->toUnicode(s_str);
        RIQ_SET_STACKS(6, s_gbk);
		if(ret <= 6)
			goto Done;	
	}
Done:
	return ret;
}

void NearbySiteInfoQueryDlg::slots_timerDone(void)
{	
	static int ret = RET_ERROR_OP_FAIL;
	int err = LIB_GUI_ERROR;
	int flag1 = 0;
	m_stat = RET_ERROR_OP_FAIL;
	QString stringShow;
	gui_access_state_t state;
		
	if(timerFlag == 0) //检查网络是否正常
	{
        if(lib_gui_access_state(g_gui, &state, 500) == LIB_GUI_EOK)  //检查是否连接中心成功
        {
                if(state.connect_stat != 3)  //NDEV_NETSTAT_SESSION
                {
                        timerCount = BINFO_RETURN_FAIL_PERIOD;
                        ret = RET_ERROR_NETWORK_DISCONNT;  //网络异常
                        flag1 = 2;
                        timerFlag = 2;
                }
                else
                {
                		ret = RET_ERROR_NORMAL;
                        flag1 = 0;
                        timerFlag = 1;
                }
        }
	}

	//if(timerFlag == 1)
	{
		switch(ret)
		{
			case RET_ERROR_NETWORK_DISCONNT: //网络异常
			{
				esc_quit_key = ECS_QUIT_ON;
                stringShow.sprintf("提示:网络异常,不能执行附近网点信息查询操作, %02d秒后自动返回上一级界面, 按[退出]键直接返回", timerCount);
			}
			break;

            case RET_ERROR_NORMAL:
			{
                /* 开始查询附近网点信息 */
				timerCount = BINFO_EHANDLE_PERIOD;

				memset(&bPageConfig, 0, sizeof(struct gui_basic_info_page_config));
				err = lib_gui_get_basic_page_config(g_gui, &bPageConfig, 500); 
	
				memset(&nearby_site_info_qry_req, 0, sizeof(nearby_site_info_qry_req));
				nearby_site_info_qry_req.siteNo = LNT_HTONS(bPageConfig.terminal_no);
				//nearby_site_info_qry_req.siteNo = LNT_HTONS(0x2780); //test
				
                err = lib_gui_nearby_site_info_qry_req(g_gui, &nearby_site_info_qry_req, 500);
				printf("----------lib_gui_nearby_site_info_qry_req, err:%d\n", err);
 				if(err == LIB_GUI_EOK)
				{
					ret = RET_ERROR_OP_LOOP;
					timerCount = BINFO_RETURN_SUCCESS_PERIOD;
					timerFlag = 2;
				}
				else //执行失败
				{
					ret = RET_ERROR_OP_FAIL;
					timerCount = BINFO_RETURN_FAIL_PERIOD;
					timerFlag = 2;
				}	
			}
			break;

			case RET_ERROR_OP_FAIL:
			{
				esc_quit_key = ECS_QUIT_ON;
                stringShow.sprintf("提示:附近网点信息查询失败, %02d秒后自动返回上一级界面, 按[退出]键直接返回", timerCount);
			}
			break;

            case RET_ERROR_OP_LOOP:
			{
                /* 附近网点信息查询流程 */
				//esc_quit_key = ECS_QUIT_OFF;

                stringShow.sprintf("提示:附近网点信息查询进行中,请耐心等待%02d秒,并仔细留意屏幕上的提示", timerCount);
	
                memset(&nearby_site_info_qry_ack, 0, sizeof(gui_nearby_site_info_qry_ack_t));
                err = lib_gui_nearby_site_info_qry_ack(g_gui, &nearby_site_info_qry_ack, 900);
				//printf("============%s:\n", nearby_site_info_qry_ack.info);
				printf("-------------lib_gui_nearby_site_info_qry_ack, err:%d\n", err);
				if(err == LIB_GUI_EFULL)  //有数据
				{
					if(timer.isActive())
					{
						timer.stop();
					}

                    if(nearby_site_info_qry_ack.item == 0) //找不到附近网点信息
					{
						timerCount = 10; //10秒
						handleReslut = 4;
					}
					else
					{
						printf("---------------items:%d\n", nearby_site_info_qry_ack.item);
						if(nearby_site_info_qry_ack.item > 6)
							nearby_site_info_qry_ack.item = 6; //最多显示6条
						
						timerCount = 30; //30秒
						handleReslut = 3;
                        __setNearbySiteInfo(); //显示附近网点信息
					}
	
					ret = RET_ERROR_OP_SUCCESS;
					timer.start(1000);
					timerFlag = 2;
					esc_quit_key = ECS_QUIT_ON;
					
				}
				else if((err == LIB_GUI_EEMPTY) && (timerCount == 1)) //超时还没有收到数据
				{
                    ret = RET_ERROR_OP_FAIL; //查询失败
					timerCount = BINFO_RETURN_FAIL_PERIOD;
					timerFlag = 2;
				}
				else if(err != LIB_GUI_EEMPTY)//读取数据失败
				{
                    ret = RET_ERROR_OP_FAIL; //查询失败
					timerCount = BINFO_RETURN_FAIL_PERIOD;
					timerFlag = 2;
				}
			}

            case RET_ERROR_OP_SUCCESS: //收到附近网点信息应答
			{
				if(handleReslut == 3)
                    stringShow.sprintf("提示:一共查询到%d个附近网点信息, %02d秒后自动返回上一级界面, 按[退出]键直接返回", nearby_site_info_qry_ack.item, timerCount);

				if(handleReslut == 4)
                    stringShow.sprintf("提示:找不到附近网点信息, %02d秒后自动返回上一级界面, 按[退出]键直接返回", timerCount);
			}
			break;
		}

        label_nearbySiteInfoQryShow->setText(stringShow);
		
		if(timerCount == 0)
		{
			timerFlag = 3;
            emit signals_nearbySiteInfoReject();
		}
	}
	
	timerCount--;
}

void NearbySiteInfoQueryDlg::__allWidgetClear(void)
{
        label_nearbySiteInfoQryShow->clear();

	//RIQ_SN(1)->clear();
        RIQ_SITENAME(1)->clear();
        RIQ_DISTTANCE(1)->clear();
        RIQ_BIKES(1)->clear();
        RIQ_STACKS(1)->clear();

	//RIQ_SN(2)->clear();
        RIQ_SITENAME(2)->clear();
        RIQ_DISTTANCE(2)->clear();
        RIQ_BIKES(2)->clear();
        RIQ_STACKS(2)->clear();

	//RIQ_SN(3)->clear();
        RIQ_SITENAME(3)->clear();
        RIQ_DISTTANCE(3)->clear();
        RIQ_BIKES(3)->clear();
        RIQ_STACKS(3)->clear();

	//RIQ_SN(4)->clear();
        RIQ_SITENAME(4)->clear();
        RIQ_DISTTANCE(4)->clear();
        RIQ_BIKES(4)->clear();
        RIQ_STACKS(4)->clear();

	//RIQ_SN(5)->clear();
        RIQ_SITENAME(5)->clear();
        RIQ_DISTTANCE(5)->clear();
        RIQ_BIKES(5)->clear();
        RIQ_STACKS(5)->clear();

	//RIQ_SN(6)->clear();
        RIQ_SITENAME(6)->clear();
        RIQ_DISTTANCE(6)->clear();
        RIQ_BIKES(6)->clear();
        RIQ_STACKS(6)->clear();
} 

void NearbySiteInfoQueryDlg::keyPressEvent(QKeyEvent *e)
{
	if(e->key() == Qt::Key_Escape)  
	{
		if(esc_quit_key == ECS_QUIT_ON)
           emit signals_nearbySiteInfoAccept();
	}
}






