#include <QtGui>
#include <QDebug>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>

#include "debug_tools_dlg.h"
#include "gui_header.h"
#include "lib_general.h"

extern char g_terminal_no[32];
extern int g_adminCardCtrlSet;

DebugToolsDlg::DebugToolsDlg(QWidget *parent) : QDialog(parent)
{
	setupUi(this);
	setWindowFlags(Qt::FramelessWindowHint);	

	__controlSettings(g_adminCardCtrlSet);

	__allWidgetClear();

	sdcopy_f = 1;
	sdtradecopy_f = 1;
	unityFileCopyIn_f = 1;
	unityFileCopyOut_f = 1;

	//pushButton_manualRefresh->setVisible(false);
	pushButton_manualRefresh->installEventFilter(this);
	pushButton_deviceReboot->installEventFilter(this);
	pushButton_copySDLog->installEventFilter(this);
	pushButton_unityFileCopyIn->installEventFilter(this);
	pushButton_unityFileCopyOut->installEventFilter(this);
	pushButton_copySDTradeRecord->installEventFilter(this);
	
	connect(pushButton_deviceReboot, SIGNAL(clicked()), this, SLOT(slots_deviceReboot()));
 	connect(pushButton_copySDLog, SIGNAL(clicked()), this, SLOT(slots_SDCopy()));
	connect(pushButton_copySDTradeRecord, SIGNAL(clicked()), this, SLOT(slots_SDTradeRecordCopy()));
	connect(pushButton_unityFileCopyIn, SIGNAL(clicked()), this, SLOT(slots_UnityFileCopyIn()));
	connect(pushButton_unityFileCopyOut, SIGNAL(clicked()), this, SLOT(slots_UnityFileCopyOut()));
}


DebugToolsDlg::~DebugToolsDlg(void)
{

}


#define	NET_INFO_BACKUP_PATH	"/opt/logpath/net_info.txt"
void DebugToolsDlg::slots_deviceReboot(void)
{
	system("/mnt/firmware/reboot_wdt");
	label_debugToolsShow->setText(tr("设备准备重启"));	
}

void DebugToolsDlg::slots_SDCopy(void)
{
	DIR * dp = NULL;
   	struct dirent *filename = NULL;
	bool sdflag = false;
	label_copySDString->clear();

	if(sdcopy_f == 2)
	{
		label_copySDString->setText(tr("数据已拷贝完成,请退出"));
		return;
	}
	
	dp = opendir("/media");
    	if (!dp)
    	{
           	label_copySDString->setText(tr("找不到 /media 目录"));
           	return;
    	}

 	while (filename = readdir(dp))
    	{
	           if(strcmp(filename->d_name,".") && strcmp(filename->d_name,".."))
	           {
		               sdflag = true;

		               time_t timep;
		               struct tm *p = NULL;
		               time(&timep);
		               p = localtime(&timep);

		               char cmd_str[256] = { 0 };

				/* umount */
				sprintf(cmd_str, "umount /media/%s", filename->d_name);
				system(cmd_str);

				/* mount */
				memset(cmd_str, 0, sizeof(cmd_str));
				sprintf(cmd_str, "mount /dev/%s	/media/%s", filename->d_name, filename->d_name);
				system(cmd_str);

				/* copy */
				memset(cmd_str, 0, sizeof(cmd_str));
		               sprintf(cmd_str,"cp /opt/logpath /media/%s/logpath_%s_%d-%02d-%02d_%02d-%02d-%02d -rf", filename->d_name, g_terminal_no, (1900+p->tm_year), (1+p->tm_mon), p->tm_mday, p->tm_hour, \
					   	p->tm_min, p->tm_sec);   
		               system(cmd_str);

				/* umout */
				memset(cmd_str, 0, sizeof(cmd_str));
				sprintf(cmd_str, "umount /media/%s", filename->d_name);
				system(cmd_str);	   
			
		               label_copySDString->setText(tr("数据拷贝完成"));

				sdcopy_f = 2;
	           }
    	}	

    	if(sdflag == false)
        	label_copySDString->setText(tr("SD未插入"));

	sdcopy_f = 2;
	 closedir(dp);
	 	
}

