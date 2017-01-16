#ifndef LNT_REGISTER_TASK_H
#define LNT_REGISTER_TASK_H

#include <QRunnable>
#include <QObject>
#include <QThread>
#include "lib_lnt.h"

class lnt_register_thread : public QThread
{
public:
    lnt_register_thread(lib_lnt_config_t *config);
    ~lnt_register_thread();
    void run();

public:
    lib_lnt_config_t lnt_config;
    lnt_register_thread *lnt_reg_thread;

};


#endif // LNT_REGISTER_TASK_H
