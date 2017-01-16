#include "lnt_register_task.h"
#include "lib_lnt.h"
#include <stdio.h>


void lnt_register_thread::run()
{
    printf("---------------lnt_config.tty:%s\n",lnt_config.tty);
    __lnt_register_entry(&lnt_config);
}

lnt_register_thread::lnt_register_thread(lib_lnt_config_t *config)
{
    printf("---------------------config->tty:%s\n",config->tty);
    memcpy(&lnt_config, config, sizeof(lib_lnt_config_t));
}

lnt_register_thread::~lnt_register_thread()
{

}
