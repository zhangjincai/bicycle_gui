#ifndef __BASIC_INFO_DLG_H__
#define __BASIC_INFO_DLG_H__

#include <QDialog>
#include <QTimer>
#include "ui_basic_info_ui.h"

#include "lib_gui.h"
#include "lib_wireless.h"

class BasicInfoDlg : public QDialog, public Ui::BasicInfoClass
{
	Q_OBJECT	

public:
	BasicInfoDlg(QWidget *parent = 0);
	~BasicInfoDlg(void);

private slots:
	void slots_manualRefresh(void);
	void slots_timerDone(void);
	bool eventFilter(QObject *watched, QEvent *event);
	
private:
	int retval;
	QTimer timer;
	int timerCount;
	bool refreshFlag;
	int unityFilePage;
	struct gui_basic_info_page_config bPageConfig;
	gui_unity_file_info_t bUnity_file_info;
		
	void __showConfigResult(const unsigned char result);
	void __allWidgetClear(void);
	int __getPageConfig(void);
	void __setPageConfig(void);



};













#endif


