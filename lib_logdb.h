#ifndef __LIB_LOGDB_H__
#define __LIB_LOGDB_H__

/*@*/
#ifdef __cplusplus
extern "C" {
#endif
/*@*/


/*
 * ���������붨��
 */
#define LIB_LOG_NULL					(NULL) 
#define LIB_LOG_EOK					(0)  //����
#define LIB_LOG_ERROR				(-1) //����
#define LIB_LOG_ETIMEOUT			(-2) //��ʱ
#define LIB_LOG_EFULL				(-3) //��
#define LIB_LOG_EEMPTY				(-4) //��
#define LIB_LOG_ENOMEM 				(-5) //�ڴ治��
#define LIB_LOG_EXCMEM				(-6) //�ڴ�Խ��
#define LIB_LOG_EBUSY				(-7) //æ
#define LIB_LOG_ERR_COMMAND		(-8) //��֧�ָ�ָ��

#define LIB_LOG_TRUE					(1)
#define LIB_LOG_FALSE				(0)



enum LOG_TYPE
{
	LOG_TYPE_NDEV = 1,	   //�ڵ��
	LOG_TYPE_SAE,		   //��׮
	LOG_TYPE_YCT,		  //���ͨ
	LOG_TYPE_WL,             //���� 
	LOG_TYPE_UPGRADE,  //�̼�����    
	LOG_TYPE_DOG_SOFT, //��  
	LOG_TYPE_GUI, //UI
	LOG_TYPE_END
};

enum LOG_LEVEL
{
	LOG_LEVEL_ERR = 1,       //����
	LOG_LEVEL_WARNING,   //����
	LOG_LEVEL_NOTICE,     //����Ȼ����Ҫ
	LOG_LEVEL_INFO,        //֪ͨ��Ϣ
	LOG_LEVEL_DEBUG,     //��Ϸ��Ϣ
	LOG_LEVEL_END
};

typedef struct logdb lib_logdb_t;

lib_logdb_t *lib_logdb_new(void);
void lib_logdb_destroy(lib_logdb_t *log);
int lib_logdb_reconnect(lib_logdb_t *log);
int lib_logdb_version(lib_logdb_t *log, char *ver, const unsigned int msec);
int lib_logdb_put_string(lib_logdb_t *log, const unsigned char type, const unsigned char level, const char *s_log, const unsigned int msec);
int lib_logdb_sprintf(lib_logdb_t *log, const unsigned char type, const unsigned char level,  const unsigned int msec, const char *format, ...);
int lib_logdb_set_space_size(lib_logdb_t *log, const unsigned int size, const unsigned int msec);
int lib_logdb_set_clear_log_quantity(lib_logdb_t *log, const unsigned int quantity, const unsigned int msec);




/*@*/
#ifdef __cplusplus
}
#endif
#endif

