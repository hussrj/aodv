/* aodv_packet_queue.ex.c */
/* C file for AODV Packet Queue APIs */


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
#include <manet.h>
#include <ip_addr_v4.h>

/***** Prototypes *****/
static void						aodv_packet_queue_size_stat_update (AodvT_Packet_Queue*);
static void						aodv_packet_queue_route_discovery_time_stat_update (AodvT_Packet_Queue*, List*);
static AodvT_Packet_Queue*		aodv_packet_queue_mem_alloc (InetT_Addr_Family);
static AodvT_Packet_Entry*		aodv_packet_queue_entry_mem_alloc (void);
static void						aodv_packet_queue_entry_mem_free (AodvT_Packet_Entry*);


AodvT_Packet_Queue*
aodv_packet_queue_create (int size, AodvT_Local_Stathandles* local_stat_handles_ptr, InetT_Addr_Family addr_family)
	{
	AodvT_Packet_Queue*		pkt_queue_ptr;
	
	/** Creates and allocates memory for	**/
	/** the packet queue which buffers data	**/
	/** packets while a route is being		**/
	/** discovered to the destination		**/
	FIN (aodv_packet_queue_create (<args>));
	
	pkt_queue_ptr = aodv_packet_queue_mem_alloc (addr_family);
	pkt_queue_ptr->max_queue_size = size;
	pkt_queue_ptr->stat_handle_ptr = local_stat_handles_ptr;
	
	FRET (pkt_queue_ptr);
	}
	

void
aodv_packet_queue_insert (AodvT_Packet_Queue* pkt_queue_ptr, Packet* data_pkptr, InetT_Address dest_addr)
	{
	List*						pkt_lptr;
	AodvT_Packet_Entry*			pkt_entry_ptr;
	void*						old_contents_ptr;
	AodvT_Global_Stathandles*	global_stathandle_ptr = OPC_NIL;
	
	/** Inserts a new data packet into the packet queue. If the queue is full	**/
	/** the oldest packet in the queue that has the same destination with the	**/
	/** new packet is dropped and the new packet inserted; or the new packet is	**/
	/** dropped if there is no packet to the same destination in the full queue.**/
	FIN (aodv_packet_queue_insert (<args>));
	
	/* Check if there are already other packets */
	/* waiting for this destination.			*/
	pkt_lptr = (List *) inet_addr_hash_table_item_get (pkt_queue_ptr->pkt_queue_table, &dest_addr);

	/* Is the queue already full?				*/
	if ((pkt_queue_ptr->max_queue_size != AODVC_INFINITE_PACKET_QUEUE) &&
		(pkt_queue_ptr->max_queue_size == pkt_queue_ptr->current_queue_size))
		{
		/* There is no more space in the queue to insert	*/
		/* any other packet. Delete the oldest packet in 	*/
		/* packet queue to the same destination to make		*/
		/* space for the new packet.						*/
		if (pkt_lptr == OPC_NIL)
			{
			/* There is no packet in the queue for the same	*/
			/* destination. Hence drop the new packet.		*/
			manet_rte_ip_pkt_destroy (data_pkptr);

			/* Update the local and global number of		*/
			/* packets dropped statistics.					*/	
			op_stat_write (pkt_queue_ptr->stat_handle_ptr->num_pkts_discard_shandle, 1.0);
			global_stathandle_ptr = aodv_support_global_stat_handles_obtain ();
			op_stat_write (global_stathandle_ptr->num_pkts_discard_global_shandle, 1.0);
			
			FOUT;
			}
		else
			{
			/* Drop the oldest packet to the same			*/
			/* destination, which is at the head of the		*/
			/* queue.										*/
			pkt_entry_ptr = (AodvT_Packet_Entry*) op_prg_list_remove (pkt_lptr, OPC_LISTPOS_HEAD);
			aodv_packet_queue_entry_mem_free (pkt_entry_ptr);

			/* Update the local and global number of		*/
			/* packets dropped statistics.					*/	
			op_stat_write (pkt_queue_ptr->stat_handle_ptr->num_pkts_discard_shandle, 1.0);
			global_stathandle_ptr = aodv_support_global_stat_handles_obtain ();
			op_stat_write (global_stathandle_ptr->num_pkts_discard_global_shandle, 1.0);

			/* Decrement the size of the packet queue.		*/
			pkt_queue_ptr->current_queue_size--;
	
			/* Update the packet queue size statistic.		*/
			aodv_packet_queue_size_stat_update (pkt_queue_ptr);
			}
		}
	
	if (pkt_lptr == OPC_NIL)
		{
		/* No entry exists for this destination	*/
		/* Create a queue for the destination.	*/
		pkt_lptr = op_prg_list_create ();
		inet_addr_hash_table_item_insert (pkt_queue_ptr->pkt_queue_table, &dest_addr, pkt_lptr, &old_contents_ptr);
		}
	
	/* Allocate memory to store the new packet	*/
	pkt_entry_ptr = aodv_packet_queue_entry_mem_alloc ();
	pkt_entry_ptr->pkptr = data_pkptr;
	pkt_entry_ptr->insert_time = op_sim_time ();
	
	/* Always insert at the tail of the list so	*/
	/* that the oldest packets are at the head 	*/
	/* of the list automatically				*/
	op_prg_list_insert (pkt_lptr, pkt_entry_ptr, OPC_LISTPOS_TAIL);
	pkt_queue_ptr->current_queue_size++;
	
	/* Update the packet queue size statistic	*/
	aodv_packet_queue_size_stat_update (pkt_queue_ptr);
	
	FOUT;
	}


