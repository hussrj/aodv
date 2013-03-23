/* aodv_route_table.ex.c */
/* C file for AODV Route Table APIs */


/****************************************/
/*		Copyright (c) 1986-2011			*/
/*		by OPNET Technologies, Inc.		*/
/*		(A Delaware Corporation)		*/
/*	7255 Woodmont Av., Suite 250  		*/
/*     Bethesda, MD 20814, U.S.A.       */
/*			All Rights Reserved.		*/
/****************************************/

/***** Includes *****/
#include <opnet.h>
#include <stdarg.h>
#include <aodv.h>
#include <aodv_pkt_support.h>
#include <aodv_ptypes.h>
#include <ip_addr_v4.h>
#include <ip_cmn_rte_table.h>

/***** Prototypes *****/

static AodvT_Route_Entry*		aodv_route_table_entry_mem_alloc (void);
static void						aodv_route_table_entry_mem_free (AodvT_Route_Entry*);
extern void						aodv_rte_entry_expiry_handle (void*, int);


AodvT_Route_Table*
aodv_route_table_create (IpT_Cmn_Rte_Table* cmn_rte_table_ptr, IpT_Rte_Proc_Id proto_id, double expiry_time, 
					     double delete_period, AodvT_Local_Stathandles* local_stat_ptr, InetT_Addr_Family hash_key_addr_family)
	{
	AodvT_Route_Table*		route_table_ptr;
	
	/** Creates and allocates memory for	**/
	/** the AODV route table				**/
	FIN (aodv_route_table_create (cmn_rte_table_ptr, proto_id, expiry_time, delete_period, local_stat_ptr, hash_key_addr_family));
	
	route_table_ptr = (AodvT_Route_Table *) op_prg_mem_alloc (sizeof (AodvT_Route_Table));
	route_table_ptr->ip_cmn_rte_table_ptr = cmn_rte_table_ptr;
	route_table_ptr->aodv_protocol_id     = proto_id;
	route_table_ptr->route_expiry_time    = expiry_time;
	route_table_ptr->delete_period        = delete_period;
	route_table_ptr->stat_handles_ptr     = local_stat_ptr;
	route_table_ptr->active_route_count   = 0;
	if (hash_key_addr_family == InetC_Addr_Family_v4)
		route_table_ptr->route_table = inet_addr_hash_table_create (100, 0);
	else
		route_table_ptr->route_table = inet_addr_hash_table_create (0, 100);
	
	FRET (route_table_ptr);
	}


AodvT_Route_Entry*
aodv_route_table_entry_create (AodvT_Route_Table* route_table_ptr, InetT_Address dest_addr, InetT_Subnet_Mask subnet_mask,
	InetT_Address next_hop_addr, IpT_Port_Info out_port_info, int num_hops, int dest_seq_num, double expiry_time)
	{
	AodvT_Route_Entry*			route_entry_ptr;
	AodvT_Global_Stathandles*	global_stathandle_ptr;
	void*						old_contents_ptr;
	
	/** Adds a new route table entry	**/
	/** in the route table				**/
	FIN (aodv_route_table_entry_create (<args>));

	/* Allocate memory for the route entry	*/
	route_entry_ptr = aodv_route_table_entry_mem_alloc ();
	route_entry_ptr->dest_prefix = ip_cmn_rte_table_dest_prefix_create (dest_addr, subnet_mask);
	route_entry_ptr->dest_seq_num = dest_seq_num;
	
	if (dest_seq_num != AODVC_DEST_SEQ_NUM_INVALID)
		route_entry_ptr->valid_dest_sequence_number_flag = OPC_TRUE;
	else
		route_entry_ptr->valid_dest_sequence_number_flag = OPC_FALSE;
	
	route_entry_ptr->route_entry_state = AodvC_Valid_Route;
	route_entry_ptr->next_hop_addr = inet_address_copy (next_hop_addr);
	route_entry_ptr->next_hop_port_info = out_port_info;
	route_entry_ptr->hop_count = num_hops;
	route_entry_ptr->route_expiry_time = op_sim_time () + expiry_time;
	
	/* This event will be processed by aodv_rte_entry_expiry_handle */
	/* function when the timer expires.								*/
	route_entry_ptr->route_expiry_evhandle = op_intrpt_schedule_call (route_entry_ptr->route_expiry_time,
		AODVC_ROUTE_ENTRY_INVALID,	aodv_rte_entry_expiry_handle, route_entry_ptr);
	
	/* Set the route entry for this destination	*/
	/* in the route table						*/
	inet_addr_hash_table_item_insert (route_table_ptr->route_table, &dest_addr, route_entry_ptr, &old_contents_ptr);
	
	/* Insert the route in the IP common route table	*/
	Inet_Cmn_Rte_Table_Entry_Add_Options (route_table_ptr->ip_cmn_rte_table_ptr, OPC_NIL, route_entry_ptr->dest_prefix, 
		route_entry_ptr->next_hop_addr, route_entry_ptr->next_hop_port_info, route_entry_ptr->hop_count, 
		route_table_ptr->aodv_protocol_id, 1, OPC_NIL, IPC_CMN_RTE_TABLE_ENTRY_ADD_INDIRECT_NEXTHOP_OPTION);
	
	/* Update the size of the route table	*/
	route_table_ptr->active_route_count++;

	/* Update the route table size statistic	*/
	op_stat_write (route_table_ptr->stat_handles_ptr->route_table_size_shandle, route_table_ptr->active_route_count);
	
	/* Update the statistic for the number of hops	*/
	op_stat_write (route_table_ptr->stat_handles_ptr->num_hops_shandle, route_entry_ptr->hop_count);	
	
	/* Get a handle to the global statistics	*/
	global_stathandle_ptr = aodv_support_global_stat_handles_obtain ();
	
	/* Update the global statistic for the number of hops	*/
	op_stat_write (global_stathandle_ptr->num_hops_global_shandle, route_entry_ptr->hop_count);

	FRET (route_entry_ptr);
	}


