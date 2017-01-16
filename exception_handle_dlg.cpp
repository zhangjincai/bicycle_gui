#include <QtGui>
#include <QDebug>
#include "exception_handle_dlg.h"
#include "lib_serialnumber.h"
#include "lib_general.h"
#include "helper.h"
#include "lib_gui.h"
#include "lib_wireless.h"
#include "lib_lnt.h"

#define BINFO_REFRESH_PERIOD					(20)
#define BINFO_RETURN_PERIOD						(15)


enum RET_ERROR
{
	RET_ERROR_NORMAL = 0,   //正常
	RET_ERROR_CARD_NOTAG = 1,  //没有卡
	RET_ERROR_NOVIP = 2,  //非会员 
	RET_ERROR_DP_INIT = 3,	//押金初始化
	RET_ERROR_OP_FAIL = 4, //操作失败
	RET_ERROR_OP_SUCCESS = 5,  //操作成功
	RET_ERROR_OP_LOOP = 6, //轮询
	RET_ERROR_LNT_BAD = 7, //岭南通读卡器坏 
	RET_ERROR_NETWORK_DISCONNT = 8, //网络不通
	RET_ERROR_LNT_NOT_REG = 9, //读卡器没有签到
	RET_ERROR_NOT_EXCEPT = 10, //不存在异常
};

extern lib_gui_t *g_gui;
extern lib_wl_t *g_wl;

ExceptionHandleDlg::ExceptionHandleDlg(QWidget *parent) : QDialog(parent)
{
	setupUi(this);
	setWindowFlags(Qt::FramelessWindowHint);	

	esc_quit_key = ECS_QUIT_OFF;  //允许使用ESC退出键
	retError = RET_ERROR_NORMAL; 
		
	struct ThreadTaskData threadTaskData;
	memset(&threadTaskData, 0, sizeof(struct ThreadTaskData));

	threadTaskData.taskType = TASK_TYPE_EXCEPTION;  //异常处理
	threadTaskData.refresh_period = BINFO_REFRESH_PERIOD;
	threadTaskData.return_period = BINFO_RETURN_PERIOD;
	ThreadTaskHandle * threadTaskHandle = new ThreadTaskHandle(&threadTaskData);
	if(threadTaskHandle != NULL)
	{
		threadTaskHandle->setAutoDelete(true);
		connect(threadTaskHandle, SIGNAL(signals_ExceptionResult(struct ThreadTaskResult *)), this, SLOT(slots_ThreadTaskResult(struct ThreadTaskResult *)), Qt::QueuedConnection);
	 	QThreadPool::globalInstance()->start(threadTaskHandle);	
	}

	QString string;
	string.sprintf("提示:请在%02d秒内,把卡放置到指定刷卡区域上", BINFO_REFRESH_PERIOD - (8));
	label_exceptionHandleShow->setText(string);
	
	timerCount = BINFO_REFRESH_PERIOD;
	connect(&timer, SIGNAL(timeout()), this, SLOT(slots_timerDone()));

	connect(this, SIGNAL(signals_exceptionAccept()), this, SLOT(accept()));
	connect(this, SIGNAL(signals_exceptionReject()), this, SLOT(reject()));
}

ExceptionHandleDlg::~ExceptionHandleDlg(void)
{
	
}

void ExceptionHandleDlg::slots_ThreadTaskResult(struct ThreadTaskResult *threadResult)
{
	if(threadResult == NULL)
	{
		SYS_LOG_ALERT("%s: threadResult is NULL\n", __FUNCTION__);
		return;
	}
	
	switch(threadResult->result)
	{
		case THREAD_TASK_RESULT_OP_SHOW_LOOP:
		{
			infoString.sprintf(threadResult->s_info);
			label_exceptionHandleShow->setText(infoString);
		}
		break;
		
		case THREAD_TASK_RESULT_OP_TIMER_LOOP:
		{
			esc_quit_key = ECS_QUIT_ON;
			timerCount = BINFO_RETURN_PERIOD;
			retError = RET_ERROR_OP_LOOP;
			memset(&m_ThreadTaskResult, 0, sizeof(struct ThreadTaskResult));
			memcpy(&m_ThreadTaskResult, threadResult, sizeof(struct ThreadTaskResult));
			
			timer.start(1000); 
		}
		break;

		case THREAD_TASK_RESULT_OP_FREE: //成功,释放内存
		{
			esc_quit_key = ECS_QUIT_ON;
			timerCount = BINFO_RETURN_PERIOD;
			retError = RET_ERROR_OP_LOOP;
			memset(&m_ThreadTaskResult, 0, sizeof(struct ThreadTaskResult));
			memcpy(&m_ThreadTaskResult, threadResult, sizeof(struct ThreadTaskResult));
			
			timer.start(1000); 

			if(threadResult != NULL)
			{
				free(threadResult);
				threadResult = NULL;
			}
		}
		break;

		case THREAD_TASK_RESULT_OP_FAIL_FREE:  //失败，释放内存
		{
			esc_quit_key = ECS_QUIT_ON;
			timerCount = BINFO_RETURN_PERIOD;
			retError = RET_ERROR_OP_LOOP;
			memset(&m_ThreadTaskResult, 0, sizeof(struct ThreadTaskResult));
			memcpy(&m_ThreadTaskResult, threadResult, sizeof(struct ThreadTaskResult));
			
			timer.start(1000); 
			
			if(threadResult != NULL)
			{
				free(threadResult);
				threadResult = NULL;
			}
		}
		break;
	}

//	if(threadResult->step == EX_TASK_STEP_TICKET_QRY)
//		esc_quit_key = ECS_QUIT_ON;
		
	//qDebug("ActiveThreadCount:%d\n", QThreadPool::globalInstance()->activeThreadCount());
}

void ExceptionHandleDlg::slots_timerDone(void)
{
	
		
	switch(retError)
	{
		case RET_ERROR_OP_LOOP:
		{
			infoString.sprintf(m_ThreadTaskResult.s_info);
			QString string;
			string.sprintf(",系统将会在%02d秒后自动返回上一级界面, 按[退出]键直接返回",  timerCount);
			infoString.append(string);
			string.clear();
		}
		break;
	}
	


	label_exceptionHandleShow->setText(infoString);
	infoString.clear();
	
	if(timerCount == 0)
	{
		emit signals_exceptionReject();
	}
	
	timerCount--;
}


void ExceptionHandleDlg::keyPressEvent(QKeyEvent *e)
{
	if(e->key() == Qt::Key_Escape)  
	{
		if(esc_quit_key == ECS_QUIT_ON)
			emit signals_exceptionAccept();	
	}
}




