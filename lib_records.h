#ifndef __LIB_RECORDS_H__
#define __LIB_RECORDS_H__


/*@*/
#ifdef __cplusplus
extern "C" {
#endif
/*@*/

/*
 * 函数返回码定义
 */
#define LIB_REC_NULL			(NULL) 
#define LIB_REC_OK           		0   /* Successful result */
#define LIB_REC_ERROR        1   /* SQL error or missing database */
#define LIB_REC_INTERNAL     2   /* Internal logic error in SQLite */
#define LIB_REC_PERM         3   /* Access permission denied */
#define LIB_REC_ABORT        4   /* Callback routine requested an abort */
#define LIB_REC_BUSY         5   /* The database file is locked */
#define LIB_REC_LOCKED       6   /* A table in the database is locked */
#define LIB_REC_NOMEM        7   /* A malloc() failed */
#define LIB_REC_READONLY     8   /* Attempt to write a readonly database */
#define LIB_REC_INTERRUPT    9   /* Operation terminated by sqlite3_interrupt()*/
#define LIB_REC_IOERR       10   /* Some kind of disk I/O error occurred */
#define LIB_REC_CORRUPT     11   /* The database disk image is malformed */
#define LIB_REC_NOTFOUND    12   /* Unknown opcode in sqlite3_file_control() */
#define LIB_REC_FULL        13   /* Insertion failed because database is full */
#define LIB_REC_CANTOPEN    14   /* Unable to open the database file */
#define LIB_REC_PROTOCOL    15   /* Database lock protocol error */
#define LIB_REC_EMPTY       16   /* Database is empty */
#define LIB_REC_SCHEMA      17   /* The database schema changed */
#define LIB_REC_TOOBIG      18   /* String or BLOB exceeds size limit */
#define LIB_REC_CONSTRAINT  19   /* Abort due to constraint violation */
#define LIB_REC_MISMATCH    20   /* Data type mismatch */
#define LIB_REC_MISUSE      21   /* Library used incorrectly */
#define LIB_REC_NOLFS       22   /* Uses OS features not supported on host */
#define LIB_REC_AUTH        23   /* Authorization denied */
#define LIB_REC_FORMAT      24   /* Auxiliary database format error */
#define LIB_REC_RANGE       25   /* 2nd parameter to sqlite3_bind out of range */
#define LIB_REC_NOTADB      26   /* File opened that is not a database file */
#define LIB_REC_NOTICE      27   /* Notifications from sqlite3_log() */
#define LIB_REC_WARNING     28   /* Warnings from sqlite3_log() */
#define LIB_REC_ROW         100  /* sqlite3_step() has another row ready */
#define LIB_REC_DONE        101  /* sqlite3_step() has finished executing */




typedef struct records_file lib_records_file_t;

char *lib_records_get_version(void);
lib_records_file_t *lib_records_create(const char *pathname);
void lib_records_destroy(lib_records_file_t *rec);

int lib_records_consume_insert_data(lib_records_file_t *rec, const int pkt_sn, const unsigned char pid[8], const unsigned char lid[8], const unsigned char trade_record[90]);
int lib_records_exception_insert_data(lib_records_file_t *rec, const int sn, const unsigned char pid[8], const unsigned char trade_record[90]);

int lib_records_reconsume_insert_data(lib_records_file_t *rec, const int pkt_sn, const unsigned char pid[8], const unsigned char macaddr[6], const int pkt_len, void *pkt_data);
int lib_records_reconsume_select_data(lib_records_file_t *rec, const int pkt_sn, int *pkt_len, void *pkt_data);
int lib_records_reconsume_select_all_data(lib_records_file_t *rec, int *pkt_sn, int *pkt_len, void *pkt_data);
int lib_records_reconsume_delete_data(lib_records_file_t *rec, const int pkt_sn);
int lib_records_reconsume_count(lib_records_file_t *rec, int *count);

int lib_records_oncard_ack_insert_data(lib_records_file_t *rec, const int pkt_sn, const unsigned char pid[8], const unsigned char macaddr[6], const int pkt_len, void *pkt_data);
int lib_records_oncard_ack_select_data(lib_records_file_t *rec, const int pkt_sn, int *pkt_len, void *pkt_data);
int lib_records_oncard_ack_select_all_data(lib_records_file_t *rec, int *pkt_sn, int *pkt_len, void *pkt_data);
int lib_records_oncard_ack_delete_data(lib_records_file_t *rec, const int pkt_sn);
int lib_records_oncard_ack_count(lib_records_file_t *rec, int *count);

int lib_records_offcard_ack_insert_data(lib_records_file_t *rec, const int pkt_sn, const unsigned char pid[8], const unsigned char macaddr[6], const int pkt_len, void *pkt_data);
int lib_records_offcard_ack_select_data(lib_records_file_t *rec, const int pkt_sn, int *pkt_len, void *pkt_data);
int lib_records_offcard_ack_select_all_data(lib_records_file_t *rec, int *pkt_sn, int *pkt_len, void *pkt_data);
int lib_records_offcard_ack_delete_data(lib_records_file_t *rec, const int pkt_sn);
int lib_records_offcard_ack_count(lib_records_file_t *rec, int *count);

int lib_records_recharge_insert_data(lib_records_file_t *rec, const int pkt_sn, const unsigned char pid[8], const unsigned char macaddr[6], const int pkt_len, void *pkt_data);
int lib_records_recharge_select_data(lib_records_file_t *rec, const int pkt_sn, int *pkt_len, void *pkt_data);
int lib_records_recharge_select_all_data(lib_records_file_t *rec, int *pkt_sn, int *pkt_len, void *pkt_data);
int lib_records_recharge_delete_data(lib_records_file_t *rec, const int pkt_sn);
int lib_records_recharge_count(lib_records_file_t *rec, int *count);







/*@*/
#ifdef __cplusplus
}
#endif
#endif


