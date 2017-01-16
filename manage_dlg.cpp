#include <QtGui>
#include <QDebug>
#include "manage_dlg.h"


ManageDlg::ManageDlg(QWidget *parent) : QDialog(parent)
{
	setupUi(this);
	setWindowFlags(Qt::FramelessWindowHint);	




	
	//connect(this, SIGNAL(signals_manageQuit()), this, SLOT(close()));  //关闭manage dialog界面

	connect(this, SIGNAL(signals_manageQuit()), this, SLOT(accept()));
	
	connect(this, SIGNAL(signals_entryBasicInfoDlg()), this, SLOT(slots_entryBasicInfoDlg()));  //基本信息界面
	connect(this, SIGNAL(signals_entryNdevConfigDlg()), this, SLOT(slots_entryNdevConfigDlg()));  //节点机配置界面
	connect(this, SIGNAL(signals_entrySaeRegisterDlg()), this, SLOT(slots_entrySaeRegisterDlg()));  //锁桩签到界面
	connect(this, SIGNAL(signals_entrySaeInfoDlg()), this, SLOT(slots_entrySaeInfoDlg()));  //锁桩信息界面
	connect(this, SIGNAL(signals_entrySaeStatusDlg()), this, SLOT(slots_entrySaeStatusDlg()));  //锁桩状态界面
	connect(this, SIGNAL(signals_entryWirelessDlg()), this, SLOT(slots_entryWirelessDlg()));   //无线信息界面
	connect(this, SIGNAL(signals_entryDebugToolsDlg()), this, SLOT(slots_entryDebugToolsDlg()));   //调试工具界面
	connect(this, SIGNAL(signals_entryDeviceInfoDlg()), this, SLOT(slots_entryDeviceInfoDlg()));   //设备信息界面
}

ManageDlg::~ManageDlg(void)
{
	
}






void ManageDlg::slots_entryBasicInfoDlg(void)
{
	m_pBasicInfoDlg = new BasicInfoDlg;
	m_pBasicInfoDlg->exec();

	if(m_pBasicInfoDlg != NULL)
	{
		delete m_pBasicInfoDlg;
		m_pBasicInfoDlg = NULL;
	}
}

void ManageDlg::slots_entryNdevConfigDlg(void)
{
	m_pNdevConfigDlg = new NdevConfigDlg;
	m_pNdevConfigDlg->exec();

	if(m_pNdevConfigDlg != NULL)
	{
		delete m_pNdevConfigDlg;
		m_pNdevConfigDlg = NULL;
	}	
}

void ManageDlg::slots_entrySaeRegisterDlg(void)
{
	m_pSaeRegisterDlg = new SaeRegisterDlg;
	m_pSaeRegisterDlg->exec();

	if(m_pSaeRegisterDlg != NULL)
	{
		delete m_pSaeRegisterDlg;
		m_pSaeRegisterDlg = NULL;
	}		
}

void ManageDlg::slots_entrySaeInfoDlg(void)
{
	m_pSaeInfoDlg = new SaeInfoDlg;
	m_pSaeInfoDlg->exec();

	if(m_pSaeInfoDlg != NULL)
	{
		delete m_pSaeInfoDlg;
		m_pSaeInfoDlg = NULL;
	}		
}

void ManageDlg::slots_entrySaeStatusDlg(void)
{
	m_pSaeStatusDlg = new SaeStatusDlg;
	m_pSaeStatusDlg->exec();

	if(m_pSaeStatusDlg != NULL)
	{
		delete m_pSaeStatusDlg;
		m_pSaeStatusDlg = NULL;
	}		
}

void ManageDlg::slots_entryWirelessDlg(void)
{
	m_pWirelessDlg = new WirelessDlg;
	m_pWirelessDlg->exec();

	if(m_pWirelessDlg != NULL)
	{
		delete m_pWirelessDlg;
		m_pWirelessDlg = NULL;
	}		
}

void ManageDlg::slots_entryDebugToolsDlg(void)
{
	m_pDebugToolsDlg = new DebugToolsDlg;
	m_pDebugToolsDlg->exec();

	if(m_pDebugToolsDlg != NULL)
	{
		delete m_pDebugToolsDlg;
		m_pDebugToolsDlg = NULL;
	}		
}

void ManageDlg::slots_entryDeviceInfoDlg(void)
{
	m_pDeviceInfoDlg = new DeviceInfoDlg;
	m_pDeviceInfoDlg->exec();

	if(m_pDeviceInfoDlg != NULL)
	{
		delete m_pDeviceInfoDlg;
		m_pDeviceInfoDlg = NULL;
	}		
}







void ManageDlg::keyPressEvent(QKeyEvent *e)
{
	if(e->key() == Qt::Key_Escape)  
		emit signals_manageQuit();

	if(e->key() == Qt::Key_0)
		emit signals_entryBasicInfoDlg();

	if(e->key() == Qt::Key_1)
		emit signals_entryNdevConfigDlg();

	if(e->key() == Qt::Key_2)
		emit signals_entrySaeRegisterDlg();

	if(e->key() == Qt::Key_3)
		emit signals_entrySaeInfoDlg();

	if(e->key() == Qt::Key_4)
		emit signals_entrySaeStatusDlg();
	
	if(e->key() == Qt::Key_5)
		emit signals_entryWirelessDlg();

	if(e->key() == Qt::Key_6)
		emit signals_entryDebugToolsDlg();

	if(e->key() == Qt::Key_7)
		emit signals_entryDeviceInfoDlg();
	
		
	return;
}










