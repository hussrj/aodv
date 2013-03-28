#include <opnet.h>
#include <aodv.h>
#include <aodv_ptypes.h>
#include <math.h>
//MKA 11/21/10
#include <aodv_geo_support.h>
#include <ip_rte_support.h>
/* 2012/02/16 RC */
#include <aodv_geo_table.h>

const double PI = 3.141592653589793238462643383279502884197169399375;
const double MAX_ANGLE = 360;

// MKA 12/02/10
// This is here for use in get_node_ip. The other option was to
// change all the method signatures and pass it around, so I figured this
// would be easier.
static InetT_Addr_Family aodv_addressing_mode;


/* 2012/03/01 - RC - init anything for statistics collecitons, including the file to write to */
static void aodv_geo_record_location_information_statistics_init(void);
static FILE* STATISTICS_FILE;

/* RC 2012/02/09 - Function for recording statistics for 
 * the amount of error in the locations stored in the geo table.
 * This will enable us to see how accurate the information is in the 
 * geo table.
 */
static void aodv_geo_record_location_information_statistics(AodvT_Geo_Table* geo_table_ptr, char* address, double x, double y);

			   
// Purpose:	 Compute the length of the vector
// IN:	    	 start_x, start_y -- starting point of the vector
//			 	end_x, end_y	-- ending point of the vector
// OUT:		 length of the vector
double	aodv_geo_vector_length(double start_x, double start_y,	double end_x,   double end_y)
{
	double x, y;

	FIN (aodv_geo_vector_length( <args> ));
	
	x = end_x - start_x;
	y = end_y - start_y;
	
	FRET (sqrt(pow(x,2.0) + pow(y, 2.0)));
}

// MHAVH 11/11/08 -  THIS function is NOT needed yet!!!
// Purpose: 	determine if the current node is within the search area
//				
// Algorithm:	compute the angle between if the angle is smaller than the request level,
// 				we can forward the packet, otherwise drop it
//
// IN:			angle			-- angle in degrees formed by the 3 nodes (source, current, detination or previous, current, destination)
//				request_level	-- request angle in multiples of 90
Boolean	aodv_rte_rreq_within_area(double computed_angle, int request_level)
{	
    FIN (aodv_rte_rreq_within_area( <args> ));

	// Flooding angle starts from 0 and goes to 3
	// flooding angle = (request_level +1) *90

	// the flooding angle is being computed as the request level multiplied 
	// by 45 and not 90  degrees because the computed angle could be located
	// on either side of the line that equally divides floodingn angle
	if(computed_angle <= (request_level+1) * 45.0)
	{
	    FRET(OPC_TRUE)
	}

	FRET(OPC_FALSE);

}

// MHAVH 11/11/08 
// Purpose:   	Compute the angle SME formed by three points: start (S), middle (M), end (E)
//
// In:			start_x, start_y -- position of starting point S
//				mid_x, mid_y	 -- position of the middle point M
//				end_x, end_y	 -- position of ending point E	
//
// Out:			a value of the angle formed by the points S, M, E in units of degrees 
double aodv_geo_compute_angle(double start_x, double start_y, 
							  double mid_x, double mid_y, 
							  double end_x, double end_y)
{
	double vector_MS_x;
	double vector_MS_y;
	
	double vector_ME_x;
	double vector_ME_y;
			
	double angle_form_numer;
	double angle_form_denom;
	
	
	double angle;
	
		
	FIN (aodv_geo_compute_angle( <args> ));
	
	vector_MS_x = mid_x - start_x;
	vector_MS_y = mid_y - start_y;
	
	vector_ME_x = mid_x - end_x;
	vector_ME_y = mid_y - end_y;
	
		
	angle_form_numer = (vector_MS_x * vector_ME_x) + (vector_MS_y * vector_ME_y);
	angle_form_denom = aodv_geo_vector_length(mid_x, mid_y, start_x, start_y) *  
		               aodv_geo_vector_length(mid_x, mid_y, end_x, end_y);	
	
	angle = acos(angle_form_numer/angle_form_denom) * (180/PI);
	
	
// Debugging messages
//	printf("angle_form_numer -> %.f, angle_form_denom -> %.f\n", angle_form_numer,	angle_form_denom);
//	printf("acos(angle_form_numer/angle_form_denom) * (180/pi) -> %.f\n", angle);
	
	
    FRET(angle);	

}