void DebugToolsDlg::slots_SDTradeRecordCopy(void)
{
	DIR * dp = NULL;
   	struct dirent *filename = NULL;
	bool sdflag = false;
	label_copySDTradeRecordString->clear();

	if(sdtradecopy_f == 2)
	{
		label_copySDTradeRecordString->setText(tr("交易数据已拷贝完成,请退出"));
		return;
	}
	
	dp = opendir("/media");
    	if (!dp)
    	{
           	label_copySDTradeRecordString->setText(tr("找不到 /media 目录"));
           	return;
    	}

 	while (filename = readdir(dp))
    	{
	           if(strcmp(filename->d_name,".") && strcmp(filename->d_name,".."))
	           {
		               sdflag = true;

		               time_t timep;
		               struct tm *p = NULL;
		               time(&timep);
		               p = localtime(&timep);

		               char cmd_str[256] = { 0 };

				/* umount */
				sprintf(cmd_str, "umount /media/%s", filename->d_name);
				system(cmd_str);

				/* mount */
				memset(cmd_str, 0, sizeof(cmd_str));
				sprintf(cmd_str, "mount /dev/%s	/media/%s", filename->d_name, filename->d_name);
				system(cmd_str);

				/* copy */
				memset(cmd_str, 0, sizeof(cmd_str));
		               sprintf(cmd_str,"cp /opt/records /media/%s/records_%s_%d-%02d-%02d_%02d-%02d -rf", filename->d_name, g_terminal_no, (1900+p->tm_year), (1+p->tm_mon), p->tm_mday, p->tm_hour, \
					   	p->tm_min);   
		               system(cmd_str);

				/* umout */
				memset(cmd_str, 0, sizeof(cmd_str));
				sprintf(cmd_str, "umount /media/%s", filename->d_name);
				system(cmd_str);	   
			
		               label_copySDTradeRecordString->setText(tr("交易数据拷贝完成"));

				sdtradecopy_f = 2;
	           }
    	}	

    	if(sdflag == false)
        	label_copySDTradeRecordString->setText(tr("SD未插入"));

	 closedir(dp);
}

void DebugToolsDlg::slots_UnityFileCopyIn(void)
{
	DIR * dp = NULL;
	DIR * dp2 = NULL;
	struct dirent *filename2 = NULL;
   	struct dirent *filename = NULL;
	bool unit_file_f = false;
	label_copyUnityString->clear();

	if(unityFileCopyIn_f == 2)
	{
		label_copyUnityString->setText(tr("泛文件已拷入完成,请退出"));
		return;
	}
	
	dp = opendir("/media");
	if (!dp)
	{
            label_copyUnityString->setText(tr("找不到 /media 目录"));
            return;
	}

 	while (filename = readdir(dp))
    	{
               if(strcmp(filename->d_name,".") && strcmp(filename->d_name,".."))
               {
                    char s_dir[64] = {0};
                    sprintf(s_dir, "/media/%s/UnityFileDir", filename->d_name);
                    dp2 = opendir(s_dir);
                    if (!dp2)
                    {
                            closedir(dp);
                            label_copyUnityString->setText(tr("找不到UnityFileDir文件夹"));
                            return;
                    }
                    closedir(dp2);

                    label_copyUnityString->setText(tr("找到UnityFileDir文件夹"));

                    sprintf(s_dir, "/media/%s/UnityFileDir/universal", filename->d_name);
                    dp2 = opendir(s_dir);
                    if (!dp2)
                    {
                            closedir(dp);
                            label_copyUnityString->setText(tr("找不到/universal文件夹"));
                            return;
                    }

                    label_copyUnityString->setText(tr("找到/universal文件夹"));

                    char newFile[24] = {0};
                    while (filename2 = readdir(dp2))
                    {
                                if(strcmp(filename2->d_name,".") && strcmp(filename2->d_name,".."))
                                {
                                        // 拷贝unity_file 文件
                                        if(strncmp(filename2->d_name,"unity_file",10)!=0)
                                                continue;

                                        if(strcmp(newFile,filename2->d_name)<0)
                                                strcpy(newFile,filename2->d_name);
                                }
                    }
                    closedir(dp2);

                    if(strlen(newFile)==0){
                            closedir(dp);
                            label_copyUnityString->setText(tr("没有泛文件"));
                            return;
                    }

                    //   printf("**************%s**********\n",newFile);

                    unit_file_f = true;

                    char cmd_str[256] = { 0 };

                    /* umount */
                    sprintf(cmd_str, "umount /media/%s", filename->d_name);
                    system(cmd_str);

                    /* mount */
                    memset(cmd_str, 0, sizeof(cmd_str));
                    sprintf(cmd_str, "mount /dev/%s	/media/%s", filename->d_name, filename->d_name);
                    system(cmd_str);

                    /* remove */
                    memset(cmd_str, 0, sizeof(cmd_str));
                    sprintf(cmd_str, "rm /opt/universal/unity_file*.db");
                    system(cmd_str);


                    /* copy in */
                    memset(cmd_str, 0, sizeof(cmd_str));

                    sprintf(cmd_str,"cp /media/%s/UnityFileDir/universal/%s /opt/universal/unity_file.db -rf",filename->d_name, newFile);
                    //printf("**************%s**********\n",cmd_str);
                    system(cmd_str);

                    /* umout */
                    memset(cmd_str, 0, sizeof(cmd_str));
                    sprintf(cmd_str, "umount /media/%s", filename->d_name);
                    system(cmd_str);

                    label_copyUnityString->setText(tr("拷贝完成,请重启设备"));

                    unityFileCopyIn_f = 2;

               }
    	}	

    	if(unit_file_f == false)
        label_copyUnityString->setText(tr("SD未插入"));
	
	 closedir(dp);
}

