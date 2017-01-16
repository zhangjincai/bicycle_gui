#ifndef __DEVICE_INFO_DLG_H__
#define __DEVICE_INFO_DLG_H__


#include <QDialog>
#include "ui_device_info_ui.h"
#include "lib_lnt.h"
#include "lib_gui.h"


class DeviceInfoDlg : public QDialog, public Ui::DeviceInfoClass
{
	Q_OBJECT	

public:
	DeviceInfoDlg(QWidget *parent = 0);
	~DeviceInfoDlg(void);




private slots:
	void slots_manualRefresh(void);
	bool eventFilter(QObject *watched, QEvent *event);
		

signals:

private:
	void __allWidgetClear(void);
	int __getPageConfig(void);
	void __setPageConfig(void);
	void __showConfigResult(const unsigned char result);
	

	
private:
	int m_ret;
	unsigned char m_stat;
	lib_lnt_getversion_ack_t lnt_ver_ack;
	struct gui_ndev_config ndev_config;
	lib_lnt_config_t lnt_config;
	gui_iboard_device_version_t iboard_version;

	//gui_net_info_backup_t net_info;

};



#endif


