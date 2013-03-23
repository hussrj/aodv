/* aodv_support.ex.c */
/* C support file for AODV support functions */


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
#include <aodv.h>
#include <aodv_ptypes.h>


AodvT_Global_Stathandles*
aodv_support_global_stat_handles_obtain (void)
	{
	static Boolean		   					stat_handles_registered = OPC_FALSE;
	static AodvT_Global_Stathandles*		stat_handle_ptr = OPC_NIL;
	
	/** Registers the global statistics and returns a	**/
	/** handle to the global statistics					**/
	FIN (aodv_support_global_stat_handles_obtain (void));
	
	if (stat_handles_registered == OPC_FALSE)
		{
		/* The statistic handles have not yet been registered	*/
		/* Register the global statistic handles				*/
		stat_handle_ptr = (AodvT_Global_Stathandles*) op_prg_mem_alloc (sizeof (AodvT_Global_Stathandles));
		
		stat_handle_ptr->route_discovery_time_global_shandle = op_stat_reg ("AODV.Route Discovery Time", OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
		stat_handle_ptr->num_hops_global_shandle = op_stat_reg ("AODV.Number of Hops per Route", OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
		stat_handle_ptr->num_pkts_discard_global_shandle = op_stat_reg ("AODV.Total Packets Dropped", OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
		stat_handle_ptr->rte_traf_rcvd_bps_global_shandle = op_stat_reg ("AODV.Routing Traffic Received (bits/sec)", OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
		stat_handle_ptr->rte_traf_rcvd_pps_global_shandle = op_stat_reg ("AODV.Routing Traffic Received (pkts/sec)", OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
		stat_handle_ptr->rte_traf_sent_bps_global_shandle = op_stat_reg ("AODV.Routing Traffic Sent (bits/sec)", OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
		stat_handle_ptr->rte_traf_sent_pps_global_shandle = op_stat_reg ("AODV.Routing Traffic Sent (pkts/sec)", OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
		stat_handle_ptr->total_requests_sent_global_shandle = op_stat_reg ("AODV.Total Route Requests Sent", OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
		stat_handle_ptr->total_replies_sent_global_shandle = op_stat_reg ("AODV.Total Route Replies Sent", OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
		stat_handle_ptr->total_replies_sent_from_dest_global_shandle = op_stat_reg ("AODV.Total Replies Sent from Destination", OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
		stat_handle_ptr->total_cached_replies_sent_global_shandle = op_stat_reg ("AODV.Total Cached Replies Sent", OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
		stat_handle_ptr->total_route_errors_sent_global_shandle = op_stat_reg ("AODV.Total Route Errors Sent", OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
				
		/* Set the flag to indicate that the statistics	*/
		/* have been registered							*/
		stat_handles_registered = OPC_TRUE;
		}
	
	FRET (stat_handle_ptr);
	}

Boolean
aodv_support_hello_efficiency_sim_attr_get (void)
	{
	static Boolean		hello_efficiency_mode;
	static Boolean		attr_read = OPC_FALSE;
	
	/** Reads the global attribute	**/
	FIN (aodv_support_hello_efficiency_sim_attr_get (void));
	
	if (attr_read == OPC_FALSE)
		{
		/* Read the global attribute once	*/
		op_ima_sim_attr_get (OPC_IMA_TOGGLE, "AODV Hello Efficiency", &hello_efficiency_mode);
		
		/* Set the flag so that	*/
		/* it is not read again	*/
		attr_read = OPC_TRUE;
		}
	
	FRET (hello_efficiency_mode);
	}


void
aodv_support_routing_traffic_sent_stats_update (AodvT_Local_Stathandles* stat_handle_ptr, AodvT_Global_Stathandles* global_stathandle_ptr, Packet* pkptr)
	{
	OpT_Packet_Size			pkt_size;
	
	/** Updates the routing traffic sent statistics	**/
	/** in both bits/sec and pkts/sec				**/
	FIN (aodv_support_routing_traffic_sent_stats_update (<args>));
	
	/* Get the size of the packet	*/
	pkt_size = op_pk_total_size_get (pkptr);
	
	/* Update the local routing traffic sent stat in bps 	*/
	op_stat_write (stat_handle_ptr->rte_traf_sent_bps_shandle, pkt_size);
	op_stat_write (stat_handle_ptr->rte_traf_sent_bps_shandle, 0.0);
	
	/* Update the local routing traffic sent stat in pps 	*/
	op_stat_write (stat_handle_ptr->rte_traf_sent_pps_shandle, 1.0);
	op_stat_write (stat_handle_ptr->rte_traf_sent_pps_shandle, 0.0);
	
	/* Update the global routing traffic sent stat in bps 	*/
	op_stat_write (global_stathandle_ptr->rte_traf_sent_bps_global_shandle, pkt_size);
	op_stat_write (global_stathandle_ptr->rte_traf_sent_bps_global_shandle, 0.0);
	
	/* Update the global routing traffic sent stat in pps 	*/
	op_stat_write (global_stathandle_ptr->rte_traf_sent_pps_global_shandle, 1.0);
	op_stat_write (global_stathandle_ptr->rte_traf_sent_pps_global_shandle, 0.0);
	
	FOUT;
	}


void
aodv_support_routing_traffic_received_stats_update (AodvT_Local_Stathandles* stat_handle_ptr, AodvT_Global_Stathandles* global_stathandle_ptr, Packet* pkptr)
	{
	OpT_Packet_Size			pkt_size;
	
	/** Updates the routing traffic sent statistics	**/
	/** in both bits/sec and pkts/sec				**/
	FIN (aodv_support_routing_traffic_received_stats_update (<args>));
	
	/* Get the size of the packet	*/
	pkt_size = op_pk_total_size_get (pkptr);
	
	/* Update the local routing traffic sent stat in bps 	*/
	op_stat_write (stat_handle_ptr->rte_traf_rcvd_bps_shandle, pkt_size);
	op_stat_write (stat_handle_ptr->rte_traf_rcvd_bps_shandle, 0.0);
	
	/* Update the local routing traffic sent stat in pps 	*/
	op_stat_write (stat_handle_ptr->rte_traf_rcvd_pps_shandle, 1.0);
	op_stat_write (stat_handle_ptr->rte_traf_rcvd_pps_shandle, 0.0);
	
	/* Update the global routing traffic sent stat in bps 	*/
	op_stat_write (global_stathandle_ptr->rte_traf_rcvd_bps_global_shandle, pkt_size);
	op_stat_write (global_stathandle_ptr->rte_traf_rcvd_bps_global_shandle, 0.0);
	
	/* Update the global routing traffic sent stat in pps 	*/
	op_stat_write (global_stathandle_ptr->rte_traf_rcvd_pps_global_shandle, 1.0);
	op_stat_write (global_stathandle_ptr->rte_traf_rcvd_pps_global_shandle, 0.0);
	
	FOUT;
	}


void
aodv_support_route_reply_sent_stats_update (AodvT_Local_Stathandles* stat_handle_ptr, AodvT_Global_Stathandles* global_stathandle_ptr, 
												Boolean dest_route_reply)
	{
	/** Updates the statistics related to the route replies sent	**/
	FIN (aodv_support_route_reply_sent_stats_update (<args>));
	
	/* Based on whether the route reply is a 	*/
	/* cached route reply or a route reply from	*/
	/* the destination, update the statistic	*/
	
	if (!dest_route_reply)
		{
		/* Update the statistic for the total	*/
		/* number of cached route replies sent	*/
		/* both locally and globally			*/
		op_stat_write (stat_handle_ptr->total_cached_replies_sent_shandle, 1.0);
		op_stat_write (global_stathandle_ptr->total_cached_replies_sent_global_shandle, 1.0);
		}
	else
		{
		/* Update the statistic for the total	*/
		/* number of route replies sent from 	*/
		/* the destination both local and global*/
		op_stat_write (stat_handle_ptr->total_replies_sent_from_dest_shandle, 1.0);
		op_stat_write (global_stathandle_ptr->total_replies_sent_from_dest_global_shandle, 1.0);
		}
	
	/* Update the statistic for the total number of	*/
	/* route replies sent both locally and globally	*/
	op_stat_write (stat_handle_ptr->total_replies_sent_shandle, 1.0);
	op_stat_write (global_stathandle_ptr->total_replies_sent_global_shandle, 1.0);
	
	FOUT;
	}


void
aodv_support_route_table_print_to_string (AodvT_Route_Table* route_table_ptr, InetT_Addr_Family addr_family)
	{
	AodvT_Route_Entry*		route_entry_ptr = OPC_NIL;
	List*					routes_lptr = OPC_NIL;
	InetT_Address			dest_addr;
	int						num_routes, count;
	char					message_str [10000] = "";
	char*					final_str = OPC_NIL;
	int						add_elements;
	char					dest_addr_str [INETC_ADDR_STR_LEN];
	char					next_addr_str [INETC_ADDR_STR_LEN];
	char					rt_entry[128];
	
	
	/** Prints the AODV Route Table maintained at this node	**/
	FIN (aodv_support_route_table_print_to_string (<args>));

	/* Get the size of each string	*/
	add_elements = strlen (message_str);
		
	/* Append the new string to the return string	*/
	final_str = (char*) aodv_array_elements_add (final_str, 0, (add_elements + 1), sizeof (char));
	strcat (final_str, message_str);
	
	/* Get the number of routes in the route table	*/
	routes_lptr = (List *) inet_addr_hash_table_item_list_get (route_table_ptr->route_table, addr_family);
	num_routes = op_prg_list_size (routes_lptr);
	
	if (num_routes == 0)
		{
		printf("\n Empty Route Table \n");
		}
	else
		{
		
		printf("=========================================================================================\n");
		printf("-                              ROUTE TABLE                                               \n");
		printf("=========================================================================================\n");
		printf("Dest. Node	Dest Seq.Num	Next Hop	HopCount	Rt.Expiry Time	Rt.Entry Status	 \n");
		printf("----------	------------	--------	--------	--------------	--------------   \n");
		
		for (count = 0; count < num_routes; count++)
		{
		
		/* For each route entry	*/
		route_entry_ptr = (AodvT_Route_Entry *) op_prg_list_access (routes_lptr, count);
		
		if(route_entry_ptr == OPC_NIL)
			{
			printf("One Entry but Empty Route Table  \n");
			continue;
			}
		else
			{			
			if (route_entry_ptr->route_entry_state == AodvC_Valid_Route)
				{
				sprintf(rt_entry, "VALID");
				}
			else if(route_entry_ptr->route_entry_state == AodvC_Invalid_Route)
				{
				sprintf(rt_entry, "INVALID");
				}
			else 
				{
				sprintf(rt_entry, "UNDEFINED");
				}
			
			dest_addr = ip_cmn_rte_table_dest_prefix_addr_get (route_entry_ptr->dest_prefix);
			inet_address_print (dest_addr_str, dest_addr);
			inet_address_print (next_addr_str, route_entry_ptr->next_hop_addr);
			printf ("%s\t%d\t\t%s\t%d\t%18.5f\t%s\n", dest_addr_str, route_entry_ptr->dest_seq_num, next_addr_str,
				route_entry_ptr->hop_count, route_entry_ptr->route_expiry_time, rt_entry);	
			}
		}
		printf("---------------------------------------------------------------------------------------------\n");
		
		}/* close else */		
	
	
	/* Free the list cells and list object. */
	while (op_prg_list_size (routes_lptr) > 0)
		op_prg_list_remove (routes_lptr, OPC_LISTPOS_HEAD);

	op_prg_mem_free (routes_lptr);
		
	FOUT;
	
	}


void*
aodv_array_elements_add (void* array, int count, int add_count, int elem_size)
	{
	void*		new_array;

	/** This procedure allocates a new array of elements of	**/
	/** the specified size and copies the elements of the	**/
	/** original array into the new array.  The old array	**/
	/** is deallocated and the new array is returned.		**/
	FIN (aodv_array_elements_add (<args>));

	/* Allocate the array. If this fails, the called 	*/
	/* procedure will handle the error.					*/
	new_array = op_prg_mem_alloc ((count + add_count) * elem_size);

	/* If there were any elements previously, copy 		*/
	/* them into the new array.							*/
	if (count != 0)
		{
		/* Copy the old array into the new array. */
		op_prg_mem_copy (array, new_array, count * elem_size);

		/* Deallocate the old array. */
		op_prg_mem_free (array);
		}

	/* Return the new array pointer. */
	FRET (new_array);
	}

void
aodv_support_conn_table_print_to_string (InetT_Address_Hash_Table_Handle neighbor_conn_table, InetT_Addr_Family addr_family)
	{
	List*				neighbors_lptr = OPC_NIL;
	char				address [INETC_ADDR_STR_LEN];
	AodvT_Conn_Info*	neighbor_conn_info_ptr = OPC_NIL;
	int 				num_neighbors;
	int 				count;

	/** Prints the neighbor connectivity table	**/
	FIN (aodv_support_conn_table_print_to_string (<args>));
	
	/* Get the list and number of the neighbors in connectivity hash table */
	neighbors_lptr = (List*) inet_addr_hash_table_item_list_get (neighbor_conn_table, addr_family);
	num_neighbors = op_prg_list_size (neighbors_lptr);
	
	if (num_neighbors == 0)
		{
		printf("Neighbor Connectivity Table Empty \n");
		}
	else
		{
		printf("Num of neighbors: %d\n", num_neighbors);
		printf("Neighbor IP Address	LastHelloRecv Time \n");
		printf("-------------------	------------------ \n");
		
		for (count = 0; count < num_neighbors; count++)
			{
			/* Access the connectivity information of each neighbor	*/
			neighbor_conn_info_ptr = (AodvT_Conn_Info*) op_prg_list_access (neighbors_lptr, count);		
		
			inet_address_print (address, neighbor_conn_info_ptr->neighbor_address);
		
			printf(" %s\t%18.5f \n", address, neighbor_conn_info_ptr->last_hello_received_time);

			}
		printf("------------------------------------------------------------------\n");
		}
	
	/* Free list cells and list object.	*/
	while (op_prg_list_size (neighbors_lptr) > 0)
		op_prg_list_remove (neighbors_lptr, OPC_LISTPOS_HEAD);

	op_prg_mem_free (neighbors_lptr);
	
	FOUT;
	}
