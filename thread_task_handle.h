#ifndef __THREAD_TASK_HANDLE_H__
#define __THREAD_TASK_HANDLE_H__

#include <QRunnable>
#include <QObject>
#include <time.h>

//#include "gui_header.h"

enum EX_TASK_STEP
{
	EX_TASK_STEP_LNT_DEVICE = 1, //检查岭南通读卡器是否存在
	EX_TASK_STEP_REGISTER = 2, //是否已经连接中心注册
	EX_TASK_STEP_TICKET_QRY= 3,  //票卡查询-询卡
	EX_TASK_STEP_RENT_INFO_QRY = 4, //租还车信息查询
	
	EX_TASK_STEP_EHANDLE_REQ = 10,  //异常处理请求
	EX_TASK_STEP_EHANDLE_ACK = 11, //异常处理确认

	EX_TASK_STEP_EHANDLE_START = 12, //开始异常处理

	EX_TASK_STEP_H_RENT_INFO_QRY = 20, //租还车信息查询-异常处理
	EX_TASK_STEP_H_RENT_INFO_SET = 21, //租还车信息设置-异常处理
	EX_TASK_STEP_H_CONSUME = 22,  //消费-异常处理
	EX_TASK_STEP_H_RECORD_GET = 23, //获取消费记录-异常处理

	EX_TASK_STEP_CONSUME_UPLOAD = 30, //消费记录上传

	EX_TASK_STEP_FINISH = 80,  //完毕
};




enum THREAD_TASK_RESULT
{
	THREAD_TASK_RESULT_OP_SHOW_LOOP = 1,
	THREAD_TASK_RESULT_OP_TIMER_LOOP,
	THREAD_TASK_RESULT_OP_FREE,
	THREAD_TASK_RESULT_OP_FAIL_FREE,
};


enum TASK_TYPE
{
	TASK_TYPE_EXCEPTION = 1,   //异常处理
};


struct ThreadTaskResult
{
	unsigned int step;      //步骤
	unsigned int result;   //结果
	char s_info[128];  //信息
};


struct ThreadTaskData
{
	int taskType;  //服务类型

	int  refresh_period; //刷新周期
	int return_period;  //返回刷新周期
};

class ThreadTaskHandle :  public QObject, public QRunnable
{
	Q_OBJECT
public:
	ThreadTaskHandle(struct ThreadTaskData *data);

signals:
	void signals_ExceptionResult(struct ThreadTaskResult *result);  //异常卡处理

private:
	void ThreadSleep(unsigned long secs);
	void mThreadSleep(unsigned long msecs);
	int ExceptionHandle(struct ThreadTaskResult *TaskResult);

	
	

protected:
    	void run();	

private:
	unsigned int threadTaskStep;
	struct ThreadTaskData threadTaskData;
	time_t begin_times, end_times;
	int dealCnt;
	unsigned int m_serialNumber;
	int doneFree;
};


#endif


