#ifndef __ON_OFF_CARD_SERVICE_DLG_H__
#define __ON_OFF_CARD_SERVICE_DLG_H__

#include <QDialog>
#include <QThread>
#include <QTimer>
#include "ui_on_off_card_service_ui.h"
#include "on_off_card_task.h"
#include "lib_lnt.h"
#include "lib_gui.h"



class OnOffCardServiceDlg : public QDialog, public Ui::OnOffCardServiceClass
{
	Q_OBJECT	

public:
	OnOffCardServiceDlg(QWidget *parent = 0);
	~OnOffCardServiceDlg(void);


public slots:
	void slots_OnOffCardStart(void);
	void slots_TaskResult(struct TaskResult *result);
	void slots_timerDone(void);




	
signals:
	void signals_OnOffCardQuit(void);



private:
        unsigned char confirmCnt;//开销卡过程中按下确认键的次数
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