// Purpose:		find neighbors in current flooding angle
// IN:			neighbor_list	-- list of neighbors
//				request_level	-- request angle in multiples of 90
//				geo_entry_ptr	-- not sure if needed!!
// OUT:			TRUE: neighbor is found within flooding angle
//				FALSE: no neighbors found
Boolean aodv_geo_find_neighbor(	AodvT_Geo_Table* geo_table_ptr,
								PrgT_List* 	     neighbor_list,	// list of neighbors
								int 		     request_level,	// request level/flooding angle
								double	src_x,   double src_y,
								double dest_x,   double dest_y)
{
	double 				angle;
	AodvT_Geo_Entry* 	geo_neighbor;
	AodvT_Conn_Info* 	neighbor;
	int					i, size;

	FIN(aodv_geo_find_neighbor(<args>));
	
	size = prg_list_size(neighbor_list);

	// go through the list of neighbors
	for(i = 0; i < size; i++)
	{
		// get  neighbor
		neighbor = prg_list_access(neighbor_list, i);
		
		// get neighbour GeoTable entry
		geo_neighbor = aodv_geo_table_entry_get(geo_table_ptr, neighbor->neighbor_address, OPC_FALSE);
		
		// If neighbour exists then check if it is in the area defined  by the flooding anglee
		if(geo_neighbor != OPC_NIL)
		{
			// compute the angle formed by destination, previous node, and neighbor node
			angle = aodv_geo_compute_angle(	dest_x, dest_y, src_x, src_y,
											geo_neighbor->dst_x, geo_neighbor->dst_y);
			
			// Check if is within the area defined by the flooding angle/request level
			if(aodv_rte_rreq_within_area(angle, request_level));
			{
				FRET(OPC_TRUE);
			}
		}
	}

	FRET(OPC_FALSE);

}

// Purpose:	 Determine if the length of the vector formed by start-end  points(vector SE) is
//			 	 greater than the length of the vector formed by middle-end points(vector ME)
//
// IN:			 start_x, start_y	-- position of node where the RREQ was generated (e.g. previous node, not an originator)
//				 mid_x, mid_y		-- position of node that receives the RREQ
//				 end_x, end_y		-- position of the destination node
//				 lar_scale_factor	-- scale factor for start-end vector length (alpha)
//				 lar_padding		-- padding for scaled start-end vector length (beta)
//
// OUT:		 True,  if length(SE) >= length (ME)
//			 False, otherwise
Boolean
aodv_geo_LAR_distance(double start_x, double start_y,
					  double mid_x,   double mid_y, 
					  double end_x,   double end_y,
					  double lar_scale_factor, double lar_padding)
{
		
	FIN (aodv_rte_rreq_within_distance( <args> ));
	
	// RH 3/17/13 - Use LAR parameters in comparison (alpha and beta)
	if (lar_scale_factor * aodv_geo_vector_length(start_x, start_y, end_x, end_y) + lar_padding >=
		aodv_geo_vector_length(mid_x, mid_y, end_x, end_y))
	{
		FRET(OPC_TRUE);
	}
	
	FRET(OPC_FALSE);
}





