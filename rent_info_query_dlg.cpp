#include <QtGui>
#include <QDebug>
#include "rent_info_query_dlg.h"
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

enum RENT_INFO_FMT
{
	RIF_BIKE_NO = 32,
	RIF_RENT_NAME = 64,
	RIF_RENT_TIME = 32,
	RIF_RETURN_NAME = 64,
	RIF_RETURN_TIME = 32,
	RIF_USED_TIME = 16,
	RIF_FEE = 16
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
	RET_ERROR_TIME_INTERVAL_TOO_SHORT = 11, //时间简单太短
};

struct riq_condition
{
	unsigned char last_pid[8]; //最后一次使用的物理卡号
	unsigned int used_times; //使用次数
	time_t last_time;  //最后一次使用时间
};

#define RIQ_COND_LAST_TIME		(10)  //分钟




#define RIQ_SN(n)									label_SN##n
#define RIQ_BIKENO(n)								label_bikeNO##n
#define RIQ_RENTNAME(n)							label_rentName##n
#define RIQ_RENTTIME(n)							label_rentTime##n
#define RIQ_RETURNNAME(n)						label_returnName##n
#define RIQ_RETURNTIME(n)						label_returnTime##n
#define RIQ_USEDTIME(n)							label_usedTime##n
#define RIQ_FEE(n)								label_fee##n

#define RIQ_CLR_SN(n)							label_SN##n->clear()
#define RIQ_CLR_BIKENO(n)						label_bikeNO##n->clear()
#define RIQ_CLR_RENTNAME(n)						label_rentName##n->clear()
#define RIQ_CLR_RENTTIME(n)						label_rentTime##n->clear()
#define RIQ_CLR_RETURNNAME(n)					label_returnName##n->clear()
#define RIQ_CLR_RETURNTIME(n)					label_returnTime##n->clear()
#define RIQ_CLR_USEDTIME(n)						label_usedTime##n->clear()
#define RIQ_CLR_FEE(n)							label_fee##n->clear()

#if 0
#define RIQ_SET_SN(n, text)							label_SN##n->setText(tr(text))		
#define RIQ_SET_BIKENO(n, text)						label_bikeNO##n->setText(tr(text))
#define RIQ_SET_RENTNAME(n, text)					label_rentName##n->setText(tr(text))
#define RIQ_SET_RENTTIME(n, text)					label_rentTime##n->setText(tr(text))
#define RIQ_SET_RETURNNAME(n, text)					label_returnName##n->setText(tr(text))
#define RIQ_SET_RETURNTIME(n, text)					label_returnTime##n->setText(tr(text))
#define RIQ_SET_USEDTIME(n, text)					label_usedTime##n->setText(tr(text))
#define RIQ_SET_FEE(n, text)							label_fee##n->setText(tr(text))
#endif

#define RIQ_SET_SN(n, text)						label_SN##n->setText(text)		
#define RIQ_SET_BIKENO(n, text)					label_bikeNO##n->setText(text)
#define RIQ_SET_RENTNAME(n, text)					label_rentName##n->setText(text)
#define RIQ_SET_RENTTIME(n, text)					label_rentTime##n->setText(text)
#define RIQ_SET_RETURNNAME(n, text)				label_returnName##n->setText(text)
#define RIQ_SET_RETURNTIME(n, text)				label_returnTime##n->setText(text)
#define RIQ_SET_USEDTIME(n, text)					label_usedTime##n->setText(text)
#define RIQ_SET_FEE(n, text)						label_fee##n->setText(text)


extern lib_gui_t *g_gui;
extern lib_wl_t *g_wl;

#if 0
#define RIQ_COND_IDX_MAX    					2
static int g_riq_cond_idx = 0;
static struct riq_condition g_riq_cond[RIQ_COND_IDX_MAX];
#endif

static struct riq_condition g_riq_cond;


RentInfoQueryDlg::RentInfoQueryDlg(QWidget *parent) : QDialog(parent)
{
	setupUi(this);
	setWindowFlags(Qt::FramelessWindowHint);	

	memset(&rinfo_qry_req, 0, sizeof(gui_rent_info_qry_req_t));
	memset(&rinfo_qry_ack, 0, sizeof(gui_rent_info_qry_ack_t));
	
	m_ret = LIB_LNT_ERROR;
	m_stat = 0;
	timerFlag = 0;
	esc_quit_key = ECS_QUIT_ON;  //允许使用ESC退出键
	handleReslut = 0;
	now_time_sec = 0;
	
	__allWidgetClear();
	
	timerCount = BINFO_REFRESH_PERIOD;
	//slots_timerDone();

	QString string;
	string.sprintf("提示:请在%02d秒内,把卡放置到指定刷卡区域上", timerCount + 1);
	label_rentInfoQryShow->setText(string);	
	
	connect(&timer, SIGNAL(timeout()), this, SLOT(slots_timerDone()));
	timer.start(1000);

	connect(this, SIGNAL(signals_rentInfoAccept()), this, SLOT(accept()));
	connect(this, SIGNAL(signals_rentInfoReject()), this, SLOT(reject()));
}


RentInfoQueryDlg::~RentInfoQueryDlg(void)
{
	
}