AodvT_Route_Entry*
aodv_route_table_entry_get (AodvT_Route_Table* route_table_ptr, InetT_Address dest_addr)
	{
	AodvT_Route_Entry*			route_entry_ptr = OPC_NIL;
		
	/** Determines whether an entry exists	**/
	/** in the route table for a destination**/
	FIN (aodv_route_table_entry_get (<args>));
	
	/* Get the entry for this destination	*/
	route_entry_ptr = (AodvT_Route_Entry *) inet_addr_hash_table_item_get (route_table_ptr->route_table, &dest_addr);
	
	FRET (route_entry_ptr);
	}


Compcode
aodv_route_table_entry_param_get (AodvT_Route_Entry* route_entry_ptr, int param, void* value_ptr)
	{
	int*						int_value_ptr;
	double*						dbl_value_ptr;
	Boolean*					bool_value_ptr;
	AodvC_Route_Entry_State*	route_state_ptr;
	InetT_Address*				ip_addr_ptr;
	List**						list_pptr;
	IpT_Port_Info*				port_info_ptr;
	
	/** Access any field in the route table entry	**/
	FIN (aodv_route_table_entry_param_get (<args>));
	
	if (route_entry_ptr == OPC_NIL)
		{
		FRET (OPC_COMPCODE_FAILURE);
		}
	
	/* Based on the input parameter, return	*/
	/* the appropriate parameter			*/
	switch (param)
		{
		case (AODVC_ROUTE_ENTRY_DEST_SEQ_NUM):
			{
			int_value_ptr = (int*) value_ptr;
			*int_value_ptr = route_entry_ptr->dest_seq_num;
			break;
			}
			
		case (AODVC_ROUTE_ENTRY_VALID_SEQ_NUM_FLAG):
			{
			bool_value_ptr = (Boolean*) value_ptr;
			*bool_value_ptr = route_entry_ptr->valid_dest_sequence_number_flag;
			break;
			}
			
		case (AODVC_ROUTE_ENTRY_ROUTE_ENTRY_STATE):
			{
			route_state_ptr = (AodvC_Route_Entry_State*) value_ptr;
			*route_state_ptr = route_entry_ptr->route_entry_state;
			break;
			}
			
		case (AODVC_ROUTE_ENTRY_NEXT_HOP_ADDR):
			{
			ip_addr_ptr = (InetT_Address*) value_ptr;
			*ip_addr_ptr = route_entry_ptr->next_hop_addr;
			break;
			}
			
		case (AODVC_ROUTE_ENTRY_OUT_PORT_INFO):
			{
			port_info_ptr = (IpT_Port_Info*) value_ptr;
			*port_info_ptr = route_entry_ptr->next_hop_port_info;
			break;
			}
		   
		case (AODVC_ROUTE_ENTRY_HOP_COUNT):
			{
			int_value_ptr = (int*) value_ptr;
			*int_value_ptr = route_entry_ptr->hop_count;
			break;
			}
			
		case (AODVC_ROUTE_ENTRY_PRECURSOR_LIST):
			{
			list_pptr = (List**) value_ptr;
			*list_pptr = route_entry_ptr->precursor_lptr;
			break;
			}
			
		case (AODVC_ROUTE_ENTRY_EXPIRY_TIME):
			{
			dbl_value_ptr = (double*) value_ptr;
			*dbl_value_ptr = route_entry_ptr->route_expiry_time;
			break;
			}
		
		default :
			{
			/* Unknown input parameter	*/
			FRET (OPC_COMPCODE_FAILURE);
			}
		}
		
	FRET (OPC_COMPCODE_SUCCESS);
	}
			

