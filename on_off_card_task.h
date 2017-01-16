#ifndef __ON_OFF_CARD_TASK_H__
#define __ON_OFF_CARD_TASK_H__

#include <QRunnable>
#include <QObject>
#include <QRunnable>
#include <time.h>
#include "lib_lnt.h"


enum CARD_SERVICE
{
		CARD_SERVICE_NOT = 0,

        CARD_SERVICE_ON = 1,        //����
        CARD_SERVICE_OFF = 2,       //����
        CARD_SERVICE_RECHARGE = 3,  //��ֵ 2016-04-07 by zjc

        CARD_SERVICE_ON_NORMAL = 4, //��ͨ��
        CARD_SERVICE_ON_TIME = 5,   //�ο�
        CARD_SERVICE_ON_DAY = 6,    //�쿨
        CARD_SERVICE_ON_MONTH = 7,  //�¿�
        CARD_SERVICE_ON_YEAR = 8,   //�꿨
};

/*
 * �������в���
 */
enum TaskRunStep
{
	TaskRunStep_Input_ID = 1,          //�������֤������ֻ�����
	TaskRunStep_Deposit = 2,           //�۳�Ѻ��
	TaskRunStep_Get_OrderNO = 3,  //��ȡ������
	TaskRunStep_Modify_Member_Flag = 4,  //�޸Ļ�Ա��־
	TaskRunStep_Recharge = 5,   //��ֵ
	TaskRunStep_Finish = 6,   //�������


	TaskRunStep_p0 = 10,
	TaskRunStep_p1 = 11,
	TaskRunStep_p2 = 12,
	TaskRunStep_p3 = 13,
	TaskRunStep_p4 = 14,
	TaskRunStep_p5 = 15,
	TaskRunStep_p6 = 16
};

/*
 * ��������״̬
 */
 enum TaskRunStepStat
{
	TaskRunStepStat_INIT = 1,
	TaskRunStepStat_ING = 2,
	TaskRunStepStat_OK = 3,
	TaskRunStepStat_Fail = 4
	
};

struct TaskResult
{
	int type;  //����

	unsigned int step;      //����
	unsigned int result;   //���
	char s_info[128];  //��Ϣ

	int serviceType;  	//��������
	int taskRunStat;  //����״̬
	int taskRunStep;   //�������в���

    unsigned short memberCount;	//���������������������� 2016-05-12 by zjc
};

enum TASK_RESULT
{
	TASK_RESULT_OP_PRINTF_FREE = 0x01, 
	TASK_RESULT_OP_LOOP = 0x11,
	TASK_RESULT_OP_LOOP_FREE = 0x12,
	TASK_RESULT_OP_FAIL_FREE = 0xff
};


enum TASKSTEP
{
	TASKSTEP_INIT = 0,
		
	TASKSTEP_START = 0xe001,
	TASKSTEP_QRY_TICKET = 0xe002,  //Ʊ����ѯ
	TASKSTEP_PMFLAG = 0xe003,   //���Ѻ���־
	TASKSTEP_SOCKET = 0xe004, //���Ӵ�������
	TASKSTEP_QRY_RENT = 0xe005, //�⻹����Ϣ
	TASKSTEP_DEPOSIT = 0xe006, //Ѻ����
	TASKSTEP_OUT_OF_SERVICE = 0xe007, //��ͣ����
	
	TASKSTEP_ON_STAGE1TO2 = 0xe015, //1��2�׶�
	TASKSTEP_ON_CONSUME = 0xe016,  //���ѿ�Ѻ��
	TASKSTEP_ON_STAGE3TO4 = 0xe017, //3��4�׶�
	TASKSTEP_ON_STAGE5TO6 = 0xe018, //5��6�׶�
	TASKSTEP_ON_SET_RENT = 0xe019, //�����⻹����Ϣ
	TASKSTEP_ON_STAGE7TO8 = 0xe01a, //7��8�׶�

	TASKSTEP_OFF_STAGE1TO2 = 0xe025, //1��2�׶�
	TASKSTEP_OFF_STAGE3TO4 = 0xe026, //3��4�׶�
	TASKSTEP_OFF_STAGE5TO6= 0xe027, //5��6�׶�
	TASKSTEP_OFF_STAGE7TO8 = 0xe028,  //7��8�׶�
	TASKSTEP_OFF_STAGE9TO10 = 0xe029, //9��10�׶�
	TASKSTEP_OFF_STAGE11TO12 = 0xe02a, //11��12�׶�
	TASKSTEP_OFF_STAGE13TO14 = 0xe02b, //13��14�׶�
	TASKSTEP_OFF_STAGE15TO16 = 0xe02c, //15��16�׶�
	TASKSTEP_OFF_STAGE17TO18 = 0xe02d, //17��18�׶�
	TASKSTEP_OFF_STAGE19TO20 = 0xe02e, //19��20�׶�
	TASKSTEP_OFF_STAGE21TO22 = 0xe02f, //21��22�׶�
	TASKSTEP_OFF_STAGE23TO24 = 0xe030, //23��24�׶�


	/************************��ֵ�׶�*************************/
	TASKSTEP_RECHARGE_STAGE1TO2 = 0xe035, //1��2�׶�
	TASKSTEP_RECHARGE_STAGE3TO4 = 0xe036, //3��4�׶�
	TASKSTEP_RECHARGE_STAGE5TO6= 0xe037, //5��6�׶�
	TASKSTEP_RECHARGE_STAGE7TO8 = 0xe038,  //7��8�׶�
	TASKSTEP_RECHARGE_STAGE9TO10 = 0xe039, //9��10�׶�
	TASKSTEP_RECHARGE_STAGE11TO12 = 0xe03a, //11��12�׶�
	TASKSTEP_RECHARGE_STAGE13TO14 = 0xe03b, //13��14�׶�
	TASKSTEP_RECHARGE_STAGE15TO16 = 0xe03c, //15��16�׶�
	TASKSTEP_RECHARGE_STAGE17TO18 = 0xe03d, //17��18�׶�

	
	
	TASKSTEP_END
};



struct TaskData
{
        int serviceType;                        //��������
	unsigned char id_card[36];		//���֤����
        unsigned char phone_number[6];          //�ֻ�����
	unsigned char member_type;	        //��Ա����
        unsigned short member_typeCount;	//�Ρ��졢�¡����� 2016-04-14
        unsigned char order_number[10];	        //��ֵ������

        unsigned char confimeCnt; //����ȷ�ϼ��Ĵ���
};

class OnOffCardTask : public QObject, public QRunnable
{
	Q_OBJECT
public:
	OnOffCardTask(struct TaskData *data);

signals:
	void signals_Result(struct TaskResult *result);


protected:
    	void run();

private:
	void emit_TaskRunStep(enum TaskRunStep step);

private:
	struct TaskData taskdata;
	unsigned int taskStep;
	unsigned int f_operate;
	time_t begin_times, end_times;
};




#endif

