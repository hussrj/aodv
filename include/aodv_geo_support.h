//RCMKA 11/22/2010
#ifndef _AODV_GEO_SUPPORT
#define _AODV_GEO_SUPPORT

#include <opnet.h>
#include <aodv.h>
#include <aodv_ptypes.h>
#include <math.h>
#include <stdio.h>
#include <ip_rte_support.h>


// MKA 01/08/11
void aodv_geo_retrieve_coordinates(AodvT_Geo_Table* geo_table_ptr, 
									int aodv_type, Boolean location_data_distributed,
									InetT_Address dest_addr, 
									double* dst_x, double* dst_y);

Boolean aodv_geo_rebroadcast(
						double orig_x, double orig_y,		
						double prev_x, double prev_y,		
						double curr_x, double curr_y, 		
						double dest_x, double dest_y,		
						double flooding_angle,	
						double angle_padding,
						int	   aodv_type,
						double dest_velocity,	//MKA 12/02/10 - Needed for LAR.
						double lar_scale_factor, double lar_padding); // RH 3/17/13 - LAR alpha/beta

int aodv_geo_compute_expand_flooding_angle( 
			InetT_Address_Hash_Table_Handle 	neighbor_connectivity_table, 	
			InetT_Address 						dest_addr, 
			double								src_x,
			double								src_y,
			int 								request_level, 
			AodvT_Geo_Table* 					geo_table_ptr,		
			int	   								aodv_type,
			Boolean								location_data_distributed, //MKA_VH 7/18/11
			double								dst_x, 
			double								dst_y,
			double								angle_expand);

Boolean aodv_geo_LAR_distance(double start_x, double start_y,  double mid_x,   double mid_y,  double end_x,   double end_y, double lar_scale_factor, double lar_padding);

double  aodv_geo_compute_angle(double start_x, double start_y,  double mid_x,   double mid_y,  double end_x,   double end_y);

Boolean aodv_rte_rreq_within_area(double computed_angle, int request_level, double angle_expand);

double  aodv_geo_vector_length(double start_x, double start_y,	double end_x,   double end_y);

Boolean aodv_geo_find_neighbor(AodvT_Geo_Table* geo_table_ptr,
										PrgT_List* 	     neighbor_list,	
										int 		     request_level,	
										double	src_x,   double src_y,
										double dest_x,   double dest_y,
										double angle_expand );

/* --------------- */
/* LAR Definitions */
/* --------------- */

// VHRCMA	11/11/10
#define LAR_UPDATE				((invoke_mode == OPC_PROINV_DIRECT) && (intrpt_type == OPC_INTRPT_SELF) && (intrpt_code == AODVC_LAR_UPDATE))


// VHRCMA	11/11/10
// This function will be called upon each LAR interrupt to update
// each node's LAR_Data information.
/* 2012/02/22 Added the parameter for the geo_table. The statistics collection needs this */
void aodv_geo_LAR_update(int, double, AodvT_Geo_Table*);

//MKA	11/23/10
typedef struct LAR_Data
{
	double x;
	double y;
	double velocity;
	double time;
} LAR_Data;

// MKA 11/23/10
// This is the category in which the LAR_Data will
// be stored in the global database.
static const char *LAR_OMS_CATEGORY = "LAR_DATA";
// RC 04/17/2012
// After finding a bug in sending hello messages we determined that the statistics were not
// being distributed. A global database of coordinates was used to circumvent this bug.
static const char *HELLO_OMS_CATEGORY = "HELLO_MESSAGE_DATA";


// MKA 11/25/10
// Retrieve the current node's IP address on the given output interface and store it in ip_str.
// RC 04/19/2012 Needed to change the scope of this so that external entities can see it
void get_node_ip(char*, IpT_Rte_Module_Data*, int);


// MKA 12/02/10
// Initialize LAR, including inserting the initial LAR_Data entry into the database.
void aodv_geo_LAR_init( IpT_Rte_Module_Data*, InetT_Addr_Family , double , double );


// MKA 11/23/10
// For some reason, I can't declare the function as inline, so
// the following macro allows us to perform velocity calculations inline.
#define aodv_geo_LAR_calc_velocity(oldX, oldY, newX, newY, oldTime, newTime) sqrt(pow(newY - oldY, 2) + pow(newX - oldX, 2))/(newTime - oldTime)


// MKA 11/23/10
// Comment out the following define to turn off LAR debug printing.
//#define LAR_DEBUG
//#define LAR_UPDATE_DEBUG

// MKA 12/02/10
// This method returns whether or not the current node is within the 
// request zone. The request zone will be as specified by LAR Scheme 1 (with Dan Urbano's alterations
// to take into account a source parallel to the destination).
Boolean aodv_geo_LAR_within_request_zone(double src_x, double src_y, double curr_x, double curr_y, double dest_x, double dest_y, double radius);

/* These can be found in aodv.h now
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
*/

// MKA 12/02/10
// Simple helper function that determines whether or not the given Point
// is within the bounds of the provided Rectangle.
Boolean aodv_geo_LAR_is_point_contained(Point2D *location, Rectangle *zone);

// MKA 12/02/10
// Retrieve LAR_Data from the global database using the given IP.
LAR_Data* aodv_geo_LAR_retrieve_data(char* ip);


/* ------------------- */
/* END LAR DEFINITIONS */
/* ------------------- */

#endif
