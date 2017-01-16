#ifndef __MANAGE_DLG_H__
#define __MANAGE_DLG_H__


#include <QDialog>
#include "ui_manage_ui.h"
#include "basic_info_dlg.h"
#include "ndev_config_dlg.h"
#include "sae_register_dlg.h"
#include "sae_info_dlg.h"
#include "sae_status_dlg.h"
#include "wireless_dlg.h"
#include "debug_tools_dlg.h"
#include "device_info_dlg.h"




class ManageDlg : public QDialog, public Ui::ManageClass
{
	Q_OBJECT	

public:
	ManageDlg(QWidget *parent = 0);
	~ManageDlg(void);




private slots:
	void slots_entryBasicInfoDlg(void);
	void slots_entryNdevConfigDlg(void);
	void slots_entrySaeRegisterDlg(void);
	void slots_entrySaeInfoDlg(void);
	void slots_entrySaeStatusDlg(void);
	void slots_entryWirelessDlg(void);
	void slots_entryDebugToolsDlg(void);
	void slots_entryDeviceInfoDlg(void);

	

signals:
	void signals_manageQuit(void);
	void signals_entryBasicInfoDlg(void);
	void signals_entryNdevConfigDlg(void);
	void signals_entrySaeRegisterDlg(void);
	void signals_entrySaeInfoDlg(void);
	void signals_entrySaeStatusDlg(void);
	void signals_entryWirelessDlg(void);
	void signals_entryDebugToolsDlg(void);
	void signals_entryDeviceInfoDlg(void);

	

private:
	BasicInfoDlg *m_pBasicInfoDlg;
	NdevConfigDlg *m_pNdevConfigDlg;
	SaeRegisterDlg *m_pSaeRegisterDlg;
	SaeInfoDlg *m_pSaeInfoDlg;
	SaeStatusDlg *m_pSaeStatusDlg;
	WirelessDlg *m_pWirelessDlg;
	DebugToolsDlg *m_pDebugToolsDlg;
	DeviceInfoDlg *m_pDeviceInfoDlg;
	void keyPressEvent(QKeyEvent *e);
};
















#endif