Compcode
aodv_route_table_entry_param_set (AodvT_Route_Entry* route_entry_ptr, int param, ...)
	{
	va_list						arg_list;
	
	/** Set the fields of the route table entry	**/
	FIN (aodv_route_table_entry_param_set (<args>));
	
	if (route_entry_ptr == OPC_NIL)
		FRET (OPC_COMPCODE_FAILURE);
	
	/* Initialize the list of arguments. Though a list arguments may	*/
    /* not always be passed this approach will help us identify the		*/
    /* data type of the parameters and appropriately cast it.			*/
    va_start (arg_list, param);
	
	/* Based on the input parameter, set	*/
	/* the appropriate parameter			*/
	switch (param)
		{
		case (AODVC_ROUTE_ENTRY_DEST_SEQ_NUM):
			{
			route_entry_ptr->dest_seq_num = va_arg (arg_list, int);
			break;
			}
			
		case (AODVC_ROUTE_ENTRY_VALID_SEQ_NUM_FLAG):
			{
			route_entry_ptr->valid_dest_sequence_number_flag = va_arg (arg_list, Boolean);
			break;
			}
			
		default :
			{
			/* Unknown input parameter	*/
			FRET (OPC_COMPCODE_FAILURE);
			}
		}
	
	FRET (OPC_COMPCODE_SUCCESS);
	}


Compcode
aodv_route_table_precursor_add (AodvT_Route_Entry* route_entry_ptr, InetT_Address precursor_addr)
	{
	InetT_Address*				precursor_addr_ptr;
	InetT_Address*				pre_addr_ptr;
	InetT_Address				existing_precursor_addr;
	int 						num, size;
	Boolean						FOUND = OPC_FALSE;
	
	/** Adds a percursor node to the entry in	**/
	/** the route table for a destination		**/
	FIN (aodv_route_table_precursor_add (<args>));
	
	if (route_entry_ptr == OPC_NIL)
		FRET (OPC_COMPCODE_FAILURE);
		
	/* Insert this precursor node in the list					*/
	/* Here we need to check and insert the precursor addr		*/
	/* First check if the precursor addr is already in the list,*/
	/* if yes, then don't insert, if no then insert.			*/	
	size = op_prg_list_size (route_entry_ptr->precursor_lptr);
	for(num = 0; num < size; num++)
		{
		pre_addr_ptr = (InetT_Address*) op_prg_list_access (route_entry_ptr->precursor_lptr, num);
		existing_precursor_addr = *pre_addr_ptr;
		
		if(inet_address_equal (precursor_addr, existing_precursor_addr))
			FOUND = OPC_TRUE;		
		}
		   
	if(FOUND == OPC_FALSE)
		{		
		precursor_addr_ptr = inet_address_create_dynamic (precursor_addr);
		op_prg_list_insert (route_entry_ptr->precursor_lptr, precursor_addr_ptr, OPC_LISTPOS_TAIL);
		}
	
	FRET (OPC_COMPCODE_SUCCESS);
	}