// Purpose:	Given positions of the nodes, flooding angle, and aodv type
//				determine if the current node should rebroadcast RREQ or not
// IN:			orig_x, orig_y 		-- position of the node that originated the RREQ
//				prev_x, prev_y 		-- position of the node where the RREQ was received from
//				curr_x, curr_y 		-- position of the node that received the RREQ
//				dest_x, dest_y 		-- position of the destination node
//				flooding_angle 		-- acceptable angle to forward the RREQ
//				aodv_type	   		-- type of aodv being used
//				lar_scale_factor	-- scale factor for start-end vector length for LAR distance
//				lar_padding			-- padding for scaled start-end vector length for LAR distance
// Out:		TRUE if the current node should rebroadcase the RREQ
//				FALSE if the RREQ should be destroyed
Boolean aodv_geo_rebroadcast(
						double orig_x, double orig_y,		// Coordinates of the node that originated RREQ
						double prev_x, double prev_y,		// Coordinates of the node that send RREQ
						double curr_x, double curr_y, 		// Coordinates of the node that received RREQ
						double dest_x, double dest_y,		// Coordinates of the destination node
						double flooding_angle,				// Angle in degrees of the flooding angle
						double angle_padding,				// The maximum value by which the flooding angle can expand (for Geo_Expand only)
						int	   aodv_type, 					// Type of AODV being used
						double dest_velocity,				// The calculated velocity of the destination node (LAR)
						double lar_scale_factor,			// alpha parameter used in LAR distance
						double lar_padding)					// beta parameter used in LAR distance
{
	double angle;
	
	FIN (aodv_geo_rebroadcast( <args> ));
	
	//MKA 12/26/10
	//Since dest_x and dest_y are -1, -1 by default when destination coordinates are unknown and -1, -1 is a valid
	//coordinate, this check is necessary to make sure we aren't just performing the initial flooding like regular AODV.
	
	if (flooding_angle >= MAX_ANGLE)
	{
		//We're flooding, so you have to rebroadcast.
	
		//This takes care of regular AODV too since flooding_angle will always be 360 for regular AODV
		//when it is computed in aodv_geo_compute_expand_flooding_angle.
		FRET (OPC_TRUE);
	}
	
	//If we're not in broadcast mode, we can do what each type of AODV would normally do.
	
	switch(aodv_type)
	{
		case (AODV_TYPE_LAR_DISTANCE):
			// if current node is at least as close as the previous node from destination
			// then rebroadcast RREQ (return true), else drop (return false)
			// NOTE: in LAR distance the nodes compare the distances with the previous node.
			// LAR TODO
			FRET(aodv_geo_LAR_distance(prev_x, prev_y, curr_x, curr_y, 	dest_x, dest_y, lar_scale_factor, lar_padding));
				
	
		case AODV_TYPE_GEO_STATIC:
		case AODV_TYPE_GEO_EXPAND:
			// GeoAODV implementation:
			// Compute the angle formed by the destination, source and current nodes
			// if computed angle is not larger than flooding angle (e.g. the value is carried via request level)
			// then forward RREQ, else drop RREQ
			
			// NOTE: GEO_Expand operated the same way plus one additional condition which check if
			// there are any neighboring nodes within the search area defined by the flooding angle -- this is implemented
			// in aodv_rte process model
			
			// Check if this is not a broadcast
			if(flooding_angle < 360)
			{
				
				// Compute the angle formed by the destination node, originating node, and current node
				// Since the angle may be located on either side of the vector formed by the source-destination
				// nodes we need to multiple the computed value of angle by 2 before comparing it to the value of the
				// flooding angle, e.g. flooding angle is evenly devided by the line formed via source-destination nodes
				angle = 2*aodv_geo_compute_angle(dest_x, dest_y, orig_x, orig_y, curr_x, curr_y);
				
				
				// Use functions within the area if possible
				// Since the angle formed 
				if (angle > flooding_angle)
				{
					//MKA 12/28/10
					// If we're using GEO_EXPAND, then we have some wiggle-room.
					if (aodv_type == AODV_TYPE_GEO_EXPAND && angle - flooding_angle <= angle_padding)
					{
						printf("GEO_EXPAND: Angle (%.2f) is within the padding of %.2f degrees of the flooding angle %.2f degrees, so it will rebroadcast the RREQ.\n", 
													angle, angle_padding, flooding_angle);
						FRET(OPC_TRUE);
					}
					
					FRET(OPC_FALSE);
				}
				
				
			}	
			
			// THis is NOT a brodcast or 
			//    the angle formed by orig, curr, and dest node is less than floodign angle
			//	
			FRET(OPC_TRUE);
		
		case AODV_TYPE_GEO_ROTATE:
			// GeoAODV Rotate implementation:
			
			// Set flooding angle to intial value degrees, forward to all neighbours in the search 
			// area formed by the  flooding angle
			// if fails to find the route then increment flooding angle until it reaches 360 degrees 
			// and morphs into regular AODV
			
			//
			// NOTE: angle at the intermediate node is computed based on the previous node location
			
			// Check if this is not a broadcast
			if(flooding_angle < MAX_ANGLE)
			{
				// Compute the angle formed by the destination node, originating node, and previous node
				// Since the angle may be located on either side of the vector formed by the previous-destination
				// nodes we need to multiple the computed value of angle by 2 before comparing it to the value of the
				// flooding angle, e.g. flooding angle is evenly devided by the line formed via prev-destination nodes
				angle = 2*aodv_geo_compute_angle(dest_x, dest_y, prev_x, prev_y, curr_x, curr_y);
				
				if (angle > flooding_angle)
				{
					FRET(OPC_FALSE);
				}
				
				
			}
			
			// THis is NOT a brodcast or 
			//    the angle formed by orig, curr, and dest node is less than floodign angle
			//	
			FRET(OPC_TRUE);

	
		case AODV_TYPE_GEO_ROTATE_01:
			// GeoAODV Rotate_01 implementation:
			
			// Set angle to 180 degrees, forward to all neighbours in the search area formed by the 
			// flooding angle of 180 degrees, 
			// if fails to find the route then resort to regular AODV
			// NOTE: angle at the intermediate node is computed based on the previous node location
			
			// Check if this is not a broadcast
			if(flooding_angle < MAX_ANGLE)
			{
				// Compute the angle formed by the destination node, originating node, and previous node
				// Since the angle may be located on either side of the vector formed by the previous-destination
				// nodes we need to multiple the computed value of angle by 2 before comparing it to the value of the
				// flooding angle, e.g. flooding angle is evenly devided by the line formed via prev-destination nodes
				angle = 2*aodv_geo_compute_angle(dest_x, dest_y, prev_x, prev_y, curr_x, curr_y);
				
				if (angle > MAX_ANGLE/2)
				{
					FRET(OPC_FALSE);
				}
				
				
			}
			
			// THis is NOT a broadcast or 
			//    the angle formed by orig, curr, and dest node is less than floodign angle
			//	
			FRET(OPC_TRUE);
		
		case AODV_TYPE_LAR_ZONE:
			// LAR TODO!
			FRET(aodv_geo_LAR_within_request_zone(orig_x, orig_y, curr_x, curr_y, dest_x, dest_y, dest_velocity));
		case AODV_TYPE_REGULAR:		
			FRET(OPC_TRUE);
		}
	
	FRET (OPC_TRUE);
}


