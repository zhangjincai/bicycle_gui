#ifndef __INFO_QUERY_DLG_H__
#define __INFO_QUERY_DLG_H__


#include <QDialog>
#include <QTimer>
#include "ui_info_query_ui.h"
#include "ticket_info_query_dlg.h"
#include "member_info_query_dlg.h"
#include "rent_info_query_dlg.h"
#include "nearby_site_info_query_dlg.h"



class InfoQueryDlg : public QDialog, public Ui::InfoQryClass
{
	Q_OBJECT	

public:
	InfoQueryDlg(QWidget *parent = 0);
	~InfoQueryDlg(void);




private slots:
	void slots_timerDone(void);
	void slots_entryTicketInfoQryDlg(void);
	void slots_enrtyMemberInfoQry(void);
	void slots_entryRentInfoQryDlg(void);
	
    void slots_entryNearbySiteInfoQryDlg(void);

		

signals:
	void signals_infoQueryQuit(void);
	void signals_entryTicketInfoQryDlg(void);
	void signals_entryMemberInfoQryDlg(void);
	void signals_entryRentInfoQryDlg(void);
	
	void signals_entryNearbySiteInfoQryDlg(void);


private:
	int timerCount;
	QTimer timer;
	TicketInfoQueryDlg *m_pTicketInfoQryDlg;
	MemberInfoQueryDlg *m_pMemberInfoQryDlg;
	RentInfoQueryDlg *m_pRentInfoQryDlg;
	
	NearbySiteInfoQueryDlg *m_pNearbySiteInfoQryDlg;
	void keyPressEvent(QKeyEvent *e);
};
















#endif


