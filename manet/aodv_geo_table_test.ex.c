#include <aodv_geo_table_test.h>
#include <aodv_geo_table.h>

InetT_Address* generate_random_InetT_Address()
{
   InetT_Address *inetT_address = malloc(sizeof(InetT_Address));
   
   //only test IPv4 
   //TODO Not sure what to set this too
   inetT_address->addr_family = 0;
   inetT_address->address.ipv4_addr = rand();

   printf("Returning random InetT_Address\n");
   return inetT_address;
}

void aodv_geo_table_entry_exists_test(AodvT_Geo_Table* geo_table_ptr)
{
    int i;
    InetT_Address_Helper *inett_address_helper;
	
	inett_address_helper = malloc(sizeof(InetT_Address_Helper));

    // first test to ensure that it can find entries
    // that do exist
    for (i = 0; i < _ENTRY_EXISTS_TESTS; i++)
    {
        inett_address_helper->inett_address = generate_random_InetT_Address();
		inett_address_helper->x = rand();
		inett_address_helper->y = rand();
        printf("Adding IPv4 addr %d (%u)", i, inett_address_helper->inett_address->address.ipv4_addr);
        aodv_geo_table_insert(geo_table_ptr, *inett_address_helper->inett_address, inett_address_helper->x, inett_address_helper->y, 0);
        //should always return true
        if (aodv_geo_table_entry_exists(geo_table_ptr, *inett_address_helper->inett_address))
        {
            printf("Entry does exist test (%d) passed!\n", i);
        }
        else
        {
            printf("Entry does exist test (%d) failed!\n", i);
        }
    }

    // here we are unsure of the outcome.
    for (i = 0; i < _ENTRY_EXISTS_TESTS; i++)
    {
        inett_address_helper->inett_address = generate_random_InetT_Address();
		
        if (aodv_geo_table_entry_exists(geo_table_ptr, *inett_address_helper->inett_address))
        {
            printf("The table does contain %u\n", inett_address_helper->inett_address->address.ipv4_addr);
        }
        else
        {
            printf("The table doesn't contain %u\n", inett_address_helper->inett_address->address.ipv4_addr);
        }
    }
}


void aodv_geo_table_update_test(AodvT_Geo_Table* geo_table_ptr)
{
	int i, j;
	InetT_Address_Helper	*inett_address_helper[_UPDATE_TESTS]; 
	AodvT_Geo_Entry 		*tmp_entry;
	char					ip_address[INETC_ADDR_STR_LEN];
//	InetT_Address_Helper *inett_address_helper;
	
	printf("\n\nStarting update tests(%d)\n\n",_UPDATE_TESTS);
	
	//*inett_address_helper = malloc(_UPDATE_TESTS * sizeof(InetT_Address_Helper));
	
	for (i = 0; i < _UPDATE_TESTS; i++)
	{
		inett_address_helper[i] = malloc(sizeof(InetT_Address_Helper));
		inett_address_helper[i]->inett_address = generate_random_InetT_Address();
		inett_address_helper[i]->x = rand();
		inett_address_helper[i]->y = rand();
		
		aodv_geo_table_insert(geo_table_ptr, *inett_address_helper[i]->inett_address, inett_address_helper[i]->x, inett_address_helper[i]->y, 0);
		
		inet_address_print (ip_address, *inett_address_helper[i]->inett_address);	
		//printf("Added %s with coordinate (%.0f, %.0f)\n", ip_address, inett_address_helper[i]->x, inett_address_helper[i]->y);
	}
	
	
	
	printf("\n\nFinished populating list, now running tests...\n\n");	
	
	for (i = 0; i < _UPDATE_TESTS; i++) 
	{
		for (j = rand() % 32; j > 0; j--)
		{
			// update the current ip address a random amount of times with random new x and y coordinates.
			inett_address_helper[i]->x = rand();
			inett_address_helper[i]->y = rand();
			
			aodv_geo_table_update (geo_table_ptr, *inett_address_helper[i]->inett_address, inett_address_helper[i]->x, inett_address_helper[i]->y, 0);
			
			
			// now that we have made our update, we want to retrieve the the x and y coordinates that the table has
			tmp_entry = aodv_geo_table_entry_get (geo_table_ptr, *inett_address_helper[i]->inett_address, OPC_FALSE);
			
			
			if (tmp_entry->dst_x == inett_address_helper[i]->x && tmp_entry->dst_y == inett_address_helper[i]->y)
			{
				printf("Test Passed!: ");
			}
			else
			{
				printf("Test failed!: ");
			}
			
			printf("was (%f, %f) is (%f, %f)\n", inett_address_helper[i]->x, inett_address_helper[i]->y, tmp_entry->dst_x, tmp_entry->dst_y);
				
		}
	}	
}

