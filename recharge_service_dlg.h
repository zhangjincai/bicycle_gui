#ifndef __RECHARGE_DLG_H__
#define __RECHARGE_SERVICE_DLG_H__

#include <QDialog>
#include <QThread>
#include <QTimer>
#include "ui_recharge_service_ui.h"
#include "on_off_card_task.h"
#include "lib_lnt.h"
#include "lib_gui.h"


class RechargeServiceDlg : public QDialog, public Ui::RechargeServiceClass
{
	Q_OBJECT	

public:
	RechargeServiceDlg(QWidget *parent = 0);
	~RechargeServiceDlg(void);


public slots:
	void slots_RechargeStart(void);
	void slots_TaskResult(struct TaskResult *result);
	void slots_timerDone(void);
	
signals:
	void signals_RechargeQuit(void);

private:
        unsigned char confirmCnt;//°´ÏÂÖ´ÐÐ³äÖµ²Ù×÷µÄ´ÎÊý

	int retError;
	int timerCount;
	int ptn_start_f;
	int esc_quit_key_f;
	QTimer timer;
	QString infoString;
	struct TaskResult taskResult;
	int checkEnvironment(void);
	void keyPressEvent(QKeyEvent *e);
	bool eventFilter(QObject *watched, QEvent *event);
};


#endif

