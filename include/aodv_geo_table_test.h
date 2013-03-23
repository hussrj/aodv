// 07/07/2010 RC
// Test funnctions for the table
// NOTE: THESE TEST FUNCTIONS ARE MEANT TO BE DELETED ONCE 
// THE FUNCTIONALITY OF THE TABLE HAS BEEN CONFIRMED
/* aodv_geo_table_test.c
 * Test the implementation of aodv_geo_table
 * for use in the geo aodv routing protocol.
 *
 * Author: Remo Cocco
 */

#ifndef _AODV_GEO_TABLE_TEST_H
#define _AODV_GEO_TABLE_TEST_H

#include <opnet.h>
#include <aodv.h>
#include <aodv_ptypes.h>
#include <math.h>

#define _ENTRY_EXISTS_TESTS 20
#define _UPDATE_TESTS 20
#define _REMOVE_TESTS 20

typedef struct InetT_Address_Helper
{
	InetT_Address *inett_address;
	double x;
	double y;
} InetT_Address_Helper;

void aodv_geo_table_test();

InetT_Address* generate_random_InetT_Address();

void aodv_geo_table_entry_exists_test(AodvT_Geo_Table*);
void aodv_geo_table_update_test(AodvT_Geo_Table*);

#endif /*_AODV_GEO_TABLE_TEST_H */