// MKA 01/08/11
//
// Purpose:	Retrieve the coordinates of the node with the given IP address.
// IN: 		AodvT_Geo_Table* geo_table_ptr	-	a reference to the geo table (needed for AODV_GEO
//			int aodv_type					-	the type of AODV we are using.
//			InetT_Address dest_addr			-	the address of the node whose coordinates we want
//			
//			double* dst_x, double* dst_y	-	destination coordinates will be stored here.
//
void aodv_geo_retrieve_coordinates(AodvT_Geo_Table* geo_table_ptr, 
									int aodv_type, Boolean location_data_distributed,
									InetT_Address dest_addr, 
									double* dst_x, double* dst_y)
{

	AodvT_Geo_Entry* 	geo_entry_ptr;
	char				ip_str[INETC_ADDR_STR_LEN];
	LAR_Data*			lar_data;
	
	FIN ( aodv_geo_retrieve_coordinates( <args> ) );
	
	// default destination coordinates and flood angle is brodacast angle.
	*dst_x = DEFAULT_X;
	*dst_y = DEFAULT_Y;
	
	
	if (aodv_type == AODV_TYPE_REGULAR)
	{
		//Does not maintain coordinates!
		FOUT;
	}
	
	/*switch (aodv_type)
	{
		case AODV_TYPE_GEO_STATIC:
		case AODV_TYPE_GEO_EXPAND:
		case AODV_TYPE_GEO_ROTATE:
		case AODV_TYPE_GEO_ROTATE_01:
	*/
	if (location_data_distributed)	//MKA_VH 7/18/11 - if we're using a distributed database, check the geo table.
	{
        
        	if (aodv_geo_table_entry_exists(geo_table_ptr, dest_addr) == OPC_TRUE)
			{
				// We have previously known coordinates for the destination. 
				// Retrieve the coordinates from the geo table.
        
				// Get Destination's info
				geo_entry_ptr = aodv_geo_table_entry_get(geo_table_ptr, dest_addr, OPC_FALSE);
        
				if (geo_entry_ptr != OPC_NIL)
				{
                	// Set destination coordinates
                	*dst_x = geo_entry_ptr->dst_x;
					*dst_y = geo_entry_ptr->dst_y;
				}
				else
				{
					printf("aodv_geo_retrieve_coordinates:\t ERROR! geoTable doesn't exist! Can't set destination coordinates!\n");
				}
			}
		//	break;
	}
	else	//MKA 7/18/11 - Otherwise, use the centralized database. 
	{
			
		//case AODV_TYPE_LAR_DISTANCE:
		//case AODV_TYPE_LAR_ZONE:
			
			//Get ip as a string
			inet_address_print(ip_str, dest_addr);
			
			//Retrieve the global entry we have for this node.
			lar_data = aodv_geo_LAR_retrieve_data(ip_str);
			if (lar_data != OPC_NIL)
			{
				// Set destination coordinates.
				*dst_x = lar_data->x;
				*dst_y = lar_data->y;
			}
			else
			{
				//This should never happen unless the interrupt updates haven't been set correctly.
				printf("aodv_geo_retrieve_coordinates:\t ERROR! LAR_Data doesn't exist for %s! Can't set destination coordinates!\n", ip_str);
			}
			//break;
			
		/*case AODV_TYPE_REGULAR:
		default:
			// does NOT maintain coordinates
			break; */
		
	}
	
	FOUT;
}



