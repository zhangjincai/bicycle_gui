#ifndef __RENT_INFO_QUERY_DLG_H__
#define __RENT_INFO_QUERY_DLG_H__


#include <QDialog>
#include <QTimer>
#include <time.h>
#include "ui_rent_info_query_ui.h"
#include "lib_lnt.h"
#include "lib_gui.h"

#define RENT_INFO_FMT_MAX			(10)

class RentInfoQueryDlg : public QDialog, public Ui::RentInfoQryClass
{
	Q_OBJECT	

public:
	RentInfoQueryDlg(QWidget *parent = 0);
	~RentInfoQueryDlg(void);


private slots:
	void slots_timerDone(void);

signals:
	void signals_rentInfoAccept(void);
	void signals_rentInfoReject(void);

	
private:
	void __allWidgetClear(void);
	int __getPageConfig(void);
	void __showConfigResult(const unsigned char result);
	int __parseRentInfoQry(gui_rent_info_qry_ack_t *rent_info_qry, gui_rent_info_fmt_t rent_info_fmt[RENT_INFO_FMT_MAX]);
	int __explainRentInfoTime(char *dest, char *src);
	int __explainRentInfoFee(char *dest, char *src);
	int __setRentInfo(void);
	void keyPressEvent(QKeyEvent *e);
	

private:
	int timerCount;
	int timerFlag;
	int qry_ticket_f; 
	int esc_quit_key;
	QTimer timer;
	int m_ret;
	int handleReslut;
	unsigned char m_stat;
	time_t now_time_sec;
	gui_rent_info_fmt_t rent_info_fmt[RENT_INFO_FMT_MAX];
	gui_rent_info_qry_req_t rinfo_qry_req;
	gui_rent_info_qry_ack_t rinfo_qry_ack;
};










#endif

