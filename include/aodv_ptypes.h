/* aodv_ptypes.h */
/* Prototypes used by the AODV routing protocol	*/

/****************************************/
/*      Copyright (c) 1986-2008         */
/*     by OPNET Technologies, Inc.      */
/*      (A Delaware Corporation)        */
/*   7255 Woodmont Av., Suite 250       */
/*     Bethesda, MD 20814, U.S.A.       */
/*       All Rights Reserved.           */
/****************************************/

/* Protect against multiple includes. 	*/
#ifndef	_AODV_PTYPES_H_INCLUDED_
#define _AODV_PTYPES_H_INCLUDED_

#include <aodv.h>
#include <aodv_pkt_support.h>
#include <ip_addr_v4.h>
#include <ip_cmn_rte_table.h>

#if defined (__cplusplus)
extern "C" {
#endif

/***********************************************/
/******* Includes prototypes for        ********/
/******* 1. aodv_route_table.ex.c       ********/
/******* 2. aodv_packet_queue.ex.c      ********/
/******* 3. aodv_request_table.ex.c     ********/
/******* 4. aodv_pkt_support.ex.c		********/
/***********************************************/

/* aodv_route_table function prototypes	*/
AodvT_Route_Table*			aodv_route_table_create (IpT_Cmn_Rte_Table*, IpT_Rte_Proc_Id, double, double, AodvT_Local_Stathandles*,
													 InetT_Addr_Family);
AodvT_Route_Entry*			aodv_route_table_entry_create (AodvT_Route_Table*, InetT_Address, InetT_Subnet_Mask,
														   InetT_Address, IpT_Port_Info, int, int, double);
AodvT_Route_Entry*			aodv_route_table_entry_get (AodvT_Route_Table*, InetT_Address);
Compcode					aodv_route_table_entry_param_get (AodvT_Route_Entry*, int, void*);
Compcode					aodv_route_table_entry_param_set (AodvT_Route_Entry*, int, ...);
Compcode					aodv_route_table_precursor_add (AodvT_Route_Entry*, InetT_Address);
Compcode					aodv_route_table_entry_next_hop_update (AodvT_Route_Table*, AodvT_Route_Entry*, InetT_Address, 
																		int, IpT_Port_Info);
Compcode					aodv_route_table_entry_state_set (AodvT_Route_Table*, AodvT_Route_Entry*, InetT_Address, 
																AodvC_Route_Entry_State);
Compcode					aodv_route_table_entry_delete (AodvT_Route_Table*, InetT_Address);
void						aodv_route_table_entry_expiry_time_update (AodvT_Route_Entry*, InetT_Address, double, int);


/* aodv_packet_queue function prototypes	*/
AodvT_Packet_Queue*			aodv_packet_queue_create (int, AodvT_Local_Stathandles*, InetT_Addr_Family);
void						aodv_packet_queue_insert (AodvT_Packet_Queue*, Packet*, InetT_Address);
List*						aodv_packet_queue_all_pkts_to_dest_get (AodvT_Packet_Queue*, InetT_Address, Boolean, Boolean);
void						aodv_packet_queue_all_pkts_to_dest_delete (AodvT_Packet_Queue*, InetT_Address);
int 						aodv_packet_queue_num_pkts_get (AodvT_Packet_Queue*, InetT_Address);
	

/* aodv_request_table function prototypes	*/
AodvT_Request_Table*		aodv_request_table_create (double, int, InetT_Addr_Family);

// MHAVH 13/11/08 - an extra parameter that corresponds to request level
void						aodv_request_table_orig_rreq_insert_geo (AodvT_Request_Table*, int, InetT_Address,
																	int, double, int, int);
// END MHAVH

void						aodv_request_table_forward_rreq_insert (AodvT_Request_Table*, int, InetT_Address);
Boolean						aodv_route_request_forward_entry_exists (AodvT_Request_Table*, int, InetT_Address);
Boolean						aodv_request_table_orig_rreq_exists (AodvT_Request_Table*, InetT_Address);
AodvT_Orig_Request_Entry*	aodv_route_request_orig_entry_get (AodvT_Request_Table*, int, Boolean);
void						aodv_route_request_orig_entry_delete (AodvT_Request_Table*, InetT_Address);
void						aodv_request_table_orig_entry_mem_free (AodvT_Orig_Request_Entry*);

// MHAVH 10/27/08 - aodv_geo_table function prototypes */
AodvT_Geo_Table* 			aodv_geo_table_create (InetT_Addr_Family hash_key_addr_family);
void						aodv_geo_table_insert (AodvT_Geo_Table* geo_table_ptr, InetT_Address dst_address, 
										double dst_x, double dst_y, int sequence_number);
Boolean						aodv_geo_table_entry_exists(AodvT_Geo_Table* geo_table_ptr, InetT_Address dst_address);
AodvT_Geo_Entry*			aodv_geo_table_entry_get (AodvT_Geo_Table* geo_table_ptr, InetT_Address dst_address, Boolean remove);
Compcode					aodv_geo_table_entry_delete (AodvT_Geo_Table* geo_table_ptr, InetT_Address dst_address);
void						aodv_geo_table_entry_mem_free (AodvT_Geo_Entry* geo_entry_ptr);
// MHAVH

/* aodv_pkt_support function prototypes	*/
Packet*						aodv_pkt_support_pkt_create (AodvT_Packet_Option*, int);
AodvT_Packet_Option*		aodv_pkt_support_rreq_option_create (Boolean, Boolean, Boolean, Boolean, Boolean, 
												int, int, InetT_Address, int, InetT_Address, int);
// MHAVH 10/11/08 - our modified function
AodvT_Packet_Option*
aodv_pkt_support_rreq_option_create_geo (Boolean, Boolean, Boolean, Boolean, Boolean,
	int, int, InetT_Address, int, InetT_Address, int, AodvT_LAR_Info);
// MHAVH


AodvT_Packet_Option*		aodv_pkt_support_rrep_option_create (Boolean, Boolean, int, InetT_Address, int, 
												InetT_Address, double, int);

//MHAVH 10/21/08 - our modified function
AodvT_Packet_Option*		aodv_pkt_support_rrep_option_create_geo (Boolean, Boolean, int, InetT_Address, int, 
												InetT_Address, double, int, double, double);
//MHAVH

AodvT_Packet_Option*		aodv_pkt_support_rerr_option_create (Boolean, int, List*);
AodvT_Packet_Option*		aodv_pkt_support_rrep_ack_option_create (void);
AodvT_Unreachable_Node*		aodv_pkt_support_unreachable_nodes_mem_alloc (void);

/* aodv_support function prototypes	*/
AodvT_Global_Stathandles*	aodv_support_global_stat_handles_obtain (void);
Boolean						aodv_support_hello_efficiency_sim_attr_get (void);
void						aodv_support_routing_traffic_sent_stats_update (AodvT_Local_Stathandles*, 
																	AodvT_Global_Stathandles*, Packet*);
void						aodv_support_routing_traffic_received_stats_update (AodvT_Local_Stathandles*,
																	AodvT_Global_Stathandles*, Packet*);
void						aodv_support_route_reply_sent_stats_update (AodvT_Local_Stathandles*, 
																	AodvT_Global_Stathandles*, Boolean);

/* Prototypes for aodv_support.ex.c, to generate route table report */
void  						aodv_support_route_table_print_to_string (AodvT_Route_Table*, InetT_Addr_Family);
void*						aodv_array_elements_add (void*, int, int, int);
void						aodv_support_conn_table_print_to_string (InetT_Address_Hash_Table_Handle, InetT_Addr_Family);

#if defined (__cplusplus)
} /* end of 'extern "C" {' */
#endif

/* End if for protection against multiple includes.	 */
#endif 	/*_AODV_PTYPES_H_INCLUDED_		 */
