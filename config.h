#ifndef __CONFIG_H__
#define __CONFIG_H__

#if 0
#define NEW_PROTOCOL				1	//新版本的充值协议
#define	RETRANS_TRADE_RECORD		1	//交易记录重传
#define	UPLOAD_TRY_TIMES			3	//交易记录重传次数
#define NEW_UPLOAD_ON_CARD_RESUALT	1 	//押金初始化和押金处理失败后上报开卡结果
//add by zjc 2016-03-23
#endif

#define LNT_MOC_ERROR_UNKNOW			0xEE  //未知错误  //add by zjc


#define MEMBER_COUNTS_LIMIT	100		//开卡次数上限
#define MEMBER_DAYS_LIMIT	30		//开卡天数上限
#define MEMBER_MONTHS_LIMIT	12		//开卡月数上限
#define MEMBER_YEARS_LIMIT	5		//开卡年数上限

#define MEMBER_LOWER_LIMIT	1		//开卡次数、天数、月数、年数下限


enum UPE_TYPE			//更新类型
{
	UPE_TYPE_INIT = 0,
	UPE_TYPE_KERNEL,
	UPE_TYPE_FIRMWARE,
	UPE_TYPE_APPL,
	UPE_TYPE_LNT_ZM,
	UPE_TYPE_END
};


/*
 * 2016-07-26 增加一个共享内存用于与bicycle_gui通信
 * 
 */
#define UPE_SHART_MEM_KEY		20160726	

/*
 * test by zjc
 */
#define CONFS_USING_TEST_BY_ZJC		1 //zjc测试用


/*
 * 读卡器固件升级功能开启与否 1:开启 0:关闭
 */
#define CONFS_USING_READER_UPDATE		1

/*
 * 节点机自动重启 1:开启 0:关闭
 */
#define CONFS_USING_AUTO_REBOOT		0

/* 节点机自动重启时间 */
#define REBOOT_HOUR		01 //时
#define REBOOT_MINUTE	00 //分
#define REBOOT_SECOND	00 //秒

/* 网卡信息备份路径 2016-12-21 */
//#define	NET_INFO_BACKUP_PATH	"/opt/logpath/net_info.txt"

/*
 * 网卡信息备份 1:开启 0:关闭
 */
#define CONFS_USING_NET_FLOW_BACKUP		0



#endif
