#ifndef __TICKET_INFO_QUERY_DLG_H__
#define __TICKET_INFO_QUERY_DLG_H__

#include <QDialog>
#include <QTimer>
#include "ui_ticket_info_query_ui.h"
#include "lib_lnt.h"



class TicketInfoQueryDlg : public QDialog, public Ui::TicketInfoQryClass
{
	Q_OBJECT	

public:
	TicketInfoQueryDlg(QWidget *parent = 0);
	~TicketInfoQueryDlg(void);


private slots:
	void slots_timerDone(void);

signals:
	void signals_manageAccept(void);
	void signals_manageReject(void);

	
private:
	void __allWidgetClear(void);
	int __getPageConfig(void);
	void __showConfigResult(const unsigned char result);
	void keyPressEvent(QKeyEvent *e);
	

private:
	int timerCount;
	int timerFlag;
	QTimer timer;
	int m_ret;
	unsigned char m_stat;
	lib_lnt_qry_ticket_info_ack_t lnt_gry_ticket;
	
};









#endif

