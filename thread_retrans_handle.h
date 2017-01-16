#ifndef __THREAD_RETRANS_HANDLE_H__
#define __THREAD_RETRANS_HANDLE_H__

#include <QRunnable>
#include <QObject>
#include <QRunnable>
#include <time.h>



class ThreadRetransHandle :  public QObject, public QRunnable
{
	Q_OBJECT
	public:
		ThreadRetransHandle(void);


	private:
		void ThreadSleep(unsigned long secs);
		void mThreadSleep(unsigned long msecs);

		
			
	protected:
    		void run();	
};




















#endif

