#ifndef __SAE_STATUS_DLG_H__
#define __SAE_STATUS_DLG_H__


#include <QDialog>
#include <QTimer>

#include "ui_sae_status_ui.h"
#include "lib_gui.h"


struct saeStatus
{
	unsigned char id;
	gui_sae_status_t stat;
};


class SaeStatusDlg : public QDialog, public Ui::SaeStatClass
{
	Q_OBJECT

public:
	SaeStatusDlg(QWidget *parent = 0);
	~SaeStatusDlg(void);

signals:
	void signals_saeStatAccept(void);
	void signals_saeStatReject(void);

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
	struct saeStatus sstat[65];
	struct gui_stake_info_page_config sPageConfig;
};





#endif

