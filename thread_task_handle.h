#ifndef __THREAD_TASK_HANDLE_H__
#define __THREAD_TASK_HANDLE_H__

#include <QRunnable>
#include <QObject>
#include <time.h>

//#include "gui_header.h"

enum EX_TASK_STEP
{
	EX_TASK_STEP_LNT_DEVICE = 1, //�������ͨ�������Ƿ����
	EX_TASK_STEP_REGISTER = 2, //�Ƿ��Ѿ���������ע��
	EX_TASK_STEP_TICKET_QRY= 3,  //Ʊ����ѯ-ѯ��
	EX_TASK_STEP_RENT_INFO_QRY = 4, //�⻹����Ϣ��ѯ
	
	EX_TASK_STEP_EHANDLE_REQ = 10,  //�쳣��������
	EX_TASK_STEP_EHANDLE_ACK = 11, //�쳣����ȷ��

	EX_TASK_STEP_EHANDLE_START = 12, //��ʼ�쳣����

	EX_TASK_STEP_H_RENT_INFO_QRY = 20, //�⻹����Ϣ��ѯ-�쳣����
	EX_TASK_STEP_H_RENT_INFO_SET = 21, //�⻹����Ϣ����-�쳣����
	EX_TASK_STEP_H_CONSUME = 22,  //����-�쳣����
	EX_TASK_STEP_H_RECORD_GET = 23, //��ȡ���Ѽ�¼-�쳣����

	EX_TASK_STEP_CONSUME_UPLOAD = 30, //���Ѽ�¼�ϴ�

	EX_TASK_STEP_FINISH = 80,  //���
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
	TASK_TYPE_EXCEPTION = 1,   //�쳣����
};


struct ThreadTaskResult
{
	unsigned int step;      //����
	unsigned int result;   //���
	char s_info[128];  //��Ϣ
};


struct ThreadTaskData
{
	int taskType;  //��������

	int  refresh_period; //ˢ������
	int return_period;  //����ˢ������
};

class ThreadTaskHandle :  public QObject, public QRunnable
{
	Q_OBJECT
public:
	ThreadTaskHandle(struct ThreadTaskData *data);

signals:
	void signals_ExceptionResult(struct ThreadTaskResult *result);  //�쳣������

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