List*
aodv_packet_queue_all_pkts_to_dest_get (AodvT_Packet_Queue* pkt_queue_ptr, InetT_Address dest_addr, 
											Boolean remove, Boolean update_discovery_time_stat)
	{
	AodvT_Packet_Entry*		pkt_entry_ptr;
	List*					pkt_lptr;
	List*					data_pkt_lptr;
	int						num_pkts, count;
	
	/** Based on the input flag either access or	**/
	/** remove packets from the queue 				**/
	FIN (aodv_packet_queue_all_pkts_to_dest_get (<args>));
	
	/* Based on the input flag either just access	*/
	/* or remove the packets from the packet queue	*/
	if (remove)
		{
		/* Remove the packets from the queue	*/
		data_pkt_lptr = (List *) inet_addr_hash_table_item_remove (pkt_queue_ptr->pkt_queue_table, &dest_addr);
		
		/* Return if there's no packet to remove */
		if (data_pkt_lptr == OPC_NIL)
			FRET (OPC_NIL);
		
		if (update_discovery_time_stat)
			{
			/* Write the statistic for the route discovery	*/
			/* time for this destination.					*/
			aodv_packet_queue_route_discovery_time_stat_update (pkt_queue_ptr, data_pkt_lptr);
			}
		
		if (op_prg_list_size (data_pkt_lptr) > 0)
			{
			/* Update the packet queue size statistic	*/
			aodv_packet_queue_size_stat_update (pkt_queue_ptr);
			}
		
		pkt_queue_ptr->current_queue_size -= op_prg_list_size (data_pkt_lptr);
		
		}
	else
		{
		/* Only access the packets in the queue	*/
		pkt_lptr = (List *) inet_addr_hash_table_item_get (pkt_queue_ptr->pkt_queue_table, &dest_addr);

		/* Create a list to return the packets	*/
		data_pkt_lptr = op_prg_list_create ();

		num_pkts = op_prg_list_size (pkt_lptr);
		
		for (count = 0; count < num_pkts; count++)
			{
			pkt_entry_ptr = (AodvT_Packet_Entry*) op_prg_list_access (pkt_lptr, count);
			op_prg_list_insert (data_pkt_lptr, pkt_entry_ptr->pkptr, OPC_LISTPOS_TAIL);
			}
		}
	
	FRET (data_pkt_lptr);
	}