void RentInfoQueryDlg::__showConfigResult(const unsigned char result)
{
	FormHelper::__FormSetFont_wqy(20, QFont::Normal, label_rentInfoQryShow);
	
	switch(result)
	{
		case DEV_R_CONFIG_RESULT_OK:
		{
			label_rentInfoQryShow->setText(tr("提示:租还车信息查询成功"));	
		}
		break;

		case DEV_R_CONFIG_RESULT_ERROR:
		{
			label_rentInfoQryShow->setText(tr("提示:租还车信息查询失败"));	
		}
		break;
	}	
}

int RentInfoQueryDlg::__parseRentInfoQry(gui_rent_info_qry_ack_t *rent_info_qry, gui_rent_info_fmt_t rent_info_fmt[RENT_INFO_FMT_MAX])
{
	if((rent_info_qry == NULL) || (rent_info_fmt == NULL))
		return -1;

	int begin = 0;
	int end = 0;	
	int cnt = 0;
	int size = 0;
	int i = 0;
	int m_CurrRecordNum = 0;

	printf("----------rinfo_qry_ack:\n%s\n", rent_info_qry->record);
	printf("----record num:%d, size:%d-----\n",rent_info_qry->item,rent_info_qry->length);

	for(i = 0; i < rent_info_qry->length; i++)   
	{	
		//0x09:字段间的分隔符(tab键), 0x10:记录间的分隔符(换行键)
		if((rent_info_qry->record[i] == 0x09) || (rent_info_qry->record[i] == 0x0a))
		{
			end = i;
			size = end - begin;//一个字段的长度
			switch(cnt)//将各个字段拷到结构体对应的成员中去
			{
				case 0://自行车编号
					if(size >= 32)
					{
						size = 32;
					}
					if(m_CurrRecordNum == 0)
						memcpy(rent_info_fmt[m_CurrRecordNum].bike_NO, &rent_info_qry->record[begin],size);
					else
						memcpy(rent_info_fmt[m_CurrRecordNum].bike_NO, &rent_info_qry->record[begin+1],size);

					rent_info_fmt[m_CurrRecordNum].bike_NO[size] = '\0';
					break;
					
				case 1://租车点名称
					if(size >= 64)
					{
						size = 64;
					}
					memcpy(rent_info_fmt[m_CurrRecordNum].rent_name,&rent_info_qry->record[begin+1],size);
					rent_info_fmt[m_CurrRecordNum].rent_name[size] = '\0';
					break;
					
				case 2://租车点时间
					if(size >= 32)
					{
						size = 32;
					}
					memcpy(rent_info_fmt[m_CurrRecordNum].rent_time, &rent_info_qry->record[begin+1],size);
					rent_info_fmt[m_CurrRecordNum].rent_time[size] = '\0';
					break;

				case 3://还车点名称
					if(size >= 64)
					{
						size = 64;
					}
					memcpy(rent_info_fmt[m_CurrRecordNum].return_name, &rent_info_qry->record[begin+1],size);
					rent_info_fmt[m_CurrRecordNum].return_name[size] = '\0';					
					break;
				case 4://还车时间
					if(size >= 32)
					{
						size = 32;
					}
					memcpy(rent_info_fmt[m_CurrRecordNum].return_time, &rent_info_qry->record[begin+1],size);
					rent_info_fmt[m_CurrRecordNum].return_time[size] = '\0';
					break;
				case 5://骑行时间
					if(size >= 16)
					{
						size = 16;
					}
					memcpy(&rent_info_fmt[m_CurrRecordNum].used_time, &rent_info_qry->record[begin+1],size);
					rent_info_fmt[m_CurrRecordNum].used_time[size] = '\0';
					break;
				case 6://扣费
					if(size >= 16)
					{
						size = 16;
					}
					memcpy(rent_info_fmt[m_CurrRecordNum].fee, &rent_info_qry->record[begin+1],size);
					rent_info_fmt[m_CurrRecordNum].fee[size] = '\0';
					break;
				default:
					cnt = 0;
				break;	
			}	

			begin = end;
			cnt++;//同一条记录里的第几个字段
		}

		if(rent_info_qry->record[i] == 0x0a)
		{
			#if 1
			//打印显示每一条租用记录
            printf("\n---------RecordNum:[%d]---------\n",m_CurrRecordNum);
			printf("bike_NO:%s\n",rent_info_fmt[m_CurrRecordNum].bike_NO);
			printf("rent_name:%s\n",rent_info_fmt[m_CurrRecordNum].rent_name);
			printf("rent_time:%s\n",rent_info_fmt[m_CurrRecordNum].rent_time);
			printf("return_name:%s\n",rent_info_fmt[m_CurrRecordNum].return_name);
			printf("return_time:%s\n",rent_info_fmt[m_CurrRecordNum].return_time);
			printf("used_time:%s\n",rent_info_fmt[m_CurrRecordNum].used_time);
            printf("fee:%s\n\n",rent_info_fmt[m_CurrRecordNum].fee);
			#endif
			
			m_CurrRecordNum++;//第几条记录
			cnt = 0;	
		}

		if(m_CurrRecordNum >= rent_info_qry->item)
			return m_CurrRecordNum;
	}

	return m_CurrRecordNum;
}

int RentInfoQueryDlg::__explainRentInfoTime(char *dest, char *src)
{
	if((dest == NULL) || (src == NULL))
		return -1;

	if(strlen(src) <= 1) //时间为空时不显示 add by zjc at 2016-07-27
		goto exit;
	
	strncat(dest, "20", 2);
	strncat(dest, src, 2);
	
	strncat(dest, "-", 1);
	strncat(dest, &src[2], 2);
	strncat(dest, "-", 1);
	strncat(dest, &src[4], 2);

	strncat(dest, " ", 1);
	strncat(dest, &src[6], 2);
	strncat(dest, ":", 1);
	strncat(dest, &src[8], 2);

exit:
	return 0;
}

