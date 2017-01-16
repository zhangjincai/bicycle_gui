#ifndef __MEMBER_SERVICE_DLG_H__
#define __MEMBER_SERVICE_DLG_H__



#include <QDialog>
#include <QTimer>
#include "ui_member_service_ui.h"
#include "on_off_card_service_dlg.h"
#include "delay_return_bicycle_dlg.h"
#include "recharge_service_dlg.h"  //2016-04-12 add by zjc

#include "member_on_card_service_dlg.h"



class MemberServiceDlg : public QDialog, public Ui::MemberServiceClass
{
	Q_OBJECT	

public:
	MemberServiceDlg(QWidget *parent = 0);
	~MemberServiceDlg(void);


private slots:
	void slots_timerDone(void);
        void slots_entryOnOffCardServiceDlg(void);
        void slots_entryMemberOnCardServiceDlg(void);//
	void slots_entrydelayReturnBicyleDlg(void);

        void slots_entryRechargeServiceDlg(void); //2016-04-12 add by zjc
	
signals:
	void signals_OnCardServiceQuit(void);

        void signals_entryMemberOnCardServiceDlg(void); //会员开卡服务

	void signals_entryOffCardServiceDlg(void);	
	void signals_delayReturnBicycleDlg(void);	

        void signals_entryRechargeServiceDlg(void); //2016-04-12 add by zjc



private:
	int timerCount;
	QTimer timer;
	OnOffCardServiceDlg *m_pOnOffCardServieDlg;
	DelayReturnBicycleDlg *m_pDelayReturnBicyleDlg;

        RechargeServiceDlg *m_pRechargeServieDlg; //2016-04-12 add by zjc

        MemberOnCardServiceDlg *m_pMemberOnCardServiceDlg;

	void keyPressEvent(QKeyEvent *e);
};




#endif

