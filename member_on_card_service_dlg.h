#ifndef __MEMBER_ON_CARD_SERVICE_DLG_H__
#define __MEMBER_ON_CARD_SERVICE_DLG_H__

#include <QDialog>
#include <QThread>
#include <QTimer>
#include "ui_on_off_card_service_ui.h"
#include "on_off_card_task.h"
#include "lib_lnt.h"
#include "lib_gui.h"

#include "ui_member_on_card_service_ui.h"   //
#include "on_off_card_service_dlg.h"

class MemberOnCardServiceDlg : public QDialog, public Ui::MemberOnCardServiceClass
{
	Q_OBJECT	

public:
        MemberOnCardServiceDlg(QWidget *parent = 0);
        ~MemberOnCardServiceDlg(void);

private slots:
        void slots_timerDone(void);

        void slots_entryOnOffCardServiceDlg(void);

        //void slots_memberTypeChange(int memberTypeCheck);


signals:
        void signals_OnCardServiceQuit(void);

        void signals_entryOnOffCardServiceDlg(void);




private:
        int timerCount;
        QTimer timer;

        OnOffCardServiceDlg *m_pOnOffCardServieDlg;


        void keyPressEvent(QKeyEvent *e);
};




#endif