// Purpose:	Decide to change the flooding angle based on geo table information of
//			neighboring nodes and the current request level (which is > INITIAL_REQUEST_LEVEL
//			if the first attempt has failed).
//
//			If we're using a distributed system for storing location data, then this function
//			will return BROADCAST_REQUEST_LEVEL, which means act like AODV and flood the network.
//			
// IN:			orig_x, orig_y -- position of the node that originated the RREQ
//				prev_x, prev_y -- position of the node where the RREQ was received from
//				curr_x, curr_y -- position of the node that received the RREQ
//				dest_x, dest_y -- position of the destination node
//				flooding_angle -- acceptable angle to forward the RREQ
//				aodv_type	   -- type of aodv being used
// Out:			return RREQ
int aodv_geo_compute_expand_flooding_angle( 
			InetT_Address_Hash_Table_Handle 	neighbor_connectivity_table, 	
			InetT_Address 						dest_addr, 
			double								src_x,
			double								src_y,
			int 								request_level, 
			AodvT_Geo_Table* 					geo_table_ptr,		
			int	   								aodv_type,
			Boolean								location_data_distributed, 
			double								dst_x, //&dst_x
			double	 							dst_y)	//&dst_y		
{
	//PrgT_List* 			neighbor_list;
	//RH 3/27/13 - Need global stat handle for fallback statistic
	AodvT_Global_Stathandles*	global_stat_handle_ptr;
	
	FIN (aodv_geo_rreqsend( <args> ));
	
	global_stat_handle_ptr = aodv_support_global_stat_handles_obtain();
	//MKA_VH 7/18/11 - If we're using a distributed geo table and we don't have destination coordinates,
	// use regular AODV (broadcast).
	if (aodv_type == AODV_TYPE_REGULAR ||
		(location_data_distributed && aodv_geo_table_entry_exists(geo_table_ptr, dest_addr) == OPC_FALSE))
	{
		//Since we don't have accurate destination coordinates, just broadcast.
		FRET (BROADCAST_REQUEST_LEVEL);
	}
		
	switch(aodv_type)
	{
		
		case AODV_TYPE_LAR_DISTANCE:
		case AODV_TYPE_LAR_ZONE:
			// LAR TODO
			// MKA 12/12/10
		
			// Initial LAR request failed; revert to regular AODV
			if (request_level != INITIAL_REQUEST_LEVEL)
			{
				// RH 3/27/13 - Update global aodv fallbacks statistic
				op_stat_write(global_stat_handle_ptr->num_aodv_fallbacks_global_shandle, 1.0);
				
				request_level = BROADCAST_REQUEST_LEVEL;
			}
			
			break;

		case AODV_TYPE_GEO_STATIC:
		case AODV_TYPE_GEO_EXPAND:
		case AODV_TYPE_GEO_ROTATE:		
		case AODV_TYPE_GEO_ROTATE_01:
		//MKA_VH 7/18/11 - We don't have to do anything; request_level is already incremented outside of
		// this function upon a route discovery failure, which translates into an increased angle for GeoAODV.
			if (request_level == BROADCAST_REQUEST_LEVEL) {
				// RH 3/27/13 - Update global aodv fallback statistic
				op_stat_write(global_stat_handle_ptr->num_aodv_fallbacks_global_shandle, 1.0);
			}
			break;
				
		case AODV_TYPE_REGULAR:
		default:
			// do not use or expland the flooding angle based on neightbor locations
			request_level = BROADCAST_REQUEST_LEVEL;
			break;
		
	}
	
	// Return the request level (aka the "flooding angle")
	FRET(request_level);

}

// MKA 11/25/10
// Retrieve the current node's IP and store it in ip_str.
void get_node_ip(char ip_str[], IpT_Rte_Module_Data* module_data, int output_intf_index)
{
	//Store this node's IP into ip_str.

	IpT_Interface_Info*	ifinfo;
	InetT_Address		inet_addr;
	
	FIN (get_node_ip (<args>));
	
	//IpT_Rte_Module_Data* module_data_ptr = (IpT_Rte_Module_Data*) op_pro_modmem_access ();
	
	//MKAVH	11/25/10
	ifinfo = inet_rte_intf_tbl_access(module_data, output_intf_index);
	inet_addr = inet_rte_intf_addr_get(ifinfo, aodv_addressing_mode);
	inet_address_print(ip_str, inet_addr);

#if defined(LAR_UPDATE_DEBUG) && defined(LAR_DEBUG) 
	printf("Retrieved IP: %s\n", ip_str);
#endif
	
	FOUT;
	
}


// MKA 11/23/10
// Print the given LAR_Data to stdout.
void print_lar_data(LAR_Data *lar_data)
{
	printf("LAR DATA\n");
	printf("--------\n");
	printf("Position: \t(%f, %f)\n", lar_data->x, lar_data->y);
	printf("Velocity: \t%f\n", lar_data->velocity);
	printf("Time:     \t%f\n", lar_data->time);
}

	
// VHRCMA	11/11/10
// TODO RC 11/22/2010 We need to ensure that this only runs when 
// LAR is active, i.e it shouldn't running unless it's needed.
// Maybe we can alter our interupt condition to include this.
// TODO RC 2012/02/09 - I amend my above todo now that his 
// Method handles statistics collections to.
//
// MKA 11/23/10	Finished implementing interrupt.
// NOTE: This function assumes that an entry exists in the global
//		 database to update, and always pulls the data using IF0 as a key.

