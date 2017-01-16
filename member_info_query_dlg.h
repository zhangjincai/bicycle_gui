#ifndef __MEMBER_INFO_QUERY_H__
#define __MEMBER_INFO_QUERY_H__


#include <QDialog>
#include <QTimer>
#include "ui_member_info_query_ui.h"
#include "lib_lnt.h"



class MemberInfoQueryDlg : public QDialog, public Ui::MemberInfoQryClass
{
	Q_OBJECT	

public:
	MemberInfoQueryDlg(QWidget *parent = 0);
	~MemberInfoQueryDlg(void);


private slots:
	void slots_timerDone(void);

signals:
	void signals_manageAccept(void);
	void signals_manageReject(void);

	
private:
	void __rent_flag_to_bit(struct rent_info *rinfo, struct rent_flag *flag);
	void __allWidgetClear(void);
	int __getPageConfig(void);
	void __showConfigResult(const unsigned char result);
	void keyPressEvent(QKeyEvent *e);
	

private:
	int timerCount;
	int timerFlag;
	QTimer timer;
	int m_ret;
	unsigned char m_stat;
	 enum LNT_ADMIN_CARD_TYPE adminCardType;
	
};





#endif