Compcode
aodv_route_table_entry_next_hop_update (AodvT_Route_Table* route_table_ptr, AodvT_Route_Entry* route_entry_ptr,
	InetT_Address new_next_hop_addr, int new_hop_count, IpT_Port_Info new_out_port_info)
	{
	AodvC_Route_Entry_State 	route_entry_state;
	
	/** Updates the route table entry with the new	**/
	/** next hop parameters and metric				**/
	FIN (aodv_route_table_entry_next_hop_update (<args>));
	
	route_entry_state = route_entry_ptr->route_entry_state;
	
	if (route_entry_state == AodvC_Valid_Route)
		{
		/* Update the common route table with the new information	*/
		Inet_Cmn_Rte_Table_Entry_Update (route_table_ptr->ip_cmn_rte_table_ptr, route_entry_ptr->dest_prefix,
			route_entry_ptr->next_hop_addr, route_table_ptr->aodv_protocol_id, new_next_hop_addr, new_out_port_info,
			new_hop_count, OPC_NIL);
		}

	/* Free the old information	*/
	inet_address_destroy (route_entry_ptr->next_hop_addr);
	
	/* Set the new information	*/
	route_entry_ptr->next_hop_addr = new_next_hop_addr;
	
	FRET (OPC_COMPCODE_SUCCESS);
	}


Compcode
aodv_route_table_entry_state_set (AodvT_Route_Table* route_table_ptr, AodvT_Route_Entry* route_entry_ptr, InetT_Address dest_addr, 
								  AodvC_Route_Entry_State route_entry_state)
	{	
	/** This function changes the state of a route table	**/
	/** entry. It can either set it to be a valid route or	**/
	/** and invalid route. The function appropriately 		**/
	/** handles the necessary functions with these states	**/
	FIN (aodv_route_table_entry_state_set (<args>));
	
	if (route_entry_ptr == OPC_NIL)
		FRET (OPC_COMPCODE_FAILURE);
	
	if (route_entry_state == AodvC_Valid_Route)
		{
		/* The route entry that already exists is	*/
		/* being set back to a valid route. Insert	*/
		/* this route back into the IP common		*/
		/* route table								*/
		Inet_Cmn_Rte_Table_Entry_Add_Options (route_table_ptr->ip_cmn_rte_table_ptr, OPC_NIL, route_entry_ptr->dest_prefix, 
			route_entry_ptr->next_hop_addr, route_entry_ptr->next_hop_port_info, route_entry_ptr->hop_count, 
			route_table_ptr->aodv_protocol_id, 1, OPC_NIL, IPC_CMN_RTE_TABLE_ENTRY_ADD_INDIRECT_NEXTHOP_OPTION);
		
		/* Set the state of the route to valid	*/
		route_entry_ptr->route_entry_state = AodvC_Valid_Route;
		
		/* Update the size of the route table	*/
		/* The size indicates the number of 	*/
		/* active routes that are present		*/
		route_table_ptr->active_route_count++;
		}
	
	/* Request is to invalidate the route. Perform the	*/
	/* operation unless the route is already invalid.	*/
	else if (route_entry_ptr->route_entry_state != AodvC_Invalid_Route)
		{
		/* The route entry is become invalid. Delete	*/
		/* the route from the common route table. Set	*/
		/* the state of the route to invalid in the		*/
		/* AODV route table. Do not delete it from the	*/
		/* AODV route table as it is still needed to 	*/
		/* keep information about the soft state		*/
		Inet_Cmn_Rte_Table_Route_Delete (route_table_ptr->ip_cmn_rte_table_ptr, route_entry_ptr->dest_prefix, 
											route_table_ptr->aodv_protocol_id);
		
		/* Set the state of the route to valid	*/
		route_entry_ptr->route_entry_state = AodvC_Invalid_Route;
		
		/* Increament the dest seq num for the expired entry */
		route_entry_ptr->dest_seq_num++;
				
		/* Update the expiry time of the route to be delete period	*/
		aodv_route_table_entry_expiry_time_update (route_entry_ptr, dest_addr, route_table_ptr->delete_period, 
													AODVC_ROUTE_ENTRY_EXPIRED);
		
		/* Update the size of the route table	*/
		/* The size indicates the number of 	*/
		/* active routes that are present		*/
		route_table_ptr->active_route_count--;
		}
	
	/* Update the route table size statistic	*/
	op_stat_write (route_table_ptr->stat_handles_ptr->route_table_size_shandle, route_table_ptr->active_route_count);
	
	FRET (OPC_COMPCODE_SUCCESS);
	}
	

