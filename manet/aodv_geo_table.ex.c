// MHAVH 10/25/08
/* aodv_geo_table.ex.c */

// modeled after aodv_request_table.ex.c

#include <opnet.h>
#include <aodv.h>
#include <aodv_ptypes.h>
#include <ip_addr_v4.h>
#include <prg_bin_hash.h>
// 2012-04-14 - RC - added this for debug symbols
#include <aodv_geo_table.h>


// RC JS 07/12/2010 - Moved these declarations into the code file since they are not used externally, and should not be.
static AodvT_Geo_Entry*		aodv_geo_table_entry_mem_alloc(void);
Compcode		 aodv_geo_table_entry_delete (AodvT_Geo_Table* geo_table_ptr, InetT_Address dst_address);

/************************ begin modified from aodv_request_table *******************/
AodvT_Geo_Table*
aodv_geo_table_create (InetT_Addr_Family hash_key_addr_family
	//double expiry_time - not sure if we need it???
						)
	{
	AodvT_Geo_Table*		geo_table_ptr;
	
	/** Allocates and initializes the geo table	**/
	FIN (aodv_geo_table_create (<args>));
	
	/* Allocate memory.								*/
	geo_table_ptr = (AodvT_Geo_Table*) op_prg_mem_alloc (sizeof (AodvT_Geo_Table));
	
	/* Create the Hash tables.						*/
	// 4 ==> The base-2 log of the desired size (in cells) of the hash table. 
	// sizeof (int) ==> The maximum number of bytes from the key that should be used when computing the hash value. 
	if (hash_key_addr_family == InetC_Addr_Family_v4)
		{
		printf("Creating IPv4 GeoTable\n");
		geo_table_ptr->geo_table = inet_addr_hash_table_create (100, 0);
		}
	else
		{
		geo_table_ptr->geo_table = inet_addr_hash_table_create (0, 100);
		printf("Creating IPv6 GeoTable\n");
		}
	//geo_table_ptr->geo_table = prg_bin_hash_table_create (4, sizeof (int));
	
	
	FRET (geo_table_ptr);
	}

Boolean						
aodv_geo_table_entry_exists(AodvT_Geo_Table* geo_table_ptr, InetT_Address dst_address)
	{
	AodvT_Geo_Entry*			geo_entry_ptr = OPC_NIL;
	
    /** Checks if a specific destination address exists  **/
    /** in the geo table              **/
    FIN (aodv_geo_table_entry_exists (<args>));
    
	/* Check if there exists an entry for this address  */
    geo_entry_ptr = (AodvT_Geo_Entry *) inet_addr_hash_table_item_get(geo_table_ptr->geo_table, &dst_address);


	//prg_bin_hash_table_item_get (geo_table_ptr->geo_table, (void *) &dst_address);
	
	if (geo_entry_ptr == PRGC_NIL)
	//if (geo_entry_ptr == OPC_NIL)
        FRET (OPC_FALSE);
	
	FRET(OPC_TRUE);
	
	}

// MKA 01/02/11
// Retrieve the sequence number stored for the given entry.
int
aodv_geo_table_entry_sequence_number(AodvT_Geo_Table* geo_table_ptr, InetT_Address dst_address)
{
	AodvT_Geo_Entry*			geo_entry_ptr = OPC_NIL;
	
	FIN (aodv_geo_table_entry_sequence_number (<args>));
	
	if (aodv_geo_table_entry_exists(geo_table_ptr, dst_address) == OPC_TRUE)
	{
		geo_entry_ptr = aodv_geo_table_entry_get(geo_table_ptr, dst_address, OPC_FALSE);
		FRET(geo_entry_ptr->sequence_number);
	}
	
	FRET(-1);
}

void
aodv_geo_table_insert (AodvT_Geo_Table* geo_table_ptr, InetT_Address dst_address, 
										double dst_x, double dst_y, int sequence_number) //MKA 01/02/11
	{
	AodvT_Geo_Entry*			geo_entry_ptr;
	//only used for debugging
	#ifdef GEO_AODV_TABLE_DEBUG
	Objid						own_id, ppid;
	char						addr_str [INETC_ADDR_STR_LEN];
	char						name [256];
	#endif
	
	
	/** Inserts a new geo entry into the originating geo table	**/
	FIN (aodv_geo_table_insert (<args>));
	
	
	// Print dest address for debugging
	#ifdef GEO_AODV_TABLE_DEBUG
	inet_address_print (addr_str, dst_address);
	own_id = op_id_self();
	ppid = op_topo_parent(own_id);
	op_ima_obj_attr_get(ppid, "name", &name);
	#endif
	
	if (aodv_geo_table_entry_exists(geo_table_ptr, dst_address))
	{
		#ifdef GEO_AODV_TABLE_DEBUG
		printf("^^^^^^^^^^^^^^^^%s:Entry for %s Exists. Insert is omitted!!!\n",name, addr_str);
		#endif
		
		// May want to add code to overwrite existing value
		// ???	
	}
	else
	{
		/* Create an entry for this new request	*/
		geo_entry_ptr = aodv_geo_table_entry_mem_alloc ();
		geo_entry_ptr->dst_address = inet_address_copy (dst_address);
		geo_entry_ptr->insert_time = op_sim_time ();
		geo_entry_ptr->dst_x = dst_x;
		geo_entry_ptr->dst_y = dst_y;
		geo_entry_ptr->sequence_number = sequence_number;	//MKA 01/02/11
		
		/* Insert this new request into the request table	*/
		#ifdef GEO_AODV_TABLE_DEBUG
		printf("^^^^^^^^^^^^^^^^%s: Storing Dest %s = (%.2f, %.2f) with sequence_number %d\n", name, addr_str, dst_x, dst_y, sequence_number);
		#endif
		inet_addr_hash_table_item_insert(geo_table_ptr->geo_table, &dst_address, geo_entry_ptr, PRGC_NIL);
//		prg_bin_hash_table_item_insert (geo_table_ptr->geo_table,  &(dst_address.address.ipv4_addr), 
//										geo_entry_ptr, PRGC_NIL);
	}
	
	
	FOUT;
}


