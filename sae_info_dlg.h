#ifndef __SAE_INFO_DLG_H__
#define __SAE_INFO_DLG_H__


#include <QDialog>
#include <QTimer>

#include "ui_sae_info_ui.h"
#include "lib_gui.h"



class SaeInfoDlg : public QDialog, public Ui::SaeInfoClass
{
	Q_OBJECT

public:
	SaeInfoDlg(QWidget *parent = 0);
	~SaeInfoDlg(void);

	
signals:
	void signals_saeInfoAccept(void);
	void signals_saeInfoReject(void);

private:
	void tableWidgetSet(void);
	int __getPageConfig(void);
	void __setPageConfig(void);

	
private slots:
	void slots_manualRefresh(void);
	void slots_loopBrowse(void);
	void keyPressEvent(QKeyEvent *e);
	bool eventFilter(QObject *watched, QEvent *event);
	
private:
	int retval;
	unsigned char mTotal, mRegister;
	int mPages, mDiv, mRema, mShowPage;
	struct gui_stake_all_info_page_config allInfoConfig;	
	struct gui_sae_info saeInfo[65];		
};





#endif

