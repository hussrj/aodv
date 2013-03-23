/* aodv_request_table.ex.c */
/* C file for AODV Request Table APIs */


/****************************************/
/*		Copyright (c) 1987-2008			*/
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
#include <ip_addr_v4.h>
#include <prg_bin_hash.h>

/***** Prototypes *****/
static AodvT_Orig_Request_Entry*	aodv_request_table_orig_entry_mem_alloc (void);
static AodvT_Forward_Request_Entry*	aodv_request_table_forward_entry_mem_alloc (void);
extern void							aodv_rte_rreq_timer_expiry_handle (void*, int);
extern void							aodv_rte_forward_request_delete (void* addr_str, int req_id);

AodvT_Request_Table*
aodv_request_table_create (double expiry_time, int max_retries, InetT_Addr_Family addr_family)
	{
	AodvT_Request_Table*		req_table_ptr;
	
	/** Allocates and initializes the request table	**/
	FIN (aodv_request_table_create (<args>));
	
	/* Allocate memory.								*/
	req_table_ptr = (AodvT_Request_Table*) op_prg_mem_alloc (sizeof (AodvT_Request_Table));
	
	/* Create the Hash tables.						*/
	req_table_ptr->orig_request_table = prg_bin_hash_table_create (4, sizeof (int));
	
	if (addr_family == InetC_Addr_Family_v4)
		req_table_ptr->forward_request_table = inet_addr_hash_table_create (16, 0);
	else
		req_table_ptr->forward_request_table = inet_addr_hash_table_create (0, 16);
	
	/* Set the remaining parameters.				*/
	req_table_ptr->forward_request_expiry_time = expiry_time;
	req_table_ptr->max_rreq_retries            = max_retries;
	
	FRET (req_table_ptr);
	}


// MHAVH 11/13/08 - insert an rreq entry with the broadcast level included
void
aodv_request_table_orig_rreq_insert_geo (AodvT_Request_Table* req_table_ptr, int req_id, InetT_Address dest_address, 
										int ttl_value, double request_expiry_time, int rreq_retry,
												int request_level)
	{
	AodvT_Orig_Request_Entry*	req_entry_ptr;
	void*						old_contents_ptr;
	int*						req_id_ptr;
	
	/** Inserts a new request ID into the originating request table	**/
	FIN (aodv_request_table_orig_rreq_insert (<args>));
	
	/* Create an entry for this new request	*/
	req_entry_ptr = aodv_request_table_orig_entry_mem_alloc ();
	req_entry_ptr->target_address = inet_address_copy (dest_address);
	req_entry_ptr->request_id = req_id;
	req_entry_ptr->current_ttl_value = ttl_value;
	req_entry_ptr->insert_time = op_sim_time ();
	req_entry_ptr->current_request_expiry_time = request_expiry_time;
	req_entry_ptr->num_retries = rreq_retry;
	
	// MHAVH 13/11/08 - our request level for sending a rreq
	req_entry_ptr->request_level = request_level;
	// END MHAVH
	
	/* Allocate memory for the request ID	*/
	req_id_ptr = (int*) op_prg_mem_alloc (sizeof (int));
	*req_id_ptr = req_id;
	
	req_entry_ptr->rreq_expiry_evhandle = 
		op_intrpt_schedule_call (req_entry_ptr->insert_time + req_entry_ptr->current_request_expiry_time, 
								AODVC_ROUTE_REQUEST_EXPIRY, aodv_rte_rreq_timer_expiry_handle, req_id_ptr);
			
	/* Insert this new request into the request table	*/
	prg_bin_hash_table_item_insert (req_table_ptr->orig_request_table, (void *) &req_id, req_entry_ptr, &old_contents_ptr);
	
	FOUT;
	}

/**
void
aodv_request_table_orig_rreq_insert (AodvT_Request_Table* req_table_ptr, int req_id, InetT_Address dest_address, 
										int ttl_value, double request_expiry_time, int rreq_retry)
	{
	AodvT_Orig_Request_Entry*	req_entry_ptr;
	void*						old_contents_ptr;
	int*						req_id_ptr;
	**/
	/** Inserts a new request ID into the originating request table	**/
//	FIN (aodv_request_table_orig_rreq_insert (<args>));
	
	/* Create an entry for this new request	*/
/**	req_entry_ptr = aodv_request_table_orig_entry_mem_alloc ();
	req_entry_ptr->target_address = inet_address_copy (dest_address);
	req_entry_ptr->request_id = req_id;
	req_entry_ptr->current_ttl_value = ttl_value;
	req_entry_ptr->insert_time = op_sim_time ();
	req_entry_ptr->current_request_expiry_time = request_expiry_time;
	req_entry_ptr->num_retries = rreq_retry; **/
	
	/* Allocate memory for the request ID	*/
/**	req_id_ptr = (int*) op_prg_mem_alloc (sizeof (int));
	*req_id_ptr = req_id;
	
	req_entry_ptr->rreq_expiry_evhandle = 
		op_intrpt_schedule_call (req_entry_ptr->insert_time + req_entry_ptr->current_request_expiry_time, 
								AODVC_ROUTE_REQUEST_EXPIRY, aodv_rte_rreq_timer_expiry_handle, req_id_ptr);
**/			
	/* Insert this new request into the request table	*/
