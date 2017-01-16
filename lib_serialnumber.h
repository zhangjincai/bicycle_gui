#ifndef __LIB_SERIALNUMBER_H__
#define __LIB_SERIALNUMBER_H__

/*@*/
#ifdef __cplusplus
extern "C" {
#endif
/*@*/




int lib_SN_consume_read32(unsigned int *sn32);
int lib_SN_admin_card_read32(unsigned int *sn32);
int lib_SN_default_read32(unsigned int *sn32);
int lib_SN_except_read32(unsigned int *sn32);

int lib_SN_consume_write32(unsigned int sn32);
int lib_SN_admin_card_write32(unsigned int sn32);
int lib_SN_default_write32(unsigned int sn32);
int lib_SN_except_write32(unsigned int sn32);




/*@*/
#ifdef __cplusplus
}
#endif
#endif


