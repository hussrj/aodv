/* aodv_pkt_support.h */
/* Data Structures used for the packet fields	*/
/* used in the AODV RREQ, RREP and RRER packets	*/

/****************************************/
/*      Copyright (c) 1986-2008         */
/*     by OPNET Technologies, Inc.      */
/*      (A Delaware Corporation)        */
/*   7255 Woodmont Av., Suite 250       */
/*     Bethesda, MD 20814, U.S.A.       */
/*       All Rights Reserved.           */
/****************************************/

/* Protect against multiple includes. 	*/
#ifndef	_AODV_PKT_SUPPORT_H_INCLUDED_
#define _AODV_PKT_SUPPORT_H_INCLUDED_

#include <ip_addr_v4.h>
#include <ip_rte_v4.h>
#include <aodv.h>

#if defined (__cplusplus)
extern "C" {
#endif

/********* CONSTANTS ***********/

/* Type of packet	*/
#define AODVC_ROUTE_REQUEST		1
#define AODVC_ROUTE_REPLY		2
#define AODVC_ROUTE_ERROR		3
#define	AODVC_RREP_ACK			4
#define AODVC_HELLO				5


/* Packet Sizes in bits	*/
#define AODVC_RREQ_IPV4_SIZE	192
#define AODVC_RREQ_IPV6_SIZE	384
#define	AODVC_RREP_IPV4_SIZE	160
#define	AODVC_RREP_IPV6_SIZE	352
#define AODVC_RREP_ACK_SIZE		16

/***** Enumerated Data Types ******/
typedef enum
	{
	AodvC_Link_Break_Detect,
	AodvC_Data_Packet_No_Route,
	AodvC_Rerr_Received
    } AodvC_Rerr_Process;

/******** Data Structures *********/

/* Options in the AODV packet	*/
typedef struct
	{
	int				type;
	void*			value_ptr;
	} AodvT_Packet_Option;
	

/* Encapsulate all GEO/LAR options */
//MKA 01/25/11
typedef struct
{
	Point2D				src;
	Point2D				prev;
	Point2D				dst;
	int					request_level;
	double				velocity;
} AodvT_LAR_Info;

/* Route Request Option	*/
typedef struct
	{
	Boolean				join_flag;
	Boolean				repair_flag;
	Boolean				grat_rrep_flag;
	Boolean				dest_only;
	Boolean				unknown_seq_num_flag;
	int					hop_count;
	int					rreq_id;
	InetT_Address		dest_addr;
	int					dest_seq_num;
	InetT_Address		src_addr;
	int					src_seq_num;
	AodvT_LAR_Info		geo_lar_options;
	
/*	ALL OF THESE OPTIONS HAVE BEEN ENCAPSULATED INTO
	THE AodvT_LAR_Info STRUCT!

	// MHAVH 11/10/08
	double				src_x;
	double				src_y;
	double				dst_x;
	double				dst_y;
	int					request_level;
	// END MHAVH
	//MKA 12/31/10
	double prev_x;
	double prev_y;
	//END MKA
*/
	} AodvT_Rreq;

/* Route Reply Option	*/
typedef struct
	{
	Boolean				repair_flag;
	Boolean				ack_required_flag;
	int					hop_count;
	InetT_Address		dest_addr;
	int					dest_seq_num;
	//MHAVH 10/21/08
	double				dst_x;
	double				dst_y;
	//END MHAVH
	InetT_Address		src_addr;
	double				lifetime;
	} AodvT_Rrep;

/* Route Error Option	*/
typedef struct
	{
	InetT_Address		unreachable_dest;
	int					unreachable_dest_seq_num;
	} AodvT_Unreachable_Node;

typedef struct
	{
	Boolean				no_delete_flag;
	int					num_unreachable_dest;
	List*				unreachable_dest_lptr; /* Each element is AodvT_Unreachable_Node	*/
	} AodvT_Rerr;

#if defined (__cplusplus)
} /* end of 'extern "C" {' */
#endif

/* End if for protection against multiple includes.	 */
#endif 	/*_AODV_PKT_SUPPORT_H_INCLUDED_		 */