int RentInfoQueryDlg::__explainRentInfoFee(char *dest, char *src)
{
	if((dest == NULL) || (src == NULL))
		return -1;
		
	int fee = atoi(src);
	int fen = fee % 100;
	int yuan = fee / 100;

	sprintf(dest, "%d.%02d", yuan, fen);

	return 0;
}

int RentInfoQueryDlg::__setRentInfo(void)
{
	int ret = -1;
	char s_str[32] = {0};
	QTextCodec *codec = QTextCodec::codecForLocale();
	QString s_gbk, s_DataTime;
	QDateTime DataTime;

		
	ret = __parseRentInfoQry(&rinfo_qry_ack, rent_info_fmt);
	if(ret > 0)
	{
		/*****1******/
		RIQ_SET_SN(1, "1");
		s_gbk = codec->toUnicode((char *)rent_info_fmt[0].bike_NO);  //显示中文,GBK
		RIQ_SET_BIKENO(1, s_gbk);	
				
		s_gbk.clear();
		s_gbk = codec->toUnicode((char *)rent_info_fmt[0].rent_name); 
		RIQ_SET_RENTNAME(1, s_gbk);

		s_gbk.clear();
		memset(s_str, 0, 32);
		__explainRentInfoTime(s_str, (char *)rent_info_fmt[0].rent_time);
		s_gbk = codec->toUnicode(s_str); 
		RIQ_SET_RENTTIME(1, s_gbk);

		s_gbk.clear();
		s_gbk = codec->toUnicode((char *)rent_info_fmt[0].return_name); 
		RIQ_SET_RETURNNAME(1, s_gbk);

		s_gbk.clear();
		memset(s_str, 0, 32);
		__explainRentInfoTime(s_str, (char *)rent_info_fmt[0].return_time);
		s_gbk = codec->toUnicode(s_str); 
		RIQ_SET_RETURNTIME(1, s_gbk);

		s_gbk.clear();
		s_gbk = codec->toUnicode((char *)rent_info_fmt[0].used_time); 
		RIQ_SET_USEDTIME(1, s_gbk);

		s_gbk.clear();
		memset(s_str, 0, 32);
		__explainRentInfoFee(s_str, (char *)rent_info_fmt[0].fee);
		s_gbk = codec->toUnicode(s_str); 
		RIQ_SET_FEE(1, s_gbk);

		if(ret <= 1)
			goto Done;

		/*****2******/
		RIQ_SET_SN(2, "2");
		s_gbk = codec->toUnicode((char *)rent_info_fmt[1].bike_NO);  
		RIQ_SET_BIKENO(2, s_gbk);	
				
		s_gbk.clear();
		s_gbk = codec->toUnicode((char *)rent_info_fmt[1].rent_name); 
		RIQ_SET_RENTNAME(2, s_gbk);

		s_gbk.clear();
		memset(s_str, 0, 32);
		__explainRentInfoTime(s_str, (char *)rent_info_fmt[1].rent_time);
		s_gbk = codec->toUnicode(s_str); 
		RIQ_SET_RENTTIME(2, s_gbk);

		s_gbk.clear();
		s_gbk = codec->toUnicode((char *)rent_info_fmt[1].return_name); 
		RIQ_SET_RETURNNAME(2, s_gbk);

		s_gbk.clear();
		memset(s_str, 0, 32);
		__explainRentInfoTime(s_str, (char *)rent_info_fmt[1].return_time);
		s_gbk = codec->toUnicode(s_str); 
		RIQ_SET_RETURNTIME(2, s_gbk);

		s_gbk.clear();
		s_gbk = codec->toUnicode((char *)rent_info_fmt[1].used_time); 
		RIQ_SET_USEDTIME(2, s_gbk);

		s_gbk.clear();
		memset(s_str, 0, 32);
		__explainRentInfoFee(s_str, (char *)rent_info_fmt[1].fee);
		s_gbk = codec->toUnicode(s_str); 
		RIQ_SET_FEE(2, s_gbk);

		if(ret <= 2)
			goto Done;

		/*****3******/
		RIQ_SET_SN(3, "3");
		s_gbk = codec->toUnicode((char *)rent_info_fmt[2].bike_NO);  
		RIQ_SET_BIKENO(3, s_gbk);	
				
		s_gbk.clear();
		s_gbk = codec->toUnicode((char *)rent_info_fmt[2].rent_name); 
		RIQ_SET_RENTNAME(3, s_gbk);

		s_gbk.clear();
		memset(s_str, 0, 32);
		__explainRentInfoTime(s_str, (char *)rent_info_fmt[2].rent_time);
		s_gbk = codec->toUnicode(s_str); 
		RIQ_SET_RENTTIME(3, s_gbk);

		s_gbk.clear();
		s_gbk = codec->toUnicode((char *)rent_info_fmt[2].return_name); 
		RIQ_SET_RETURNNAME(3, s_gbk);

		s_gbk.clear();
		memset(s_str, 0, 32);
		__explainRentInfoTime(s_str, (char *)rent_info_fmt[2].return_time);
		s_gbk = codec->toUnicode(s_str); 
		RIQ_SET_RETURNTIME(3, s_gbk);

		s_gbk.clear();
		s_gbk = codec->toUnicode((char *)rent_info_fmt[2].used_time); 
		RIQ_SET_USEDTIME(3, s_gbk);

		s_gbk.clear();
		memset(s_str, 0, 32);
		__explainRentInfoFee(s_str, (char *)rent_info_fmt[2].fee);
		s_gbk = codec->toUnicode(s_str); 
		RIQ_SET_FEE(3, s_gbk);

		if(ret <= 3)
			goto Done;

		/*****4******/
		RIQ_SET_SN(4, "4");
		s_gbk = codec->toUnicode((char *)rent_info_fmt[3].bike_NO);  
		RIQ_SET_BIKENO(4, s_gbk);	
				
		s_gbk.clear();
		s_gbk = codec->toUnicode((char *)rent_info_fmt[3].rent_name); 
		RIQ_SET_RENTNAME(4, s_gbk);

		s_gbk.clear();
		memset(s_str, 0, 32);
		__explainRentInfoTime(s_str, (char *)rent_info_fmt[3].rent_time);
		s_gbk = codec->toUnicode(s_str); 
		RIQ_SET_RENTTIME(4, s_gbk);

		s_gbk.clear();
		s_gbk = codec->toUnicode((char *)rent_info_fmt[3].return_name); 
		RIQ_SET_RETURNNAME(4, s_gbk);

		s_gbk.clear();
		memset(s_str, 0, 32);
		__explainRentInfoTime(s_str, (char *)rent_info_fmt[3].return_time);
		s_gbk = codec->toUnicode(s_str); 
		RIQ_SET_RETURNTIME(4, s_gbk);

		s_gbk.clear();
		s_gbk = codec->toUnicode((char *)rent_info_fmt[3].used_time); 
		RIQ_SET_USEDTIME(4, s_gbk);

		s_gbk.clear();
		memset(s_str, 0, 32);
		__explainRentInfoFee(s_str, (char *)rent_info_fmt[3].fee);
		s_gbk = codec->toUnicode(s_str); 
		RIQ_SET_FEE(4, s_gbk);

		if(ret <= 4)
			goto Done;	

		/*****5******/
		RIQ_SET_SN(5, "5");
		s_gbk = codec->toUnicode((char *)rent_info_fmt[4].bike_NO);  
		RIQ_SET_BIKENO(5, s_gbk);	
				
		s_gbk.clear();
		s_gbk = codec->toUnicode((char *)rent_info_fmt[4].rent_name); 
		RIQ_SET_RENTNAME(5, s_gbk);

		s_gbk.clear();
		memset(s_str, 0, 32);
		__explainRentInfoTime(s_str, (char *)rent_info_fmt[4].rent_time);
		s_gbk = codec->toUnicode(s_str); 
		RIQ_SET_RENTTIME(5, s_gbk);

		s_gbk.clear();
		s_gbk = codec->toUnicode((char *)rent_info_fmt[4].return_name); 
		RIQ_SET_RETURNNAME(5, s_gbk);

		s_gbk.clear();
		memset(s_str, 0, 32);
		__explainRentInfoTime(s_str, (char *)rent_info_fmt[4].return_time);
		s_gbk = codec->toUnicode(s_str); 
		RIQ_SET_RETURNTIME(5, s_gbk);

		s_gbk.clear();
		s_gbk = codec->toUnicode((char *)rent_info_fmt[4].used_time); 
		RIQ_SET_USEDTIME(5, s_gbk);

		s_gbk.clear();
		memset(s_str, 0, 32);
		__explainRentInfoFee(s_str, (char *)rent_info_fmt[4].fee);
		s_gbk = codec->toUnicode(s_str); 
		RIQ_SET_FEE(5, s_gbk);

		if(ret <= 5)
			goto Done;	

		/*****6******/
		RIQ_SET_SN(6, "6");
		s_gbk = codec->toUnicode((char *)rent_info_fmt[5].bike_NO);  
		RIQ_SET_BIKENO(6, s_gbk);	
				
		s_gbk.clear();
		s_gbk = codec->toUnicode((char *)rent_info_fmt[5].rent_name); 
		RIQ_SET_RENTNAME(6, s_gbk);

		s_gbk.clear();
		memset(s_str, 0, 32);
		__explainRentInfoTime(s_str, (char *)rent_info_fmt[5].rent_time);
		s_gbk = codec->toUnicode(s_str); 
		RIQ_SET_RENTTIME(6, s_gbk);

		s_gbk.clear();
		s_gbk = codec->toUnicode((char *)rent_info_fmt[5].return_name); 
		RIQ_SET_RETURNNAME(6, s_gbk);

		s_gbk.clear();
		memset(s_str, 0, 32);
		__explainRentInfoTime(s_str, (char *)rent_info_fmt[5].return_time);
		s_gbk = codec->toUnicode(s_str); 
		RIQ_SET_RETURNTIME(6, s_gbk);

		s_gbk.clear();
		s_gbk = codec->toUnicode((char *)rent_info_fmt[5].used_time); 
		RIQ_SET_USEDTIME(6, s_gbk);

		s_gbk.clear();
		memset(s_str, 0, 32);
		__explainRentInfoFee(s_str, (char *)rent_info_fmt[5].fee);
		s_gbk = codec->toUnicode(s_str); 
		RIQ_SET_FEE(6, s_gbk);

		if(ret <= 6)
			goto Done;	

#if 0
		/*****7******/
		RIQ_SET_SN(7, "7");
		s_gbk = codec->toUnicode((char *)rent_info_fmt[6].bike_NO);  
		RIQ_SET_BIKENO(7, s_gbk);	
				
		s_gbk.clear();
		s_gbk = codec->toUnicode((char *)rent_info_fmt[6].rent_name); 
		RIQ_SET_RENTNAME(7, s_gbk);

		s_gbk.clear();
		memset(s_str, 0, 32);
		__explainRentInfoTime(s_str, (char *)rent_info_fmt[6].rent_time);
		s_gbk = codec->toUnicode(s_str); 
		RIQ_SET_RENTTIME(7, s_gbk);

		s_gbk.clear();
		s_gbk = codec->toUnicode((char *)rent_info_fmt[6].return_name); 
		RIQ_SET_RETURNNAME(7, s_gbk);

		s_gbk.clear();
		memset(s_str, 0, 32);
		__explainRentInfoTime(s_str, (char *)rent_info_fmt[6].return_time);
		s_gbk = codec->toUnicode(s_str); 
		RIQ_SET_RETURNTIME(7, s_gbk);

		s_gbk.clear();
		s_gbk = codec->toUnicode((char *)rent_info_fmt[6].used_time); 
		RIQ_SET_USEDTIME(7, s_gbk);

		s_gbk.clear();
		memset(s_str, 0, 32);
		__explainRentInfoFee(s_str, (char *)rent_info_fmt[6].fee);
		s_gbk = codec->toUnicode(s_str); 
		RIQ_SET_FEE(7, s_gbk);

		if(ret <= 7)
			goto Done;	


		/*****8******/
		RIQ_SET_SN(8, "8");
		s_gbk = codec->toUnicode((char *)rent_info_fmt[7].bike_NO);  
		RIQ_SET_BIKENO(8, s_gbk);	
				
		s_gbk.clear();
		s_gbk = codec->toUnicode((char *)rent_info_fmt[7].rent_name); 
		RIQ_SET_RENTNAME(8, s_gbk);

		s_gbk.clear();
		memset(s_str, 0, 32);
		__explainRentInfoTime(s_str, (char *)rent_info_fmt[7].rent_time);
		s_gbk = codec->toUnicode(s_str); 
		RIQ_SET_RENTTIME(8, s_gbk);

		s_gbk.clear();
		s_gbk = codec->toUnicode((char *)rent_info_fmt[7].return_name); 
		RIQ_SET_RETURNNAME(8, s_gbk);

		s_gbk.clear();
		memset(s_str, 0, 32);
		__explainRentInfoTime(s_str, (char *)rent_info_fmt[7].return_time);
		s_gbk = codec->toUnicode(s_str); 
		RIQ_SET_RETURNTIME(8, s_gbk);

		s_gbk.clear();
		s_gbk = codec->toUnicode((char *)rent_info_fmt[7].used_time); 
		RIQ_SET_USEDTIME(8, s_gbk);

		s_gbk.clear();
		memset(s_str, 0, 32);
		__explainRentInfoFee(s_str, (char *)rent_info_fmt[7].fee);
		s_gbk = codec->toUnicode(s_str); 
		RIQ_SET_FEE(8, s_gbk);

		if(ret <= 8)
			goto Done;	


		/*****9******/
		RIQ_SET_SN(9, "9");
		s_gbk = codec->toUnicode((char *)rent_info_fmt[8].bike_NO);  
		RIQ_SET_BIKENO(9, s_gbk);	
				
		s_gbk.clear();
		s_gbk = codec->toUnicode((char *)rent_info_fmt[8].rent_name); 
		RIQ_SET_RENTNAME(9, s_gbk);

		s_gbk.clear();
		memset(s_str, 0, 32);
		__explainRentInfoTime(s_str, (char *)rent_info_fmt[8].rent_time);
		s_gbk = codec->toUnicode(s_str); 
		RIQ_SET_RENTTIME(9, s_gbk);

		s_gbk.clear();
		s_gbk = codec->toUnicode((char *)rent_info_fmt[8].return_name); 
		RIQ_SET_RETURNNAME(9, s_gbk);

		s_gbk.clear();
		memset(s_str, 0, 32);
		__explainRentInfoTime(s_str, (char *)rent_info_fmt[8].return_time);
		s_gbk = codec->toUnicode(s_str); 
		RIQ_SET_RETURNTIME(9, s_gbk);

		s_gbk.clear();
		s_gbk = codec->toUnicode((char *)rent_info_fmt[8].used_time); 
		RIQ_SET_USEDTIME(9, s_gbk);

		s_gbk.clear();
		memset(s_str, 0, 32);
		__explainRentInfoFee(s_str, (char *)rent_info_fmt[8].fee);
		s_gbk = codec->toUnicode(s_str); 
		RIQ_SET_FEE(9, s_gbk);

		if(ret <= 9)
			goto Done;	

		/*****10******/
		RIQ_SET_SN(10, "10");
		s_gbk = codec->toUnicode((char *)rent_info_fmt[9].bike_NO);  
		RIQ_SET_BIKENO(10, s_gbk);	
				
		s_gbk.clear();
		s_gbk = codec->toUnicode((char *)rent_info_fmt[9].rent_name); 
		RIQ_SET_RENTNAME(10, s_gbk);

		s_gbk.clear();
		memset(s_str, 0, 32);
		__explainRentInfoTime(s_str, (char *)rent_info_fmt[9].rent_time);
		s_gbk = codec->toUnicode(s_str); 
		RIQ_SET_RENTTIME(10, s_gbk);

		s_gbk.clear();
		s_gbk = codec->toUnicode((char *)rent_info_fmt[9].return_name); 
		RIQ_SET_RETURNNAME(10, s_gbk);

		s_gbk.clear();
		memset(s_str, 0, 32);
		__explainRentInfoTime(s_str, (char *)rent_info_fmt[9].return_time);
		s_gbk = codec->toUnicode(s_str); 
		RIQ_SET_RETURNTIME(10, s_gbk);

		s_gbk.clear();
		s_gbk = codec->toUnicode((char *)rent_info_fmt[9].used_time); 
		RIQ_SET_USEDTIME(10, s_gbk);

		s_gbk.clear();
		memset(s_str, 0, 32);
		__explainRentInfoFee(s_str, (char *)rent_info_fmt[9].fee);
		s_gbk = codec->toUnicode(s_str); 
		RIQ_SET_FEE(10, s_gbk);

		if(ret <= 10)
			goto Done;	
#endif
	}


Done:
	return ret;
}