int
aodv_packet_queue_num_pkts_get (AodvT_Packet_Queue* pkt_queue_ptr, InetT_Address dest_addr)
	{
	List*					pkt_lptr;
	
	FIN (aodv_packet_queue_num_pkts_get (<args>));
	
	/* Only access the packets in the queue	*/
	pkt_lptr = (List *) inet_addr_hash_table_item_get (pkt_queue_ptr->pkt_queue_table, &dest_addr);

	if (pkt_lptr == OPC_NIL)
		{
		FRET (0);
		}
	else
		{
		FRET (op_prg_list_size (pkt_lptr));
		}	  
	}		
	
	
void
aodv_packet_queue_all_pkts_to_dest_delete (AodvT_Packet_Queue* pkt_queue_ptr, InetT_Address dest_addr)
	{
	AodvT_Packet_Entry*			pkt_entry_ptr;
	AodvT_Global_Stathandles*	global_stathandle_ptr = OPC_NIL;
	List*						pkt_lptr;
	int							num_pkts;
	
	/** Deletes all packets to a specific destination	**/
	FIN (aodv_packet_queue_all_pkts_to_dest_delete (<args>));
	
	/* Remove the packets form the queue	*/
	pkt_lptr = (List *) inet_addr_hash_table_item_remove (pkt_queue_ptr->pkt_queue_table, &dest_addr);
	
	if (pkt_lptr == OPC_NIL)
		FOUT;
	
	/* Decrement the size of the queue	*/
	num_pkts = op_prg_list_size (pkt_lptr);
	pkt_queue_ptr->current_queue_size -= num_pkts;
	
	/* Update the packet queue size statistic	*/
	aodv_packet_queue_size_stat_update (pkt_queue_ptr);
	
	/* Update the number of packets dropped statistic	*/
	op_stat_write (pkt_queue_ptr->stat_handle_ptr->num_pkts_discard_shandle, (double) num_pkts);
	
	/* Get a handle to the global statistics	*/
	global_stathandle_ptr = aodv_support_global_stat_handles_obtain ();
	
	/* Update the global statistic for the number of pkts dropped	*/
	op_stat_write (global_stathandle_ptr->num_pkts_discard_global_shandle, (double) num_pkts);
	
	/* Free each entry in the list	*/
	while (num_pkts > 0)
		{
		pkt_entry_ptr = (AodvT_Packet_Entry*) op_prg_list_remove (pkt_lptr, OPC_LISTPOS_HEAD);
		aodv_packet_queue_entry_mem_free (pkt_entry_ptr);
		num_pkts--;
		}
	
	/* Free the list	*/
	op_prg_mem_free (pkt_lptr);
	
	FOUT;
	}


/*** Internally callable functions	***/

static void
aodv_packet_queue_route_discovery_time_stat_update (AodvT_Packet_Queue* pkt_queue_ptr, List* pkt_queue_lptr)
	{
	int							num_pkts, count;
	AodvT_Packet_Entry*			pkt_entry_ptr = OPC_NIL;
	AodvT_Global_Stathandles*	global_stathandle_ptr = OPC_NIL;
	double						first_enqueue_time = OPC_DBL_INFINITY;
	double						route_discovery_time;
	
	/** Updates the statistic for the route discovery time	**/
	FIN (aodv_packet_queue_route_discovery_time_stat_update (<args>));
	
	/* Get the number of packets for the	*/
	/* destination whose route has just 	*/
	/* been discovered						*/
	num_pkts = op_prg_list_size (pkt_queue_lptr);
	
	for (count = 0; count < num_pkts; count++)
		{
		/* Access each packet information	*/
		/* and determine the first packet	*/
		/* that was enqueued.				*/
		pkt_entry_ptr = (AodvT_Packet_Entry*) op_prg_list_access (pkt_queue_lptr, count);
		
		if (pkt_entry_ptr->insert_time < first_enqueue_time)
			{
			/* This entry is earlier than 	*/
			/* any other so far				*/
			first_enqueue_time = pkt_entry_ptr->insert_time;
		    }
		}
	
	/* The route discovery time is the time	*/
	/* difference between the first packet 	*/
	/* enqueued to this destination and the	*/
	/* current simulation time when the 	*/
	/* route was discovered					*/
	route_discovery_time = op_sim_time () - first_enqueue_time;
	
	/* Get a handle to the global statistics	*/
	global_stathandle_ptr = aodv_support_global_stat_handles_obtain ();
	
	/* Update the route discovery time for	*/
	/* a specific destination				*/
	op_stat_write (pkt_queue_ptr->stat_handle_ptr->route_discovery_time_shandle, route_discovery_time);
	
	/* Update the global route discovery time statistic	*/
	op_stat_write (global_stathandle_ptr->route_discovery_time_global_shandle, route_discovery_time);
	
	FOUT;
	}