void aodv_geo_LAR_update(int proc_id, double update_interval, AodvT_Geo_Table* geo_table_ptr)
{
	
	int 		node_id;				//This node's id.
	void*		data;					//Generic data storage pointer for when LAR_Data is retrieved from the database.
	LAR_Data*	lar_data;				//The data stored in the database
	double 		x,y, velocity, time;	//The node's current x, y, v, and t
	char		address[INETC_ADDR_STR_LEN]; 	// The node's IP address.
	IpT_Rte_Module_Data* module_data_ptr;		// The module data to use.

	FIN (aodv_geo_LAR_update( <args> ));
	
#if defined(LAR_UPDATE_DEBUG) && defined(LAR_DEBUG) 
	printf("=========================LAR UPDATE=========================\n");
#endif
	
		
	
	/* COLLECT ATTRIBUTES FROM NODE */
	node_id = op_topo_parent(proc_id);
	op_ima_obj_attr_get (node_id, "x position", &x);
	op_ima_obj_attr_get (node_id, "y position", &y);

	// the module_data_ptr should really be retrieved through the proc_id somehow...
	module_data_ptr = (IpT_Rte_Module_Data*) op_pro_modmem_access ();
	get_node_ip(address, module_data_ptr, 0);
	
	// -1. Create a struct to save x,y, velocity, and time
	// decalre data struct in aodv_geo_support.h
	// 1. Get IP address
	// 2. Convert IP address into string (char *): inet_address_print
	// 3. Compute Velocity
	// 4. Store: x,y, velocity, current time into global table using IP address as a key
	// oms_data_def_entry_insert()
	// oms_data_def_entry_access()
	
	
	/* RETRIEVE DATA FROM DATABASE */
	
	data = oms_data_def_entry_access(LAR_OMS_CATEGORY, address);
	if (data == OPC_NIL)
	{
		// We haven't stored data for this node!
		printf("No initial LAR_Data found in the global database!");
		FOUT;
	}
	else
	{
		//Previous data exists; alter it.
	
		lar_data = (LAR_Data*) data;
	
#if defined(LAR_UPDATE_DEBUG) && defined(LAR_DEBUG) 
		print_lar_data(lar_data);
#endif
		
		time = op_sim_time();
		velocity = aodv_geo_LAR_calc_velocity(lar_data->x, lar_data->y, x, y, lar_data->time, time);
		lar_data->x = x;
		lar_data->y = y;
		lar_data->velocity = (velocity < 0 ? -velocity : velocity);	//store only the magnitude of the velocity vector.
		lar_data->time = time;
	}

#if defined(LAR_UPDATE_DEBUG) && defined(LAR_DEBUG) 
	printf("\n==Coordinates at time %f for node with IP %s are (%f, %f)==\n",time, address, x, y);
	printf("\nCommitted ");
	print_lar_data(lar_data);
#endif

	/* RC 2012/02/09 - record statistics for location information */
	/* RC 2012/02/22 - Trying to do this as it's own state transistion now */
	aodv_geo_record_location_information_statistics(geo_table_ptr, address, lar_data->x, lar_data->y);	
	
	//Schedule the next interrupt.
	op_intrpt_schedule_self (op_sim_time () + update_interval, AODVC_LAR_UPDATE);

#if defined(LAR_UPDATE_DEBUG) && defined(LAR_DEBUG) 
	printf("============================================================\n\n");
#endif
	
	FOUT;
}

// MKA 11/23/10
// Create and return a new LAR_Data data structure initialized with the given coordinates.
// and default velocity (0.0) and time (0.0).
LAR_Data* new_LAR_Data(double x, double y)
{
	LAR_Data *lar_data; 

#ifdef LAR_DEBUG
	printf("\n==Creating new LAR_Data with default values and position (%f, %f)==\n", x, y);
#endif
	
	lar_data = op_prg_mem_alloc( sizeof(LAR_Data) );
	lar_data->x = x;
	lar_data->y = y;
	lar_data->velocity = 0;
	lar_data->time = op_sim_time();
	return lar_data;
}

// MKA 12/02/10
// Purpose:	This function performs initializations for LAR, including inserting the initial
// 			LAR_Data entries for the global database.
// IN:		module_data_ptr		-	a pointer to the node's module data (used to retrieve the IP.
//									(usually retrieved like this: (IpT_Rte_Module_Data*) op_pro_modmem_access () )
//			address_mode		-	the address mode we're using (this format will be used for the IPs used as
//									keys for storing LAR_Data).
//			x, y				-	the node's initial position.
									