void RentInfoQueryDlg::slots_timerDone(void)
{	
	static int ret = RET_ERROR_OP_FAIL;
	int err = LIB_GUI_ERROR;
	int flag1 = 0;
	m_stat = RET_ERROR_OP_FAIL;
	QString stringShow;
	lib_lnt_getversion_ack_t lnt_ver;
	gui_access_state_t state;
		
	if(timerFlag == 0) //检查网络是否正常,读卡器是否正常
	{
		memset(&lnt_ver, 0, sizeof(lib_lnt_getversion_ack_t));
		ret = lib_lnt_get_version(&lnt_ver, &m_stat, 200);
		if((ret > 0) && (m_stat == LNT_MOC_ERROR_NORMAL))  //有读卡器
		{
			flag1 = 1;
			timerFlag = 1;
		}
		else  //没有读卡器
		{
			flag1 = 0;
			timerCount = BINFO_RETURN_FAIL_PERIOD;
			ret = RET_ERROR_LNT_BAD; //读卡器异常 
			timerFlag = 2;
		}
		
		if(flag1 == 1)
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
					flag1 = 0;
					timerFlag = 1;
				}
			}
		}
	}

	if(timerFlag == 1)  //询卡
	{
		__allWidgetClear();

		ret = __getPageConfig(); 
		if(ret == RET_ERROR_CARD_NOTAG)
		{
			stringShow.sprintf("提示:请在%02d秒内,把卡放置到指定刷卡区域上", timerCount);
			label_rentInfoQryShow->setText(stringShow);	
		}

		if(timerCount == 0)
		{
			esc_quit_key = ECS_QUIT_ON;
			label_rentInfoQryShow->setText(tr("提示:租还车记录查询超时,自动返回上一级界面, 按[退出]键直接返回"));
			Helper::__Sleep(1);
			timerFlag = 3;
			emit signals_rentInfoReject();
		}

		if(ret != RET_ERROR_CARD_NOTAG)
		{
			timerCount = BINFO_RETURN_FAIL_PERIOD;
			timerFlag = 2;
		}
	}


	if(timerFlag == 2)
	{
		switch(ret)
		{
			case RET_ERROR_LNT_BAD: //读卡器异常
			{
				esc_quit_key = ECS_QUIT_ON;
				stringShow.sprintf("提示:读卡器异常,不能执行异常处理操作, %02d秒后自动返回上一级界面, 按[退出]键直接返回", timerCount);
			}
			break;

			case RET_ERROR_NETWORK_DISCONNT: //网络异常
			{
				esc_quit_key = ECS_QUIT_ON;
				stringShow.sprintf("提示:网络异常,不能执行异常处理操作, %02d秒后自动返回上一级界面, 按[退出]键直接返回", timerCount);	
			}
			break;

                        case RET_ERROR_NORMAL:
			{
				/* 开始查询租还车记录 */
				timerCount = BINFO_EHANDLE_PERIOD;

                                rinfo_qry_req.item = 6; //请求记录条数
				err = lib_gui_rent_info_qry_req(g_gui, &rinfo_qry_req, 500);
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
				stringShow.sprintf("提示:租还车记录查询失败, %02d秒后自动返回上一级界面, 按[退出]键直接返回", timerCount);
			}
			break;
#if 1
			case RET_ERROR_NOVIP:
			{
				esc_quit_key = ECS_QUIT_ON;
				stringShow.sprintf("提示:不支持非会员卡操作, %02d秒后自动返回上一级界面, 按[退出]键直接返回", timerCount);
			}
			break;
#endif
			case RET_ERROR_USED_TIMES_TOO_MUCH:
			{
				esc_quit_key = ECS_QUIT_ON;
				stringShow.sprintf("提示:租还车记录查询次数过于频繁,请在%2d分钟后再试, %02d秒后自动返回上一级界面, 按[退出]键直接返回", \
					RIQ_COND_LAST_TIME, timerCount);
			}
			break;

                        case RET_ERROR_OP_LOOP:
			{
				/* 租还记录查询流程 */
				esc_quit_key = ECS_QUIT_OFF;

				stringShow.sprintf("提示:租还车信息查询进行中,请勿移动卡片并耐心等待 %02d秒,并仔细留意屏幕上的提示", timerCount);
					
				memset(&rinfo_qry_ack, 0, sizeof(gui_rent_info_qry_ack_t));
				err = lib_gui_rent_info_qry_ack(g_gui, &rinfo_qry_ack, 900);
				if(err == LIB_GUI_EFULL)  //有数据
				{
					if(timer.isActive())
					{
						timer.stop();
					}

					if(rinfo_qry_ack.item == 0) //找不到租还车记录
					{
						timerCount = 10; //10秒
						handleReslut = 4;
					}
					else
					{
						timerCount = 30; //30秒
						handleReslut = 3;
						__setRentInfo(); //显示租还车信息

						#if 1
						if(memcmp(g_riq_cond.last_pid, rinfo_qry_req.pid, 8) == 0) //同一张卡连续查询
						{
							g_riq_cond.used_times++;
							time(&g_riq_cond.last_time);
						}
						else
						{
							memset(&g_riq_cond, 0, sizeof(g_riq_cond));
							memcpy(g_riq_cond.last_pid, rinfo_qry_req.pid, 8);
							g_riq_cond.used_times++;
							time(&g_riq_cond.last_time);
						}
						#endif

						#if 0
						if(g_riq_cond_idx >= RIQ_COND_IDX_MAX)
							g_riq_cond_idx = 0;

						if(memcmp(g_riq_cond[g_riq_cond_idx].last_pid, rinfo_qry_req.pid, 8) == 0) //同一张卡连续查询
						{
							g_riq_cond[g_riq_cond_idx].used_times++;
							time(&g_riq_cond[g_riq_cond_idx].last_time);
						}
						else
						{	int k;
						
							for(k = 0; k < RIQ_COND_IDX_MAX; k++)
							{
								memset(&g_riq_cond[k], 0, sizeof(g_riq_cond));
							}
							
							memcpy(g_riq_cond[g_riq_cond_idx].last_pid, rinfo_qry_req[g_riq_cond_idx].pid, 8);
							g_riq_cond[g_riq_cond_idx].used_times++;
							time(&g_riq_cond[g_riq_cond_idx].last_time);
						}

						g_riq_cond_idx++;
						#endif
					}
	
					ret = RET_ERROR_OP_SUCCESS;
					timer.start(1000);
					timerFlag = 2;
					esc_quit_key = ECS_QUIT_ON;
					
				}
				else if((err == LIB_GUI_EEMPTY) && (timerCount == 1)) //超时还没有收到数据
				{
					ret = RET_ERROR_OP_FAIL; //处理失败
					timerCount = BINFO_RETURN_FAIL_PERIOD;
					timerFlag = 2;
				}
				else if(err != LIB_GUI_EEMPTY)//读取数据失败
				{
					ret = RET_ERROR_OP_FAIL; //处理失败
					timerCount = BINFO_RETURN_FAIL_PERIOD;
					timerFlag = 2;
				}
			}

			case RET_ERROR_OP_SUCCESS: //收到租还车信息应答
			{
				if(handleReslut == 3)
                                        stringShow.sprintf("提示:一共查询到%d条租还车记录, %02d秒后自动返回上一级界面, 按[退出]键直接返回", rinfo_qry_ack.item, timerCount);

				if(handleReslut == 4)
					stringShow.sprintf("提示:找不到该卡的租还车记录, %02d秒后自动返回上一级界面, 按[退出]键直接返回", timerCount);
			}
			break;
		}

		label_rentInfoQryShow->setText(stringShow);
		
		if(timerCount == 0)
		{
			timerFlag = 3;
			emit signals_rentInfoReject();
		}
	}
	
	timerCount--;
}

