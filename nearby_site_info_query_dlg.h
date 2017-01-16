#ifndef __NEARBY_SITE_INFO_QUERY_DLG_H__
#define __NEARBY_SITE_INFO_QUERY_DLG_H__


#include <QDialog>
#include <QTimer>
#include <time.h>
#include "ui_nearby_site_info_query_ui.h"
#include "lib_lnt.h"
#include "lib_gui.h"
#include "gui_header.h"

#define NEARBY_SITE_INFO_FMT_MAX			(6)

class NearbySiteInfoQueryDlg : public QDialog, public Ui::NearbySiteInfoQryClass
{
	Q_OBJECT	

public:
        NearbySiteInfoQueryDlg(QWidget *parent = 0);
        ~NearbySiteInfoQueryDlg(void);


private slots:
	void slots_timerDone(void);

signals:
        void signals_nearbySiteInfoAccept(void);
        void signals_nearbySiteInfoReject(void);

	
private:
	void __allWidgetClear(void);
	int __getPageConfig(void);
	void __showConfigResult(const unsigned char result);
        int __parseNearbySiteInfoQry(gui_nearby_site_info_qry_ack_t *nearby_site_info_qry, gui_nearby_site_info_fmt_t nearby_site_info_fmt[NEARBY_SITE_INFO_FMT_MAX]);
        int __explainNearbySiteInfoAtoi(char *dest, char *src);
        int __setNearbySiteInfo(void);
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
        gui_nearby_site_info_fmt_t nearby_site_info_fmt[NEARBY_SITE_INFO_FMT_MAX];
        gui_nearby_site_info_qry_req_t nearby_site_info_qry_req;
        gui_nearby_site_info_qry_ack_t nearby_site_info_qry_ack;
		struct gui_basic_info_page_config bPageConfig;
};










#endif

