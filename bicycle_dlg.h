#ifndef __BICYCLE_DLG_H__
#define __BICYCLE_DLG_H__

#include <QDialog>
#include <QTimer>
#include <QMovie>

#include "ui_bicycle_ui.h"
#include "manage_dlg.h"
#include "info_query_dlg.h"
#include "operation_guide_dlg.h"
#include "exception_handle_dlg.h"
#include "member_service_dlg.h"
#include "lib_gui.h"

#include "myqrencode.h"


class BicycleDlg : public QDialog, public Ui::BicycleClass
{
	Q_OBJECT	

public:
	BicycleDlg(QWidget *parent = 0);
	~BicycleDlg(void);



private slots:
	void slots_entryMangageDlg(void);
	void slots_entryInfoQueryDlg(void);
	void slots_entryOperationGuideDlg(void);
	void slots_entryExcepionHandleDlg(void);
	void slots_entryMemberServiceDlg(void);
	void slots_timerDone(void);
    void slots_timer1Done(void);

	

signals:
	void signals_entryMangageDlg(void);
	void signals_entryInfoQueryDlg(void);
	void signals_entryOperationGuideDlg(void);
	void signals_entryExcepionHandleDlg(void);
	void signals_entryMemberServiceDlg(void);

	

private:
	ManageDlg *m_pManageDlg;
	InfoQueryDlg *m_pInfoQryDlg;
	OperationGuideDlg *m_pOperGuideDlg;
	ExceptionHandleDlg *m_pExcepHndDlg;
	MemberServiceDlg *m_pMemServDlg;
	int retval, QR_code_flag;	//网点名称变化标志
        QTimer timer, timer1; //timer1用于“羊城通充值“提示
	unsigned int timerCount;
        unsigned int warnTimeCount, warnTimeCount1;
	unsigned int gifTimeCount;
	unsigned int lntTimeCount;
	unsigned int netCount; //用于周期性备份网卡信息
	unsigned int net_info_read_flag; //读取备份网卡信息标志 0:失败 1:成功
	struct gui_ndev_home_page_info home_page_info;
	int gif_current_frame;
	struct gui_lnt_all_info lntAllInfo;
	enum LNT_ADMIN_CARD_TYPE lntAdminCardType;

	unsigned char site_QR_code[128];   //站点二维码备份 //32->64 11--2016-11-25

	
	QMovie *p_movie;   
	
	void __allWidgetClear(void);
	int __getPageConfig(const unsigned int stage);
	void __setPageConfig(void);
	
	void keyPressEvent(QKeyEvent *e);

        MyQrenCode *myQrcode;

};























#endif