void aodv_geo_LAR_init( IpT_Rte_Module_Data* module_data_ptr, InetT_Addr_Family address_mode, double x, double y )
{
	
	// MKA 11/28/10
	// The following variables are used to initialize
	// data for LAR updates.
	LAR_Data			*lar_data;
	// RC 04/17/2012
	// Added to put a false entry in the central hello messages.
	// This is because nodes should insert its entry into the table
	// when it is about to send a hello messsage. 
	LAR_Data			*hello_message_database_invalid_data;
	int					num_interfaces;
	int					ifnum;
	char				address[INETC_ADDR_STR_LEN];
	
	FIN (aodv_geo_LAR_init( <args> ));
	
	aodv_addressing_mode = address_mode;
	
	// Store initial position
	// You want to store the following information:
	// 1. x, y coordinates
	// 2. time when they were recorded
	// 3. velocity
	
	// Use function oms_data_insert
	
	
	// MKA 11/28/10
	// Create the initial entry in the global databse which will be updated at each LAR interrupt.
	// RC 04/17/2012
	// After finding a bug in sending hello messages we determined that the statistics were not
	// being distributed. A global database of coordinates was used to circumvent this bug. 
	lar_data = new_LAR_Data(x, y);
	hello_message_database_invalid_data = new_LAR_Data(DEFAULT_X, DEFAULT_Y);
	num_interfaces = inet_rte_num_interfaces_get (module_data_ptr);
	for (ifnum = 0; ifnum < num_interfaces; ifnum++)
	{
		//In case there are multiple interfaces at this node,
		//create an entry for each one referencing the same data
		//so that no matter which IP the data is pulled from, the data will
		//be the same.
		get_node_ip(address, module_data_ptr, ifnum);
		oms_data_def_entry_insert(LAR_OMS_CATEGORY, address, lar_data);
		printf("????????????????: Initiliazing hello database entry for \"%s\"\n", address);
		oms_data_def_entry_insert(HELLO_OMS_CATEGORY, address, hello_message_database_invalid_data);
	}
	
	/* 2012/03/01 - RC
	 * init the statistics collection
	 */
	aodv_geo_record_location_information_statistics_init();
	
	FOUT;
}

// MKA 12/02/10
// Purpose:	This method returns whether or not the current node is within the 
// 			request zone. The request zone will be as specified by LAR Scheme 1 (with Dan Urbano's alterations
// 			to take into account a source parallel to the destination).
// IN:		src_x, src_y	-	the coordinates of the originating source node
//			curr_x, curr_y	-	the coordinates of the node to test
//			dest_x, dest_y	-	the coordinates of the destination node.
//			radius			-	the velocity of the destination, or the radius of the expected zone per LAR1.
// OUT:		OPC_TRUE if the node is within the request zone and OPC_FALSE otherwise.
Boolean aodv_geo_LAR_within_request_zone(double src_x, double src_y, double curr_x, double curr_y, double dest_x, double dest_y, double radius)
{

	//The corners of the rectangular request zone: ll = lower-left, ul = upper-left, ur = upper-right, lr = lower-right. 
	Point2D ll, ul, ur, lr;
	
	//The location of the current node.
	Point2D currentLocation;
	
	//This is the request zone rectangle made up of the four points above.
	Rectangle requestZone;
	
	//The return value for this method (whether or not the current node is contained within the request zone.
	Boolean contained;
	
	FIN (aodv_geo_LAR_within_request_zone( <args> ));
	
	currentLocation.x = curr_x;			
	currentLocation.y = curr_y;

	//The lower-left corner of the request zone is as far left and as far down as possible.
	ll.x = min(src_x, dest_x - radius); 
	ll.y = min(src_y, dest_y - radius);
	
	//The upper-left corner of the request zone must be as far left and as far up as possible.	
	ul.x = ll.x; 									
	ul.y = max(src_y, dest_y + radius);
	
	//The upper-right corner of the request zone must be as far right and as far up as possible.
	ur.x = max(src_x, dest_x + radius); 
	ur.y = ul.y;
	
	//The lower-right corner of the request zone must be as far right and as far down as possible.	
	lr.x = ur.x;									
	lr.y = ll.y;
	
	//Encapsulate these four points into a rectangle.
	requestZone.lower_left = ll;
	requestZone.upper_left = ul;
	requestZone.upper_right = ur;
	requestZone.lower_right = lr;
	
	contained = aodv_geo_LAR_is_point_contained(&currentLocation, &requestZone);
	
#ifdef LAR_DEBUG
	printf("=> aodv_geo_LAR_within_request_zone: Current node's coordinates: (%.2f, %.2f)\n", currentLocation.x, currentLocation.y);
	printf("=> aodv_geo_LAR_within_request_zone: Src: (%.2f, %.2f) Dest: (%.2f, %.2f) Dest velocity = %.2f\n", src_x, src_y, dest_x, dest_y, radius);
	printf("=> aodv_geo_LAR_within_request_zone: Calculated request zone: \n");
	printf("\t (%.2f, %.2f) \t	(%.2f, %.2f)\n", ul.x, ul.y, ur.x, ur.y);
	printf("\t (%.2f, %.2f) \t	(%.2f, %.2f)\n", ll.x, ll.y, lr.x, lr.y);
	printf("=> aodv_geo_LAR_within_request_zone: Contained = %s\n", contained == OPC_TRUE ? "True" : "False");
#endif
	
	FRET ( contained );	
}

