#ifndef __WIRELESS_DLG_H__
#define __WIRELESS_DLG_H__


#include <QDialog>
#include <QTimer>
#include "ui_wireless_ui.h"
#include "lib_gui.h"
#include "lib_wireless.h"

class WirelessDlg : public QDialog, public Ui::WirelessClass
{
	Q_OBJECT	

public:
	WirelessDlg(QWidget *parent = 0);
	~WirelessDlg(void);

private slots:
	void slots_manualRefresh(void);
	bool eventFilter(QObject *watched, QEvent *event);

private:
	int retval;
	unsigned int wireless_dial_stat;
	struct gui_ndev_access_pattern_config access_config;
	struct lib_wl_cgmr wl_cgmr;
	struct lib_wl_hwver wl_hwver;
	struct lib_wl_pppd_info wl_pppd_info;
	struct lib_wl_csq_sysinfo wl_csq_sysinfo;
	struct lib_wl_dial_time wl_dial_time;
        struct gui_gps_info gps_info;
	
	void __showConfigResult(const unsigned char result);
	void __allWidgetClear(void);
	int __getPageConfig(void);
	void __setPageConfig(void);

};

#endif