// Purpose: Overwrites an entry in the GeoTable if exist
//			if not, simply inserts the entry
// In:		geo_table_prt -- pointer to the GeoTable
//			address -- IP address of the node to be added into the GeoTable
//			x,y     -- coordinates of the node to be added into the GeoTable
// Out: 	NONE
// Uses:	aodv_geo_table_entry_exists
//			aodv_geo_table_entry_delete
//			aodv_geo_table_insert
void aodv_geo_table_update (AodvT_Geo_Table* geo_table_ptr, 
							InetT_Address address,	double x, double y, int sequence_number)
{
	#ifdef GEO_AODV_TABLE_DEBUG 
	Objid						own_id, ppid;
	char						addr_str [INETC_ADDR_STR_LEN];
	char						name [256];
	#endif	
	
	/** Inserts a new geo entry into the originating geo table	**/
	FIN (aodv_geo_table_update (<args>));

	if (x == 0 && y == 0) 
	{
		op_prg_odb_bkpt ("geo_aodv");
	}

	
	/* Need to double check that 0 is invalid for the geo_table
	if (sequence_number == 0) {
		#ifdef GEO_AODV_TABLE_DEBUG
		inet_address_print (addr_str, address);
		printf("^^^^^^^^^^^^^^^^Ignoring update (sequence number is 0) %s (%d) with (%f, %f)\n", addr_str, sequence_number, x, y);
		#endif
		FOUT;
	}
	*/
	
	#ifdef GEO_AODV_TABLE_DEBUG
	inet_address_print (addr_str, address);
	own_id = op_id_self();
	ppid = op_topo_parent(own_id);
	op_ima_obj_attr_get(ppid, "name", &name);
	printf("^^^^^^^^^^^^^^^^%s: Trying to update %s (%d) with (%.2f, %.2f)\n", name, addr_str, sequence_number, x, y);
	#endif	
	// Delete old entry if exists
	if (aodv_geo_table_entry_exists(geo_table_ptr, address))
	{
			aodv_geo_table_entry_delete(geo_table_ptr, address);
	}
	

	// add a new entry
	aodv_geo_table_insert(geo_table_ptr, address, x, y, sequence_number);
					
	
	FOUT;
}


// Purpose: remove entry from geoTable if exists
// In:		geo_table_prt -- pointer to the GeoTable
//			address -- IP address of the node to be added into the GeoTable
// Out: 	NONE
// Uses:	aodv_geo_table_entry_exists
//			aodv_geo_table_entry_delete
Compcode aodv_geo_table_entry_remove (AodvT_Geo_Table* geo_table_ptr, InetT_Address dst_address)
{
	
	/** Deletes all entries that have the target address	**/
	FIN (aodv_geo_table_entry_remove (<args>));
	

	// Delete old entry if exists
	if (aodv_geo_table_entry_exists(geo_table_ptr, dst_address))
	{
			aodv_geo_table_entry_delete(geo_table_ptr, dst_address);
			FRET (OPC_COMPCODE_SUCCESS);
	}
	FRET (OPC_COMPCODE_FAILURE);
}