// MKA 12/02/10
// Purpose:	Simple helper function that determines whether or not the given Point
// 			is within the bounds of the provided Rectangle.
// IN:		location	-	the point to check.
// 			zone		-	the bounds to check against.
// OUT:		OPC_TRUE if the given location is contained within the zone,
//			and OPC_FALSE otherwise.
Boolean aodv_geo_LAR_is_point_contained(Point2D *location, Rectangle *zone)
{
	FIN (aodv_geo_LAR_is_point_contained( <args> ));
	
	// assumes that all sides of the 
    // rectangle are parallel to their respective axes

    // left of rectangle
    if (location->x < zone->upper_left.x)
            FRET ( OPC_FALSE );
    
	// above the rectangle
    if (location->y > zone->upper_left.y)
            FRET ( OPC_FALSE );

    // right of the rectangle
    if (location->x > zone->upper_right.x)
            FRET ( OPC_FALSE );

    // below rectangle
    if (location->y < zone->lower_right.y)
            FRET ( OPC_FALSE );

    // Otherwise, it's in the rectangle.
	FRET (OPC_TRUE);
	
}

// MKA 12/02/10
// Purpose:	Retrieve LAR_Data from the global database using the given IP.
// IN:		char* ip	-	the IP address to use as the lookup key.
// OUT:		A pointer to the LAR_Data stored in the database (or OPC_NIL if an 
//			entry doesn't exist).
LAR_Data* aodv_geo_LAR_retrieve_data(char* ip)
{
	return (LAR_Data*) oms_data_def_entry_access(LAR_OMS_CATEGORY, ip);
}

/* RC 2012/03/01
 *
 * Purpose: Initilize everything for location statistics recording
 *
 * IN:		none
 *
 * OUT:     void
 */
static void aodv_geo_record_location_information_statistics_init(void)
{
	// TODO Does this need to be append since multiple nodes are writing to it?
	STATISTICS_FILE = fopen("location_statistics.csv", "w");
}


/* RC 2012/02/09
 *
 * Purpose: Function for recording statistics for 
 *          the amount of error in the locations stored in the geo table.
 *          This will enable us to see how accurate the information is in the 
 *          geo table.
 *
 * IN:		geo_talbe_ptr A pointer to the GeoTable we are examining.
 *			this_node_name The ip address (as a string) of the current node we are in (for recording actual coordinates).
 *			x The actual x coordinate of this node.			
 *			y The actual y coordinate of this node.
 *
 * OUT:     void
 */
static void aodv_geo_record_location_information_statistics(AodvT_Geo_Table* geo_table_ptr, char* this_node_address, double x, double y) {
	/* 2012/02/08 RC - Adding code to collect statistics for the accuracy of coordinates in the routing table
	 * The CSV file will tentatively have each row with values: time and location errer for each node (need to clarify how this will be done
	 * i.e the case when no route is stored). Therefor we need to pass it. */
	
	/* The basic idea here is to dump all of the entries in the this nodes routing table (need to edit the geo_table data
     * impl so that we can do this, need to figure out how inet_addr_hash_table_item_insert) and see how it matches
	 * to each entry in the global database. We also need to check to make sure that the global database isn't 
	 * being used. */
	
	/* Psuedo code for this idea */
	/*
	foreach entry e in geo_table:
		get the central tables entry for e
		compare e to what is in the central table
		record the statistics
	*/

	PrgT_List *geo_entries_list;
	AodvT_Geo_Entry *entry_ptr;
	Objid ppid;
	char node_name[OMSC_HNAME_MAX_LEN];
	char dest_addr[INETC_ADDR_STR_LEN];
	int entry_count, i;
	
	FIN (aodv_geo_record_location_information_statistics( <args> ));


	
	/* The first part of the statistics collection is to collect this 
	 * nodes ACTUAL coordinate. This will be the same coordinates that 
	 * were placed in the centralized coordinate Database. */
	
	// TODO Consider getting op_Sim_time() into a variable to that it is
	// garunteed not to vary for this entire write
	fprintf(STATISTICS_FILE, "CENTRAL, %f, %s, %f, %f\n", op_sim_time(), this_node_address, x, y);
	
	/* The second part of the statistics collection is to collect this
	 * nodes GeoTable. */
	
	/* Get the name of this node */
	ppid = op_topo_parent(op_id_self());
	op_ima_obj_attr_get (ppid, "name", &node_name);
	
	
	geo_entries_list = aodv_geo_table_get_all_entries(geo_table_ptr);
	entry_count = op_prg_list_size(geo_entries_list);
	
	for (i = 0; i < entry_count; i++) 
	{
		entry_ptr = (AodvT_Geo_Entry*) op_prg_list_access (geo_entries_list, i);
		if (entry_ptr != OPC_NIL) 
		{
			/* Get the destination address as a string */
			inet_address_print(dest_addr , entry_ptr->dst_address);
				
			//printf("Destination @ (%f, %f)\n", entry_ptr->dst_x, entry_ptr->dst_y);
			// TODO consider using address that is passed in instead of node name
			fprintf(STATISTICS_FILE, "TABLE, %f, %s, %s, %f, %f, %f\n", op_sim_time(), node_name, dest_addr, 
				entry_ptr->dst_x, entry_ptr->dst_y, entry_ptr->insert_time);
		}
		else
		{
			printf("entry_ptr is OPC_NIL\n");
		}

	}
	
	FOUT;
}
