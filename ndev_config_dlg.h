#ifndef __NDEV_CONFIG_DLG_H__
#define __NDEV_CONFIG_DLG_H__

#include <QDialog>
#include "ui_ndev_config_ui.h"
#include "lib_gui.h"



class NdevConfigDlg : public QDialog, public Ui::NdevConfigClass
{
	Q_OBJECT	

public:
	NdevConfigDlg(QWidget *parent = 0);
	~NdevConfigDlg(void);

private slots:
	void slots_ndevConfigSetup(void);
	void slots_canBaudRate50kbpsCheckBox(int state);
	void slots_canBaudRate125kbpsCheckBox(int state);
	void slots_canBaudRate250kbpsCheckBox(int state);
	void slots_canBaudRate500kbpsCheckBox(int state);
	void slots_selectWirelessCheckBox(int state);
	void slots_selectWiredCheckBox(int state);
	bool eventFilter(QObject *watched, QEvent *event);
	
private:
	int retval;
	int canBaudRateCheck;
	int accessPatternCheck;
	struct gui_ndev_page_config nPageConfig;
	struct gui_light_ctrl_time ctrl_time, t_ctrl_time;


	void __allWidgetClear(void);
	void __showConfigResult(const unsigned char result);
	int __getPageConfig(void);
	void __setPageConfig(void);
	int __ftpSetConfig(void);
	int __centerSetConfig(void);
	int __wifiSetConfig(void);
	int __localSetConfig(void);
	int __nParameterSetConfig(void);
	int __sParameterSetConfig(void);
	int __accessPatternSetConfig(void);
	int __lntSetConfig(void);
	int __lightCtrlTimeSetConfig(void);
	void __controlSettings(const int ctrl);
	
	void keyPressEvent(QKeyEvent *e);
};














#endif



