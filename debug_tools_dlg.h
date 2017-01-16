#ifndef __DEBUG_TOOLS_DLG_H__
#define __DEBUG_TOOLS_DLG_H__


#include <QDialog>
#include "ui_debug_tools_ui.h"




class DebugToolsDlg : public QDialog, public Ui::DebugToolsClass
{
	Q_OBJECT	

public:
	DebugToolsDlg(QWidget *parent = 0);
	~DebugToolsDlg(void);


private slots:
	void slots_deviceReboot(void);
	void slots_SDCopy(void);
	void slots_SDTradeRecordCopy(void);
	void slots_UnityFileCopyIn(void);
	void slots_UnityFileCopyOut(void);
	bool eventFilter(QObject *watched, QEvent *event);

	
signals:
	

private:
	void __allWidgetClear(void);
	void __showConfigResult(const unsigned char result);
	void __controlSettings(const int ctrl);

	
	int sdcopy_f, sdtradecopy_f;
	int unityFileCopyIn_f, unityFileCopyOut_f;
		
private:
	
	
};









#endif