AodvT_Geo_Entry*
aodv_geo_table_entry_get (AodvT_Geo_Table* geo_table_ptr, InetT_Address dst_address, Boolean remove)
{
	AodvT_Geo_Entry*	geo_entry_ptr = OPC_NIL;
	//only used for debugging
	#ifdef GEO_AODV_TABLE_DEBUG 
	Objid						own_id, ppid;
	char						addr_str [INETC_ADDR_STR_LEN];
	char						name [256];
	#endif
	
	/** Checks if a specific dst_address exists	**/
	/** in the route request table				**/
	FIN (aodv_geo_table_entry_get (<args>));
	
	#ifdef GEO_AODV_TABLE_DEBUG
	inet_address_print (addr_str, dst_address);
	own_id = op_id_self();
	ppid = op_topo_parent(own_id);
	op_ima_obj_attr_get(ppid, "name", &name);
	#endif
	
	if (remove)
		{
		#ifdef GEO_AODV_TABLE_DEBUG
		printf("^^^^^^^^^^^^^^^^%s: REMOVING Dest %s with key %d\n", name, addr_str, (int) &dst_address);
		#endif
		geo_entry_ptr = (AodvT_Geo_Entry *) inet_addr_hash_table_item_get(geo_table_ptr->geo_table, &dst_address);
		// prg_bin_hash_table_item_remove (geo_table_ptr->geo_table, (void *) &dst_address);
		}
	else
		{
		#ifdef GEO_AODV_TABLE_DEBUG
		printf("^^^^^^^^^^^^^^^^%s: GETTING Dest %s with key %d\n", name, addr_str, (int) &dst_address);
		#endif
		geo_entry_ptr = (AodvT_Geo_Entry *) inet_addr_hash_table_item_get(geo_table_ptr->geo_table, &dst_address);
		//prg_bin_hash_table_item_get (geo_table_ptr->geo_table, (void *) &dst_address);
		}

	#ifdef GEO_AODV_TABLE_DEBUG
	inet_address_print (addr_str, geo_entry_ptr->dst_address);
	printf("^^^^^^^^^^^^^^^^%s: OBTAINED Dest %s \n", name, addr_str);
	#endif
	FRET (geo_entry_ptr);
}

Compcode
aodv_geo_table_entry_delete (AodvT_Geo_Table* geo_table_ptr, InetT_Address dst_address)
	{
	AodvT_Geo_Entry*	geo_entry_ptr;
	//only used for debugging
	#ifdef GEO_AODV_TABLE_DEBUG
	Objid						own_id, ppid;
	char						addr_str [INETC_ADDR_STR_LEN];
	char						name [256];
	#endif
	
	/** Deletes all entries that have the target address	**/
	FIN (aodv_geo_table_entry_delete (<args>));
	
	// Print dest address for debugging
	#ifdef GEO_AODV_TABLE_DEBUG
	inet_address_print (addr_str, dst_address);	
	own_id = op_id_self();
	ppid = op_topo_parent(own_id);
	op_ima_obj_attr_get(ppid, "name", &name);
	printf("^^^^^^^^^^^^^^^^%s: Deleting Dest %s with key %d\n", name, addr_str, (int) &dst_address);
	#endif
	
	geo_entry_ptr = (AodvT_Geo_Entry *) inet_addr_hash_table_item_remove(geo_table_ptr->geo_table, &dst_address);
	// prg_bin_hash_table_item_remove (geo_table_ptr->geo_table, 	(void *) &dst_address);

	if (geo_entry_ptr == OPC_NIL)
		FRET (OPC_COMPCODE_FAILURE);
	
	/* Free the request entry	*/
	aodv_geo_table_entry_mem_free (geo_entry_ptr);
	
	
	FRET (OPC_COMPCODE_SUCCESS);	
}

		

void
aodv_geo_table_entry_mem_free (AodvT_Geo_Entry* geo_entry_ptr)
{
	/** Frees the memory associated with the	**/
	/** geo table entry						**/
	FIN (aodv_geo_table_entry_mem_free (<args>));
	
	/* Destroy the IP address	*/
	inet_address_destroy (geo_entry_ptr->dst_address);
	
	/* Free the memory	*/
	op_prg_mem_free (geo_entry_ptr);
	
	FOUT;
} 


/*** Internally callable function ***/

static AodvT_Geo_Entry*
aodv_geo_table_entry_mem_alloc (void)
{
	static Pmohandle		geo_entry_pmh;
	AodvT_Geo_Entry*		geo_entry_ptr = OPC_NIL;
	
	/** Allocates pooled memory for every geo   	**/
	/** entry in the geo table          			**/
	FIN (aodv_geo_table_entry_mem_alloc (void));

	/* Define the pool memory handle for geo table entry	*/
	geo_entry_pmh = op_prg_pmo_define ("Geo Table Entry", sizeof (AodvT_Geo_Entry), 32);
	
	/* Allocate the geo entry from the pooled memory	*/
	geo_entry_ptr = (AodvT_Geo_Entry*) op_prg_pmo_alloc (geo_entry_pmh);
	
	FRET (geo_entry_ptr);
}

/* RC 2012/02/16 - Added this method in order to iterate all entries in the table */
PrgT_List *aodv_geo_table_get_all_entries(AodvT_Geo_Table* geo_table_ptr) 
{
	FIN (aodv_geo_table_get_all_entries (<args>));
	//inet_addr_hash_table_item_list_get (InetT_Address_Hash_Table_Handle htable_handle, InetT_Addr_Family key_addr_family);
	FRET (inet_addr_hash_table_item_list_get(geo_table_ptr->geo_table, InetC_Addr_Family_v4));
	
	// VHRC Attempt to get this to work, it essentially bypasses inet_addr_hash_table_item_list_get
	// and does what it would do internally
	//FRET(prg_bin_hash_table_item_list_get (geo_table_ptr->geo_table->ipv4_hash_table));
}