static void
aodv_packet_queue_size_stat_update (AodvT_Packet_Queue* pkt_queue_ptr)
	{
	/** Updates the send buffer statistic	*/
	FIN (aodv_packet_queue_size_stat_update (<args>));
	
	/* Update the size of the send buffer local statistic	*/
	op_stat_write (pkt_queue_ptr->stat_handle_ptr->pkt_queue_size_shandle, (double) pkt_queue_ptr->current_queue_size);
	
	FOUT;
	}
		

static AodvT_Packet_Queue*
aodv_packet_queue_mem_alloc (InetT_Addr_Family addr_family)
	{
	AodvT_Packet_Queue*		pkt_queue_ptr;
	
	/** Allocates memory for the packet queue	**/
	FIN (aodv_packet_queue_mem_alloc (addr_family));
	
	pkt_queue_ptr = (AodvT_Packet_Queue*) op_prg_mem_alloc (sizeof (AodvT_Packet_Queue));
	if (addr_family == InetC_Addr_Family_v4)
		pkt_queue_ptr->pkt_queue_table = inet_addr_hash_table_create (16, 0);
	else
		pkt_queue_ptr->pkt_queue_table = inet_addr_hash_table_create (0, 16);
	pkt_queue_ptr->current_queue_size = 0;
	pkt_queue_ptr->max_queue_size = 0;
	
	FRET (pkt_queue_ptr);
	}


static AodvT_Packet_Entry*
aodv_packet_queue_entry_mem_alloc (void)
	{
	static Pmohandle		packet_entry_pmh;
	AodvT_Packet_Entry*		packet_entry_ptr = OPC_NIL;
	static Boolean			packet_entry_pmh_defined = OPC_FALSE;
	
	/** Allocates pooled memory for every packet entry	**/
	FIN (aodv_packet_queue_entry_mem_alloc (void));
	
	if (packet_entry_pmh_defined == OPC_FALSE)
		{
		/* Define the pool memory handle for route table entry	*/
		packet_entry_pmh = op_prg_pmo_define ("Packet Queue Entry", sizeof (AodvT_Packet_Entry), 32);
		packet_entry_pmh_defined = OPC_TRUE;
		}
	
	/* Allocate the route table entry from the pooled memory	*/
	packet_entry_ptr = (AodvT_Packet_Entry*) op_prg_pmo_alloc (packet_entry_pmh);
	
	FRET (packet_entry_ptr);
	}

static void
aodv_packet_queue_entry_mem_free (AodvT_Packet_Entry* pkt_entry_ptr)
	{
	/** Frees the packet entry **/
	FIN (aodv_packet_queue_entry_mem_free (<args>));
	
	/* Destroy the packet	*/
	manet_rte_ip_pkt_destroy (pkt_entry_ptr->pkptr);
	op_prg_mem_free (pkt_entry_ptr);
	
	FOUT;
	}