void RentInfoQueryDlg::__allWidgetClear(void)
{
	label_rentInfoQryShow->clear();

	RIQ_SN(1)->clear();
	RIQ_BIKENO(1)->clear();
	RIQ_RENTNAME(1)->clear();
	RIQ_RENTTIME(1)->clear();
	RIQ_RETURNNAME(1)->clear();
	RIQ_RETURNTIME(1)->clear();
	RIQ_USEDTIME(1)->clear();
	RIQ_FEE(1)->clear();

	RIQ_SN(2)->clear();
	RIQ_BIKENO(2)->clear();
	RIQ_RENTNAME(2)->clear();
	RIQ_RENTTIME(2)->clear();
	RIQ_RETURNNAME(2)->clear();
	RIQ_RETURNTIME(2)->clear();
	RIQ_USEDTIME(2)->clear();
	RIQ_FEE(2)->clear();

	RIQ_SN(3)->clear();
	RIQ_BIKENO(3)->clear();
	RIQ_RENTNAME(3)->clear();
	RIQ_RENTTIME(3)->clear();
	RIQ_RETURNNAME(3)->clear();
	RIQ_RETURNTIME(3)->clear();
	RIQ_USEDTIME(3)->clear();
	RIQ_FEE(3)->clear();

	RIQ_SN(4)->clear();
	RIQ_BIKENO(4)->clear();
	RIQ_RENTNAME(4)->clear();
	RIQ_RENTTIME(4)->clear();
	RIQ_RETURNNAME(4)->clear();
	RIQ_RETURNTIME(4)->clear();
	RIQ_USEDTIME(4)->clear();
	RIQ_FEE(4)->clear();

	RIQ_SN(5)->clear();
	RIQ_BIKENO(5)->clear();
	RIQ_RENTNAME(5)->clear();
	RIQ_RENTTIME(5)->clear();
	RIQ_RETURNNAME(5)->clear();
	RIQ_RETURNTIME(5)->clear();
	RIQ_USEDTIME(5)->clear();
	RIQ_FEE(5)->clear();

	RIQ_SN(6)->clear();
	RIQ_BIKENO(6)->clear();
	RIQ_RENTNAME(6)->clear();
	RIQ_RENTTIME(6)->clear();
	RIQ_RETURNNAME(6)->clear();
	RIQ_RETURNTIME(6)->clear();
	RIQ_USEDTIME(6)->clear();
	RIQ_FEE(6)->clear();
#if 0
	RIQ_SN(7)->clear();
	RIQ_BIKENO(7)->clear();
	RIQ_RENTNAME(7)->clear();
	RIQ_RENTTIME(7)->clear();
	RIQ_RETURNNAME(7)->clear();
	RIQ_RETURNTIME(7)->clear();
	RIQ_USEDTIME(7)->clear();
	RIQ_FEE(7)->clear();

	RIQ_SN(8)->clear();
	RIQ_BIKENO(8)->clear();
	RIQ_RENTNAME(8)->clear();
	RIQ_RENTTIME(8)->clear();
	RIQ_RETURNNAME(8)->clear();
	RIQ_RETURNTIME(8)->clear();
	RIQ_USEDTIME(8)->clear();
	RIQ_FEE(8)->clear();
	
	RIQ_SN(9)->clear();
	RIQ_BIKENO(9)->clear();
	RIQ_RENTNAME(9)->clear();
	RIQ_RENTTIME(9)->clear();
	RIQ_RETURNNAME(9)->clear();
	RIQ_RETURNTIME(9)->clear();
	RIQ_USEDTIME(9)->clear();
	RIQ_FEE(9)->clear();

	RIQ_SN(10)->clear();
	RIQ_BIKENO(10)->clear();
	RIQ_RENTNAME(10)->clear();
	RIQ_RENTTIME(10)->clear();
	RIQ_RETURNNAME(10)->clear();
	RIQ_RETURNTIME(10)->clear();
	RIQ_USEDTIME(10)->clear();
	RIQ_FEE(10)->clear();
#endif
}

