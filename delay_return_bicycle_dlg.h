#ifndef __ON_OFF_CARD_DLG_H__
#define __ON_OFF_CARD_SERVICE_DLG_H__

#include <QDialog>
#include <QThread>
#include <QTimer>
#include "ui_delay_return_bicycle_ui.h"
#include "lib_lnt.h"
#include "lib_gui.h"

#define ECS_QUIT_ON			1
#define ECS_QUIT_OFF		2


class DelayReturnBicycleDlg : public QDialog, public Ui::DelayReturnBicycleClass
{
	Q_OBJECT	

public:
	DelayReturnBicycleDlg(QWidget *parent = 0);
	~DelayReturnBicycleDlg(void);
	void __allWidgetClear(void);
	void __rent_flag_to_bit(struct rent_info *rinfo, struct rent_flag *flag);
	int __getPageConfig(void);
	void do_delay_return_bicycle(void);

	void keyPressEvent(QKeyEvent *e);
	bool eventFilter(QObject *watched, QEvent *event);


public slots:
	void slots_DelayReturnBicycleStart(void);
	void slots_timerDone(void);
	
signals:
	void signals_delayAccept(void);
	void signals_delayReject(void);
	//void signals_DelayReturnBicycleQuit(void);

private:
	int retError;
        int timerFlag;
        int timerCount;
	int delay_flag;
        QTimer timer;
	QString infoString;	
	int m_ret;
	int retval;
	unsigned char SAVE_pid[8];
	unsigned char m_stat;
	enum LNT_ADMIN_CARD_TYPE adminCardType;

	struct gui_stake_info_page_config sPageConfig;
	gui_sae_comparison_status_t comp[65];
	lib_lnt_qry_ticket_info_ack_t lnt_qry_ticket;
	lib_lnt_rent_info_req_t lnt_rent_req;
	lib_lnt_rent_info_ack_t lnt_rent_ack;
	lib_lnt_rent_info_t lnt_rent;
	lib_lnt_consume_req_t lib_lnt_consume_req;
	lib_lnt_trade_record_ack_t lib_lnt_trade_record_ack;
	unsigned char f_type;
	struct rent_flag rf;
	
};

#endif