/**implicitly tests aodv_geo_table_entry_delete.
 * we also rely on the fact the exists was already tested, since it will be used to 
 * ensure that entries are properly removed.
 */
void aodv_geo_table_entry_remove_test(AodvT_Geo_Table* geo_table_ptr) {
	int i, j;
	InetT_Address_Helper	*inett_address_helper[_REMOVE_TESTS]; 
	//AodvT_Geo_Entry 		*tmp_entry;
	char					ip_address[INETC_ADDR_STR_LEN];
	InetT_Address			*rand_address; // used for testing
	InetT_Address_Helper	*tmp_helper; // used for swap after testing post conditions in the test loop.
	int randArrayIndex;
	
	printf("\n\nStarting remove tests(%d)\n\n",_UPDATE_TESTS);
	
	//*inett_address_helper = malloc(_UPDATE_TESTS * sizeof(InetT_Address_Helper));
	
	// first off we will insert several
	for (i = 0; i < _REMOVE_TESTS; i++)
	{
		inett_address_helper[i] = malloc(sizeof(InetT_Address_Helper));
		inett_address_helper[i]->inett_address = generate_random_InetT_Address();
		inett_address_helper[i]->x = rand();
		inett_address_helper[i]->y = rand();
		
		aodv_geo_table_insert(geo_table_ptr, *inett_address_helper[i]->inett_address, inett_address_helper[i]->x, inett_address_helper[i]->y, 0);
		
		inet_address_print (ip_address, *inett_address_helper[i]->inett_address);	
		//printf("Added %s with coordinate (%.0f, %.0f)\n", ip_address, inett_address_helper[i]->x, inett_address_helper[i]->y);
	}
	
	printf("\n\nFinished populating list ip address and coordinates, now running remove tests...\n\n");	
	
	// here we will try to remove all of the element that we added, in a random order.
	// we do so by getting a random index in the array. We will then put that element 
	// at the "end" of the list, so that it cannot be choosen the next time around (since
	// the random index is bounded by moding it with the loop counter.
	for(j = _REMOVE_TESTS; j > 0; j--)
	{
		randArrayIndex = rand(); // get a random index in the array
		randArrayIndex = randArrayIndex % j;
		
		rand_address = inett_address_helper[randArrayIndex]->inett_address;
	
		// try to remove, and index at random	
		aodv_geo_table_entry_remove (geo_table_ptr, *rand_address);
		
		inet_address_print (ip_address, *rand_address);
		// tests that it was removed
		if (aodv_geo_table_entry_exists(geo_table_ptr, *rand_address) == OPC_TRUE) 
		{
			printf("%s %S\n", "REMOVE TEST FAILED ON", ip_address);
		}
		else
		{
			printf("%s %s\n", "Remove test passed on", ip_address); 
		}
		
		// swap the removed element, so that it cannot be choosen again.
		tmp_helper = inett_address_helper[randArrayIndex];
		inett_address_helper[randArrayIndex] = inett_address_helper[j];
		inett_address_helper[j] = tmp_helper;
	}
	
}

void aodv_geo_table_test ()
{
	//create aodv_geo_table and call the proper tests with it
	AodvT_Geo_Table* table = aodv_geo_table_create(InetC_Addr_Family_v4);
	aodv_geo_table_entry_exists_test(table);
	
	table = aodv_geo_table_create(InetC_Addr_Family_v4);
	aodv_geo_table_update_test(table);
	
	table = aodv_geo_table_create(InetC_Addr_Family_v4);
	aodv_geo_table_entry_remove_test(table);
	free(table);
}