int RentInfoQueryDlg::__getPageConfig(void)
{
	int retval = LIB_LNT_ERROR;
	m_stat = LNT_MOC_ERROR_CARD_OP_FAIL;
	lib_lnt_qry_ticket_info_ack_t lnt_qry_ticket;
	pmflag_t pf;
		
	memset(&lnt_qry_ticket, 0, sizeof(lib_lnt_qry_ticket_info_ack_t));
	
	retval = lib_lnt_qry_ticket_info(&lnt_qry_ticket, &m_stat, 300);
	if((retval <= 0) && (m_stat != LNT_MOC_ERROR_NORMAL))
		return RET_ERROR_OP_FAIL;

	memcpy(rinfo_qry_req.pid, lnt_qry_ticket.ticket.pid, 8);  //票卡物理卡号
        rinfo_qry_req.item = 6; //条数
		
	if(m_stat == LNT_MOC_ERROR_CARD_NOTAG)
			return RET_ERROR_CARD_NOTAG;
		
	memset(&pf, 0, sizeof(pmflag_t));
	memcpy(&pf, &(lnt_qry_ticket.ticket.pmflag), 1);  //押金标志
		
	if((lnt_qry_ticket.ticket.pmflag == 0xff) || (pf.pflag != 1))
		return RET_ERROR_NOVIP;

	time(&now_time_sec); //现在秒数

	#if 0
	int j;
	for(j = 0; j < RIQ_COND_IDX_MAX; j++)
	{		
		if(now_time_sec < g_riq_cond[j].last_time)
			g_riq_cond[j].last_time = 0;   //保护机制

		if((memcmp(lnt_qry_ticket.ticket.pid, g_riq_cond[j].last_pid, 8) == 0) &&  
			(g_riq_cond[j].used_times >= 2) && 
			((now_time_sec - g_riq_cond[j].last_time) <= RIQ_COND_LAST_TIME * 60))
		{
			return RET_ERROR_USED_TIMES_TOO_MUCH;   //使用次数太多
		}
	}
	#endif
	
	#if 1
	if(now_time_sec < g_riq_cond.last_time)
		g_riq_cond.last_time = 0;   //保护机制

	if((memcmp(lnt_qry_ticket.ticket.pid, g_riq_cond.last_pid, 8) == 0) &&  
		(g_riq_cond.used_times >= 2) && 
		((now_time_sec - g_riq_cond.last_time) <= RIQ_COND_LAST_TIME * 60))
	{
		#if 0  //关闭检查查询次数检查功能
		return RET_ERROR_USED_TIMES_TOO_MUCH;   //使用次数太多
		#endif
	}
	#endif

	
	return RET_ERROR_NORMAL;
}

void RentInfoQueryDlg::keyPressEvent(QKeyEvent *e)
{
	if(e->key() == Qt::Key_Escape)  
	{
		if(esc_quit_key == ECS_QUIT_ON)
			emit signals_rentInfoAccept();	
	}
}






