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

        CARD_SERVICE_ON = 1,        //开卡
        CARD_SERVICE_OFF = 2,       //销卡
        CARD_SERVICE_RECHARGE = 3,  //充值 2016-04-07 by zjc

        CARD_SERVICE_ON_NORMAL = 4, //普通卡
        CARD_SERVICE_ON_TIME = 5,   //次卡
        CARD_SERVICE_ON_DAY = 6,    //天卡
        CARD_SERVICE_ON_MONTH = 7,  //月卡
        CARD_SERVICE_ON_YEAR = 8,   //年卡
};

/*
 * 任务运行步骤
 */
enum TaskRunStep
{
	TaskRunStep_Input_ID = 1,          //输入身份证号码和手机号码
	TaskRunStep_Deposit = 2,           //扣除押金
	TaskRunStep_Get_OrderNO = 3,  //获取订单号
	TaskRunStep_Modify_Member_Flag = 4,  //修改会员标志
	TaskRunStep_Recharge = 5,   //充值
	TaskRunStep_Finish = 6,   //操作完成


	TaskRunStep_p0 = 10,
	TaskRunStep_p1 = 11,
	TaskRunStep_p2 = 12,
	TaskRunStep_p3 = 13,
	TaskRunStep_p4 = 14,
	TaskRunStep_p5 = 15,
	TaskRunStep_p6 = 16
};

/*
 * 任务运行状态
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
	int type;  //类型

	unsigned int step;      //步骤
	unsigned int result;   //结果
	char s_info[128];  //信息

	int serviceType;  	//服务类型
	int taskRunStat;  //运行状态
	int taskRunStep;   //任务运行步骤

    unsigned short memberCount;	//次数、天数、月数、年数 2016-05-12 by zjc
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
	TASKSTEP_QRY_TICKET = 0xe002,  //票卡查询
	TASKSTEP_PMFLAG = 0xe003,   //检查押金标志
	TASKSTEP_SOCKET = 0xe004, //连接代理网关
	TASKSTEP_QRY_RENT = 0xe005, //租还车信息
	TASKSTEP_DEPOSIT = 0xe006, //押金处理
	TASKSTEP_OUT_OF_SERVICE = 0xe007, //暂停服务
	
	TASKSTEP_ON_STAGE1TO2 = 0xe015, //1到2阶段
	TASKSTEP_ON_CONSUME = 0xe016,  //消费扣押金
	TASKSTEP_ON_STAGE3TO4 = 0xe017, //3到4阶段
	TASKSTEP_ON_STAGE5TO6 = 0xe018, //5到6阶段
	TASKSTEP_ON_SET_RENT = 0xe019, //设置租还车信息
	TASKSTEP_ON_STAGE7TO8 = 0xe01a, //7到8阶段

	TASKSTEP_OFF_STAGE1TO2 = 0xe025, //1到2阶段
	TASKSTEP_OFF_STAGE3TO4 = 0xe026, //3到4阶段
	TASKSTEP_OFF_STAGE5TO6= 0xe027, //5到6阶段
	TASKSTEP_OFF_STAGE7TO8 = 0xe028,  //7到8阶段
	TASKSTEP_OFF_STAGE9TO10 = 0xe029, //9到10阶段
	TASKSTEP_OFF_STAGE11TO12 = 0xe02a, //11到12阶段
	TASKSTEP_OFF_STAGE13TO14 = 0xe02b, //13到14阶段
	TASKSTEP_OFF_STAGE15TO16 = 0xe02c, //15到16阶段
	TASKSTEP_OFF_STAGE17TO18 = 0xe02d, //17到18阶段
	TASKSTEP_OFF_STAGE19TO20 = 0xe02e, //19到20阶段
	TASKSTEP_OFF_STAGE21TO22 = 0xe02f, //21到22阶段
	TASKSTEP_OFF_STAGE23TO24 = 0xe030, //23到24阶段


	/************************充值阶段*************************/
	TASKSTEP_RECHARGE_STAGE1TO2 = 0xe035, //1到2阶段
	TASKSTEP_RECHARGE_STAGE3TO4 = 0xe036, //3到4阶段
	TASKSTEP_RECHARGE_STAGE5TO6= 0xe037, //5到6阶段
	TASKSTEP_RECHARGE_STAGE7TO8 = 0xe038,  //7到8阶段
	TASKSTEP_RECHARGE_STAGE9TO10 = 0xe039, //9到10阶段
	TASKSTEP_RECHARGE_STAGE11TO12 = 0xe03a, //11到12阶段
	TASKSTEP_RECHARGE_STAGE13TO14 = 0xe03b, //13到14阶段
	TASKSTEP_RECHARGE_STAGE15TO16 = 0xe03c, //15到16阶段
	TASKSTEP_RECHARGE_STAGE17TO18 = 0xe03d, //17到18阶段

	
	
	TASKSTEP_END
};



struct TaskData
{
        int serviceType;                        //服务类型
	unsigned char id_card[36];		//身份证号码
        unsigned char phone_number[6];          //手机号码
	unsigned char member_type;	        //会员类型
        unsigned short member_typeCount;	//次、天、月、年数 2016-04-14
        unsigned char order_number[10];	        //充值订单号

        unsigned char confimeCnt; //按下确认键的次数
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