Compcode
aodv_route_table_entry_delete (AodvT_Route_Table* route_table_ptr, InetT_Address dest_addr)
	{
	AodvT_Route_Entry*			route_entry_ptr = OPC_NIL;
	
	/** Deletes an entry from the AODV route table	**/
	/** If an entry is deleted from the AODV table,	**/
	/** it should have already been removed from 	**/
	/** the IP common route table when the entry 	**/
	/** was set to invalid							**/
	FIN (aodv_route_table_entry_delete (<args>));
	
	/* Remove the entry from the AODV route table	*/
	route_entry_ptr = (AodvT_Route_Entry *) inet_addr_hash_table_item_remove (route_table_ptr->route_table, &dest_addr);
	
	if (route_entry_ptr == OPC_NIL)
		FRET (OPC_COMPCODE_FAILURE);
	
	/* Free the memory for thie route table entry	*/
	aodv_route_table_entry_mem_free (route_entry_ptr);
	
	FRET (OPC_COMPCODE_SUCCESS);
	}


void
aodv_route_table_entry_expiry_time_update (AodvT_Route_Entry* route_entry_ptr, InetT_Address dest_addr, double timeout, int code)
	{		
	/** Update the expiry time for a route in the route table	**/
	FIN (aodv_route_table_entry_expiry_time_update (<args>));
	
	/* Cancel the previously scheduled event unless it is the	*/
	/* current event, which made us invalidate the route and	*/
	/* restart the expiry timer for deletion.					*/
	op_ev_cancel_if_pending (route_entry_ptr->route_expiry_evhandle);

	/* Update the expiry time for this route entry	*/
	route_entry_ptr->route_expiry_time = op_sim_time () + timeout;
	
	/* This event will be processed by aodv_rte_entry_expiry_handle */
	/* function when the timer expires.								*/
	route_entry_ptr->route_expiry_evhandle = op_intrpt_schedule_call (route_entry_ptr->route_expiry_time, code, 
		aodv_rte_entry_expiry_handle, route_entry_ptr);
	
	FOUT;
	}


static AodvT_Route_Entry*
aodv_route_table_entry_mem_alloc (void)
	{
	static Pmohandle		route_table_entry_pmh;
	AodvT_Route_Entry*		route_table_entry_ptr = OPC_NIL;
	static Boolean			route_table_entry_pmh_defined = OPC_FALSE;
	
	/** Allocates pooled memory for a route table entry	**/
	FIN (aodv_route_table_entry_mem_alloc (void));
	
	if (route_table_entry_pmh_defined == OPC_FALSE)
		{
		/* Define the pool memory handle for route table entry	*/
		route_table_entry_pmh = op_prg_pmo_define ("Route Table Entry", sizeof (AodvT_Route_Entry), 32);
		route_table_entry_pmh_defined = OPC_TRUE;
		}
	
	/* Allocate the route table entry from the pooled memory	*/
	route_table_entry_ptr = (AodvT_Route_Entry*) op_prg_pmo_alloc (route_table_entry_pmh);
	
	/* Create the precursor list.								*/
	route_table_entry_ptr->precursor_lptr = op_prg_list_create ();
	
	FRET (route_table_entry_ptr);
	}


static void
aodv_route_table_entry_mem_free (AodvT_Route_Entry* route_entry_ptr)
	{
	int					num_elements;
	InetT_Address*		precursor_addr_ptr;
	
	/** Frees the memory for the route table entry	**/
	FIN (aodv_route_table_entry_mem_free (<args>));
	
	/* Free the next hop address	*/
	inet_address_destroy (route_entry_ptr->next_hop_addr);
	
	/* Get the size of the precursor list	*/
	num_elements = op_prg_list_size (route_entry_ptr->precursor_lptr);
	
	/* Free the elements of the list	*/
	while (num_elements > 0)
		{
		precursor_addr_ptr = (InetT_Address*) op_prg_list_remove (route_entry_ptr->precursor_lptr, OPC_LISTPOS_HEAD);
		inet_address_destroy_dynamic (precursor_addr_ptr);
		num_elements--;
		}
	
	/* Free the list	*/
	op_prg_mem_free (route_entry_ptr->precursor_lptr);
	
	/* Free the destination prefix	*/
	ip_cmn_rte_table_dest_prefix_destroy (route_entry_ptr->dest_prefix);	
	
	/* Free the route entry	*/
	op_prg_mem_free (route_entry_ptr);
	
	FOUT;
	}
