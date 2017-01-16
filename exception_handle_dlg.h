#ifndef __EXCEPTION_HANDLE_DLG_H__
#define __EXCEPTION_HANDLE_DLG_H__


#include <QDialog>
#include <QTimer>
#include "ui_exception_handle_ui.h"
#include "thread_task_handle.h"


#define ECS_QUIT_ON		1
#define ECS_QUIT_OFF		2

class ExceptionHandleDlg : public QDialog, public Ui::ExceptHandleClass
{
	Q_OBJECT	

public:
	ExceptionHandleDlg(QWidget *parent = 0);
	~ExceptionHandleDlg(void);


private slots:
	void slots_timerDone(void);
	void slots_ThreadTaskResult(struct ThreadTaskResult *threadResult);

signals:
	void signals_exceptionAccept(void);
	void signals_exceptionReject(void);
	
private:
	void keyPressEvent(QKeyEvent *e);

private:
	int retError;
	int esc_quit_key;
	int timerCount;
	QTimer timer;
	QString infoString;
	struct ThreadTaskResult m_ThreadTaskResult;
};


#endif