/**	prg_bin_hash_table_item_insert (req_table_ptr->orig_request_table, (void *) &req_id, req_entry_ptr, &old_contents_ptr);
	
	FOUT;
	}
**/

void
aodv_request_table_forward_rreq_insert (AodvT_Request_Table* req_table_ptr, int req_id, InetT_Address originator_addr)
	{
	List*							req_entry_lptr;
	AodvT_Forward_Request_Entry*	req_entry_ptr;
	InetT_Address*					orig_addr_ptr;
	void*							old_contents_ptr;
	
	/** Inserts a new route request into the request table	**/
	/** for nodes forwarding the route request packet		**/
	FIN (aodv_request_table_forward_rreq_insert (<args>));
		
	/* Check if there exists an entry for this address	*/
	req_entry_lptr = (List *) inet_addr_hash_table_item_get (req_table_ptr->forward_request_table, &originator_addr);
	
	if (req_entry_lptr == OPC_NIL)
		{
		/* No entry exists for this destination	*/
		/* Create a list and insert 			*/
		req_entry_lptr = op_prg_list_create ();
		inet_addr_hash_table_item_insert (req_table_ptr->forward_request_table, &originator_addr, req_entry_lptr, &old_contents_ptr);
		}
	
	/* Create an entry for this new request	*/
	req_entry_ptr = aodv_request_table_forward_entry_mem_alloc ();
	req_entry_ptr->request_id = req_id;
	req_entry_ptr->insert_time = op_sim_time ();
	
	/* Insert this new request into the request table	*/
	op_prg_list_insert (req_entry_lptr, req_entry_ptr, OPC_LISTPOS_TAIL);
	
	orig_addr_ptr = inet_address_copy_dynamic (&originator_addr);
	op_intrpt_schedule_call (
		op_sim_time () + req_table_ptr->forward_request_expiry_time,  // Time when entry may expire
		req_id,																			   // Interrupt ID
		aodv_rte_forward_request_delete,                              // Procedure to remove entry
		orig_addr_ptr);                                               // data used to locate an entry (list) in the forward RREQ table 
	
	FOUT;
	}

Boolean
aodv_route_request_forward_entry_exists (AodvT_Request_Table* req_table_ptr, int req_id,
	InetT_Address originator_addr)
    {
    AodvT_Forward_Request_Entry*    req_entry_ptr;
    List*                           req_entry_lptr;
    int                             size, count;
   
    /** Checks if a specific request ID exists  **/
    /** in the route request table              **/
    FIN (aodv_route_request_forward_entry_exists (<args>));
   
    /* Check if there exists an entry for this address  */
    req_entry_lptr = (List*) inet_addr_hash_table_item_get (req_table_ptr->forward_request_table, &originator_addr);

    /* Error Check  */
    if ((req_entry_lptr == OPC_NIL))
        FRET (OPC_FALSE);

    size = op_prg_list_size (req_entry_lptr);

    for (count = 0; count < size; count++)
        {
        req_entry_ptr = (AodvT_Forward_Request_Entry*) op_prg_list_access (req_entry_lptr, count);

        if (req_entry_ptr->request_id == req_id)
            FRET (OPC_TRUE);
        }
			
	if (prg_list_size (req_entry_lptr) == 0)
		{
		inet_addr_hash_table_item_remove (req_table_ptr->forward_request_table, &originator_addr);
		
		prg_mem_free (req_entry_lptr);
		}
		

    FRET (OPC_FALSE);
    }


Boolean
aodv_request_table_orig_rreq_exists (AodvT_Request_Table* req_table_ptr, InetT_Address dest_address)
	{
	AodvT_Orig_Request_Entry*	req_entry_ptr;
	List*						requests_lptr;
	int							num_requests, count;
	Boolean						retval = OPC_FALSE;
	
	/** Checks if an entry exists in the originating	**/
	/** table with the specified target address			**/
	FIN (aodv_request_table_orig_rreq_exists (<args>));
	
	/* Get the keys	*/
	requests_lptr = (List *) prg_bin_hash_table_item_list_get (req_table_ptr->orig_request_table);
	num_requests = op_prg_list_size (requests_lptr);
	
	if (requests_lptr == OPC_NIL)
		FRET (OPC_FALSE);
	
	for (count = 0; count < num_requests; count++)
		{
		/* Get the current request.	*/
		req_entry_ptr = (AodvT_Orig_Request_Entry *) op_prg_list_access (requests_lptr, count);
		
		if (inet_address_equal (req_entry_ptr->target_address, dest_address))
			{
			/* An entry exists for this target address	**/
			retval = OPC_TRUE;
			break;
			}
		}
	
	/* Destroy the list.	*/
	prg_list_destroy (requests_lptr, OPC_FALSE);
	
	FRET (retval);
	}


