#ifndef __SAE_REGISTER_DLG_H__
#define __SAE_REGISTER_DLG_H__

#include <QDialog>
#include <QTimer>

#include "ui_sae_register_ui.h"
#include "lib_gui.h"



class SaeRegisterDlg : public QDialog, public Ui::SaeRegisterClass
{
	Q_OBJECT	

public:
    SaeRegisterDlg(QWidget *parent = 0);
	~SaeRegisterDlg(void);

private slots:
	void slots_manualRefresh(void);
	void slots_timerDone(void);
	bool eventFilter(QObject *watched, QEvent *event);
	
private:
	int retval;
	QTimer timer;
    	QFont font;
	int timerCount;
	bool refreshFlag;
	gui_sae_comparison_status_t comp[65];
    	QPalette pa;
	struct gui_stake_info_page_config sPageConfig;
	int __getPageConfig(void);
	void __allWidgetClear(void);
	void __showConfigResult(const unsigned char result);
	void __showSaeStatistics(const unsigned int total, const unsigned int registered, const unsigned int not_registered, const unsigned int hasbike, const unsigned int hasnotbike);
	void __showTableNotRegister(const unsigned int idx);
    void __showTableHasNotBike(const unsigned int idx, char *fw, char *bl, char *para);
    void __showTableHasBike(const unsigned int idx, char *fw, char *bl, char *para);
	
};







#endif



