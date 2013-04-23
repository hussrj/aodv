
/* aodv.h */
/* Data Structures used for the Ad Hoc On-Demand 	*/
/* Distance Vector Routing Protocol (AODV) for 		*/
/* Mobile Ad-Hoc Networking (MANET)					*/

/****************************************/
/*      Copyright (c) 1986-2008         */
/*     by OPNET Technologies, Inc.      */
/*      (A Delaware Corporation)        */
/*   7255 Woodmont Av., Suite 250       */
/*     Bethesda, MD 20814, U.S.A.       */
/*       All Rights Reserved.           */
/****************************************/

/* Protect against multiple includes. 	*/
#ifndef	_AODV_H_INCLUDED_
#define _AODV_H_INCLUDED_

#include <ip_addr_v4.h>
#include <ip_rte_v4.h>
#include <ip_cmn_rte_table.h>

#if defined (__cplusplus)
extern "C" {
#endif

/******** Constants *********/

/* Constants to access entries in the route table	*/
#define	AODVC_ROUTE_ENTRY_DEST_SEQ_NUM				1
#define	AODVC_ROUTE_ENTRY_VALID_SEQ_NUM_FLAG		2
#define AODVC_ROUTE_ENTRY_ROUTE_ENTRY_STATE			3
#define AODVC_ROUTE_ENTRY_NEXT_HOP_ADDR				4
#define AODVC_ROUTE_ENTRY_OUT_PORT_INFO				5
#define AODVC_ROUTE_ENTRY_HOP_COUNT					6
#define AODVC_ROUTE_ENTRY_PRECURSOR_LIST			7
#define AODVC_ROUTE_ENTRY_EXPIRY_TIME				8

/* Constants to identify the timers	*/
#define AODVC_ROUTE_ENTRY_INVALID		1
#define AODVC_ROUTE_ENTRY_EXPIRED		2
#define	AODVC_ROUTE_REQUEST_EXPIRY		3
#define AODVC_HELLO_TIMER_EXPIRY		4
#define AODVC_CONN_LOSS_TIMER			5
// VHRCMADU 11/15/10
#define AODVC_LAR_UPDATE				6


/* Constant to specify an infinte packet queue size	*/
#define AODVC_INFINITE_PACKET_QUEUE		-1

/* Constant to specify the delete period constant K	*/
#define AODVC_DELETE_PERIOD_CONSTANT_K	5

/* Constant to specify an invalid destination sequence number	*/
#define AODVC_DEST_SEQ_NUM_INVALID		-1

// MHAVH 11/01/08 constants for aodv_geo data
#define DEFAULT_X						-1.0
#define DEFAULT_Y						-1.0
#define INITIAL_REQUEST_LEVEL			0
// RH 4/23/13 angle_expand makes broadcast level variable
// #define BROADCAST_REQUEST_LEVEL         3
// MHAVH

// HAV 03/18/09 - types of possible aodv options
//#define AODV_OPTION_NONE				0
//#define AODV_OPTION_ANGLE				1
//#define AODV_OPTION_DISTANCE			2
//#define AODV_OPTION_ANGLE_HELLO			3

// HAV_MR 06/26/09 - types of possible aodv options
#define AODV_TYPE_REGULAR			0	// Regular AODV
#define AODV_TYPE_LAR_DISTANCE		1	// LAR based on the node's distance to the destination
#define AODV_TYPE_LAR_ZONE			2	// LAR based on request zone
#define AODV_TYPE_GEO_STATIC		3  // staic angle computed based on the source
#define AODV_TYPE_GEO_EXPAND		4  // angle is expanded by the intermediate nodes if 
										   // no neighbours within the current angle
#define AODV_TYPE_GEO_ROTATE		5  // recompute the current node's belonging to the
										   // search area based on previous node's location, instead of
											// node the source node location
#define AODV_TYPE_GEO_ROTATE_01		6  // set angle to 180 degrees, forward to all neighbours in the
										   // 180 degree, if fails to find the route do regular AODV 


/******** Enumenated Data Types *********/

/* State of the route entry in the route table	*/
typedef enum
	{
	AodvC_Undef_Route,
	AodvC_Valid_Route,
	AodvC_Invalid_Route
	} AodvC_Route_Entry_State;

/******** Data Structures *********/

/*********************************************/
/**************** STATHANDLES ****************/
/*********************************************/

/* Local Statistic handles	*/
typedef struct
	{
	Stathandle		route_discovery_time_shandle;
	Stathandle		num_hops_shandle;
	Stathandle		route_table_size_shandle;
	Stathandle		pkt_queue_size_shandle;
	Stathandle		num_pkts_discard_shandle;
	Stathandle		rte_traf_rcvd_bps_shandle;
	Stathandle		rte_traf_rcvd_pps_shandle;
	Stathandle		rte_traf_sent_bps_shandle;
	Stathandle		rte_traf_sent_pps_shandle;
	Stathandle		total_requests_sent_shandle;
	Stathandle		total_replies_sent_shandle;
	Stathandle		total_replies_sent_from_dest_shandle;
	Stathandle		total_cached_replies_sent_shandle;
	Stathandle		total_route_errors_sent_shandle;
	Stathandle		total_requests_fwd_shandle;
	} AodvT_Local_Stathandles;


/* Global Statistic handles	*/
typedef struct
	{
	Stathandle		num_hops_global_shandle;
	Stathandle		num_pkts_discard_global_shandle;
	Stathandle		route_discovery_time_global_shandle;
	Stathandle		rte_traf_rcvd_bps_global_shandle;
	Stathandle		rte_traf_rcvd_pps_global_shandle;
	Stathandle		rte_traf_sent_bps_global_shandle;
	Stathandle		rte_traf_sent_pps_global_shandle;
	Stathandle		total_requests_sent_global_shandle;
	Stathandle		total_replies_sent_global_shandle;
	Stathandle		total_replies_sent_from_dest_global_shandle;
	Stathandle		total_cached_replies_sent_global_shandle;
	Stathandle		total_route_errors_sent_global_shandle;
	Stathandle		num_aodv_fallbacks_global_shandle;
	} AodvT_Global_Stathandles;


/*********************************************/
/**************** ROUTE TABLE ****************/
/*********************************************/

/* The route table is a hash table of route entries	*/
/* Each entry in the route table is indexed by the	*/
/* destination IP address.							*/
typedef struct
	{
	InetT_Address_Hash_Table_Handle	route_table;
	IpT_Cmn_Rte_Table*				ip_cmn_rte_table_ptr;
	IpT_Rte_Proc_Id					aodv_protocol_id;
	double							route_expiry_time;
	double							delete_period;
	AodvT_Local_Stathandles*		stat_handles_ptr;
	int								active_route_count;
	} AodvT_Route_Table;

typedef struct
	{
	IpT_Dest_Prefix 			dest_prefix;
	int							dest_seq_num;
	Boolean						valid_dest_sequence_number_flag;
	AodvC_Route_Entry_State		route_entry_state;
	InetT_Address				next_hop_addr;
	IpT_Port_Info 				next_hop_port_info;
	int							hop_count;
	List*						precursor_lptr;
	double						route_expiry_time;
	Evhandle					route_expiry_evhandle;
	} AodvT_Route_Entry;


/*********************************************/
/*************** PACKET QUEUE ****************/
/*********************************************/

/* This table stores the list of data packets	*/
/* awaiting a route discovery. Each entry in 	*/
/* the packet queue table is a list of packets	*/
/* to a specific destination. Each entry in the	*/
/* table is indexed by the destination address	*/
typedef struct
	{
	InetT_Address_Hash_Table_Handle	pkt_queue_table;
	int								max_queue_size;
	int								current_queue_size;
	AodvT_Local_Stathandles*		stat_handle_ptr;
	} AodvT_Packet_Queue;

/* Each entry in the list contains the data		*/
/* packet and the insert time of the packet.	*/
/* If the queue reaches its maximum size, the  	*/
/* oldest packet in the queue is discarded 		*/
typedef struct
	{
	Packet*		pkptr;
	double		insert_time;
	} AodvT_Packet_Entry;


/*********************************************/
/*********** ROUTE REQUEST TABLE *************/
/*********************************************/

/* The request table stores information about	*/
/* route requests originated or forwarded by a	*/
/* node. This is done so that duplicate 		*/
/* reception of route requests can be discarded	*/
/* The table is indexed by the originator IP	*/
/* address of the route request and contains a 	*/
/* list of request entries 					 	*/

typedef struct
	{
	PrgT_Bin_Hash_Table*			orig_request_table;
	InetT_Address_Hash_Table_Handle	forward_request_table;
	double							forward_request_expiry_time;
	int								max_rreq_retries;
	} AodvT_Request_Table;

typedef struct
	{
	InetT_Address		target_address;
	int					request_id;
	double				insert_time;
	double				current_request_expiry_time;
	Evhandle			rreq_expiry_evhandle;
	int					current_ttl_value;
	int					num_retries;
   
	// MHAVH 13/11/08 - a field for request_level
	int					request_level;
	// END MHAVH
	
	} AodvT_Orig_Request_Entry;

typedef struct
	{
	int					request_id;
	double				insert_time;
	} AodvT_Forward_Request_Entry;

typedef struct
	{
	InetT_Address		orig_addr;
	int					request_id;
	} AodvT_Request_Expiry;


/*********************************************/
/************ CONNECTIVITY TABLE *************/
/*********************************************/

/* The connectivity table stores the list of	*/
/* neighbor nodes from which it has received 	*/
/* a hello message or any other packet. This is	*/
/* to determine the connectivity to each of its	*/
/* neighbor nodes.								*/

/* The connectivity table is a hash table 		*/
/* indexed by the neighbor IP address. Each		*/
/* entry in the table contains the connectivity	*/
/* information of the neighbor.					*/

typedef struct
	{
	InetT_Address		neighbor_address;
	double				last_hello_received_time;
	Evhandle			conn_expiry_handle;
	} AodvT_Conn_Info;

// MHAVH 10/25/08
/*******************************************/
/***************** GEO TABLE ***************/
/*******************************************/

/* The geo table records for each valid destination */
/* its x and y geo coordinates.                     */
typedef struct
	{
	InetT_Address_Hash_Table_Handle	geo_table;
	} AodvT_Geo_Table;

typedef struct
	{
	InetT_Address		dst_address;		
	double				dst_x;
	double 				dst_y;
	double				insert_time;
	int                 type; // MHAVH - 03/17/09
	int					sequence_number;	//MKA 01/02/10
	} AodvT_Geo_Entry;
// MHAVH 10/25/08

// MKA 01/25/11
// These utility structs are here so that all of the aodv code can utilize them.
typedef struct Point2D
{
	double x;
	double y;
	
} Point2D;


typedef struct Rectangle
{
	Point2D lower_left;
	Point2D upper_left;
	Point2D upper_right;
	Point2D lower_right;
	
} Rectangle;



#if defined (__cplusplus)
} /* end of 'extern "C" {' */
#endif

/* End if for protection against multiple includes.	 */
#endif 	/*_AODV_H_INCLUDED_		 */ 