AodvT_Orig_Request_Entry*
aodv_route_request_orig_entry_get (AodvT_Request_Table* req_table_ptr, int req_id, Boolean remove)
	{
	AodvT_Orig_Request_Entry*	req_entry_ptr = OPC_NIL;
	
	/** Checks if a specific request ID exists	**/
	/** in the route request table				**/
	FIN (aodv_route_request_orig_entry_get (<args>));
	
	if (remove)
		req_entry_ptr = (AodvT_Orig_Request_Entry *) prg_bin_hash_table_item_remove (req_table_ptr->orig_request_table, (void *) &req_id);
	else
		req_entry_ptr = (AodvT_Orig_Request_Entry *) prg_bin_hash_table_item_get (req_table_ptr->orig_request_table, (void *) &req_id);
	
	FRET (req_entry_ptr);
	}


void
aodv_route_request_orig_entry_delete (AodvT_Request_Table* req_table_ptr, InetT_Address dest_addr)
	{
	AodvT_Orig_Request_Entry*	req_entry_ptr;
	int							num_requests, count;
	List*						requests_lptr;
		
	/** Deletes all route requests that have the target address	**/
	FIN (aodv_route_request_orig_entry_delete (<args>));
	
	/* Get the keys	*/
	requests_lptr = (List *) prg_bin_hash_table_item_list_get (req_table_ptr->orig_request_table);	
	num_requests = op_prg_list_size (requests_lptr);
	
	for (count = 0; count < num_requests; count++)
		{
		/* Check if there exists an entry for this address	*/
		req_entry_ptr = (AodvT_Orig_Request_Entry *) op_prg_list_access (requests_lptr, count);
	
		if (inet_address_equal (req_entry_ptr->target_address, dest_addr))
			{
			req_entry_ptr = (AodvT_Orig_Request_Entry *) prg_bin_hash_table_item_remove (req_table_ptr->orig_request_table, 
								(void *) &(req_entry_ptr->request_id));
		
			/* Cancel the previously scheduled event	*/
			op_ev_cancel (req_entry_ptr->rreq_expiry_evhandle);
	
			/* Free the request entry	*/
			aodv_request_table_orig_entry_mem_free (req_entry_ptr);
			}
		}
	
	/* Destroy the list.	*/
	prg_list_destroy (requests_lptr, OPC_FALSE);
	
	FOUT;
	}


void
aodv_request_table_orig_entry_mem_free (AodvT_Orig_Request_Entry* request_entry_ptr)
	{
	/** Frees the memory associated with the	**/
	/** request table entry						**/
	FIN (aodv_request_table_orig_entry_mem_free (<args>));
	
	/* Destroy the IP address	*/
	inet_address_destroy (request_entry_ptr->target_address);
	
	/* Free the memory	*/
	op_prg_mem_free (request_entry_ptr);
	
	FOUT;
	}


/*** Internally callable functions	***/

static AodvT_Orig_Request_Entry*
aodv_request_table_orig_entry_mem_alloc (void)
	{
	static Pmohandle			orig_request_entry_pmh;
	AodvT_Orig_Request_Entry*	request_entry_ptr = OPC_NIL;
	static Boolean				orig_request_entry_pmh_defined = OPC_FALSE;
	
	/** Allocates pooled memory for every request	**/
	/** entry in the route request table			**/
	FIN (aodv_request_table_orig_entry_mem_alloc (void));
	
	if (orig_request_entry_pmh_defined == OPC_FALSE)
		{
		/* Define the pool memory handle for route table entry	*/
		orig_request_entry_pmh = op_prg_pmo_define ("Originating Route Request Entry", sizeof (AodvT_Orig_Request_Entry), 32);
		orig_request_entry_pmh_defined = OPC_TRUE;
		}
	
	/* Allocate the route table entry from the pooled memory	*/
	request_entry_ptr = (AodvT_Orig_Request_Entry*) op_prg_pmo_alloc (orig_request_entry_pmh);
	
	FRET (request_entry_ptr);
	}


static AodvT_Forward_Request_Entry*
aodv_request_table_forward_entry_mem_alloc (void)
	{
	static Pmohandle				fwd_request_entry_pmh;
	AodvT_Forward_Request_Entry*	request_entry_ptr = OPC_NIL;
	static Boolean					fwd_request_entry_pmh_defined = OPC_FALSE;
	
	/** Allocates pooled memory for every request	**/
	/** entry in the route request table			**/
	FIN (aodv_request_table_forward_entry_mem_alloc (void));
	
	if (fwd_request_entry_pmh_defined == OPC_FALSE)
		{
		/* Define the pool memory handle for route table entry	*/
		fwd_request_entry_pmh = op_prg_pmo_define ("Forwarding Route Request Entry", sizeof (AodvT_Forward_Request_Entry), 32);
		fwd_request_entry_pmh_defined = OPC_TRUE;
		}
	
	/* Allocate the route table entry from the pooled memory	*/
	request_entry_ptr = (AodvT_Forward_Request_Entry*) op_prg_pmo_alloc (fwd_request_entry_pmh);
	
	FRET (request_entry_ptr);
	}