void DebugToolsDlg::slots_UnityFileCopyOut(void)
{
	DIR * dp = NULL;
   	struct dirent *filename = NULL;
	bool unit_file_f = false;
	FILE *fp = NULL;
	label_copyUnityString->clear();

	if(unityFileCopyOut_f == 2)
	{
		label_copyUnityString->setText(tr("泛文件已拷出完成,请退出"));
		return;
	}
	
	dp = opendir("/media");
	if (!dp)
	{
       	label_copyUnityString->setText(tr("找不到 /media 目录"));
       	return;
	}

 	while (filename = readdir(dp))
    	{
	           if(strcmp(filename->d_name,".") && strcmp(filename->d_name,".."))
	           {
		               unit_file_f = true;
					   
		               time_t timep;
		               struct tm *p = NULL;
		               time(&timep);
		               p = localtime(&timep);

		               char cmd_str[256] = { 0 };



				char s_dir[64] = {0};
				char s_log[512] = {0};

				/* umount */
				sprintf(cmd_str, "umount /media/%s", filename->d_name);
				system(cmd_str);

				/* mount */
				memset(cmd_str, 0, sizeof(cmd_str));
				sprintf(cmd_str, "mount /dev/%s	/media/%s", filename->d_name, filename->d_name);
				system(cmd_str);		 

				/* copy out 能覆盖*/
				memset(cmd_str, 0, sizeof(cmd_str));
				sprintf(cmd_str,"cp /opt/universal/ /media/%s/UnityFileDir_%s_%d-%02d-%02d_%02d-%02d-%02d -rf", filename->d_name, g_terminal_no, (1900+p->tm_year), (1+p->tm_mon), p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);  
				system(cmd_str);

				/* Write Log */
				sprintf(s_dir, "/media/%s/UnityFileDir_%s_%d-%02d-%02d_%02d-%02d-%02d/UnityFileLog.txt", filename->d_name, g_terminal_no, (1900+p->tm_year), (1+p->tm_mon), p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);   
				fp = fopen(s_dir, "wb");
				if(fp == NULL)
				{
					closedir(dp);
					return;
				}

				sprintf(s_log, "TerminalNo:%s\nDataTime:%d-%02d-%02d_%02d-%02d-%02d\n",  \
							g_terminal_no,  \
							(1900+p->tm_year), (1+p->tm_mon), p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
				
				fputs(s_log, fp);
				fflush(fp);
				fclose(fp);
		
				/* umout */
				memset(cmd_str, 0, sizeof(cmd_str));
				sprintf(cmd_str, "umount /media/%s", filename->d_name);
				system(cmd_str);	   
			
		               label_copyUnityString->setText(tr("数据拷出完成"));

				unityFileCopyOut_f = 2;
	           }
    	}	

    	if(unit_file_f == false)
        	label_copyUnityString->setText(tr("SD未插入"));
	
	 closedir(dp);
}

void DebugToolsDlg::__allWidgetClear(void)
{
	label_debugToolsShow->clear();
	label_copySDString->clear();
	label_copyUnityString->clear();
	label_copySDTradeRecordString->clear();
}

void DebugToolsDlg::__showConfigResult(const unsigned char result)
{
	
}

void DebugToolsDlg::__controlSettings(const int ctrl)
{
	if(ctrl > 0) //超级管理卡
	{
		pushButton_deviceReboot->setEnabled(TRUE);
		
		pushButton_unityFileCopyIn->setEnabled(TRUE);
		pushButton_unityFileCopyOut->setEnabled(TRUE);
		
		pushButton_copySDLog->setEnabled(TRUE);
		
		pushButton_copySDTradeRecord->setEnabled(TRUE);
		
		pushButton_manualRefresh->setEnabled(TRUE);
		
	}
	else
	{
		pushButton_deviceReboot->setEnabled(TRUE);
		
		pushButton_unityFileCopyIn->setEnabled(FALSE);
		pushButton_unityFileCopyOut->setEnabled(FALSE);
		
		pushButton_copySDLog->setEnabled(FALSE);
		
		pushButton_copySDTradeRecord->setEnabled(FALSE);
		
		pushButton_manualRefresh->setEnabled(TRUE);
	}
}

bool DebugToolsDlg::eventFilter(QObject *watched, QEvent *event)
{
	 __allWidgetClear();

	if(watched == pushButton_manualRefresh)
	{
		if(event->type()==QEvent::FocusIn) 
		{
			pushButton_manualRefresh->setStyleSheet("background-color: rgb(85, 170, 255)");
		}
		else if (event->type()==QEvent::FocusOut)
		{
			pushButton_manualRefresh->setStyleSheet("background-color: rgb(255, 255, 255)");
		}
	}	

	if(watched == pushButton_deviceReboot)
	{
		if(event->type()==QEvent::FocusIn) 
		{
			pushButton_deviceReboot->setStyleSheet("background-color: rgb(85, 170, 255)");
		}
		else if (event->type()==QEvent::FocusOut)
		{
			pushButton_deviceReboot->setStyleSheet("background-color: rgb(255, 255, 255)");
		}
	}	

	    if(watched == pushButton_copySDLog)
	    {
		        if(event->type()==QEvent::FocusIn)
		        {
		            	pushButton_copySDLog->setStyleSheet("background-color: rgb(85, 170, 255)");
		        }
		        else if (event->type()==QEvent::FocusOut)
		        {
		           	 pushButton_copySDLog->setStyleSheet("background-color: rgb(255, 255, 255)");
		        }
	    }

	 	if(watched == pushButton_unityFileCopyIn)
	    {
		        if(event->type()==QEvent::FocusIn)
		        {
		            	pushButton_unityFileCopyIn->setStyleSheet("background-color: rgb(85, 170, 255)");
		        }
		        else if (event->type()==QEvent::FocusOut)
		        {
		           	 pushButton_unityFileCopyIn->setStyleSheet("background-color: rgb(255, 255, 255)");
		        }
	    }

		if(watched == pushButton_unityFileCopyOut)
	    {
		        if(event->type()==QEvent::FocusIn)
		        {
		            	pushButton_unityFileCopyOut->setStyleSheet("background-color: rgb(85, 170, 255)");
		        }
		        else if (event->type()==QEvent::FocusOut)
		        {
		           	 pushButton_unityFileCopyOut->setStyleSheet("background-color: rgb(255, 255, 255)");
		        }
	    }

		if(watched == pushButton_copySDTradeRecord)
	    {
		        if(event->type()==QEvent::FocusIn)
		        {
		            	pushButton_copySDTradeRecord->setStyleSheet("background-color: rgb(85, 170, 255)");
		        }
		        else if (event->type()==QEvent::FocusOut)
		        {
		           	 pushButton_copySDTradeRecord->setStyleSheet("background-color: rgb(255, 255, 255)");
		        }
	    }
	
	return QWidget::eventFilter(watched,event); 
}


