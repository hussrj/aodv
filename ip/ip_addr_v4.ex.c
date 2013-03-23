/****************************************/
/*     Copyright (c) 1987-2009		*/
/*		by OPNET Technologies, Inc.		*/
/*       (A Delaware Corporation)      	*/
/*    7255 Woodmont Av., Suite 250     	*/
/*     Bethesda, MD 20814, U.S.A.       */
/*       All Rights Reserved.          	*/
/****************************************/

/* ip_addr_v4.ex.c: External code for IP address package. */

/****************************************/
/* !!!!! WARNING !!!!!					*/	
/* THIS FILE IS USED BY SIM AND ETS.	*/
/* THERE SHOULD BE NO OPC CONSTANTS		*/
/* OR KP CALLS IN THIS FILE.			*/
/****************************************/

/***** Includes *****/
#include <opnet.h>

/* IP specific header files */
#include <ip_addr_v4.h>
#include <oms_string_support.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#define _USE_MATH_DEFINES /* Needed for .NET	*/
#include <math.h>

/* Constants	*/
/* Visual C++ 6.0 does not have math constants	*/
#ifndef M_LN2
#define M_LN2		0.69314718055994530942
#endif

/* Macros to Get and Set IP address component based on byte index. 		*/
/* IP address occupies 32 bits. If the components are c0, c1, c2, c3	*/
/* where c0 is highest order byte and c3 is lowest order byte, address	*/
/* storage using formula: (c0<<24 || c1<<16 || c2<<8 || c3). An address	*/
/* component is retrieved using the reverse(right shift) of the formula.*/
#define Inet_Ipv4_Addr_Component_Get(addr,index) ((addr >> (24 - (index * 8))) & 255)
#define Inet_Ipv4_Addr_Component_Set(addr,index,value) (addr = (addr & \
	Inet_Ipv4_Addr_Component_Mask(index)) | (value << ( 24 - (index * 8)))) 

/* Get a mask which masks all address components except the index-th one*/
#define Inet_Ipv4_Addr_Component_Mask(index) (~(255 << (24 - index * 8)))
#define IPC_IPV4_ADDR_NUM_COMPONENTS	4	

/* Macros */
#define MAX(_a, _b)		(((_a) > (_b)) ? (_a) : (_b))
#define MIN(_a, _b)		(((_a) < (_b)) ? (_a) : (_b))

/* Local structures		*/
/* A 128-bit IPv6 address.		*/
typedef struct Ipv6T_Address
	{
	unsigned int			addr32 [4];
	} Ipv6T_Address;

/* Different types of fields in an IPv6	address.*/
/* Used in the ipv6_address_create function.	*/
typedef enum InetT_Ipv6_Addr_Field
	{
	InetC_Ipv6_Addr_Field_Hex_Number,
	InetC_Ipv6_Addr_Field_Double_Colon,
	InetC_Ipv6_Addr_Field_Ipv4_Address,
	InetC_Ipv6_Addr_Field_Invalid
	} InetT_Ipv6_Addr_Field;


/* Local functions		*/
static Ipv6T_Address*		ipv6_address_create (const char* ip_addr_str, int length);
static Ipv6T_Address*		ipv6_address_create_from_intf_id (const char* prefix_str, int intf_id);
static PrgT_Boolean				ipv6_address_string_test (const char* ip_addr_str);
static Ipv6T_Address*		ipv6_address_mask (const Ipv6T_Address* addr_ptr, InetT_Subnet_Mask subnet_mask);
static Ipv6T_Address*		ipv6_address_copy (const Ipv6T_Address* addr_ptr);
static Ipv6T_Address*		ipv6_address_complement (const Ipv6T_Address* addr_ptr);
static int					ipv6_address_compare (const Ipv6T_Address* addr1_ptr, const Ipv6T_Address* addr2_ptr);
static Ipv6T_Address*		ipv6_address_and (const Ipv6T_Address* addr1_ptr, const Ipv6T_Address* addr2_ptr);
static Ipv6T_Address*		ipv6_address_or (const Ipv6T_Address* addr1_ptr, const Ipv6T_Address* addr2_ptr);
static Ipv6T_Address*		ipv6_address_mem_alloc (void);
static char*				ipv6_address_print (char* addr_str, const Ipv6T_Address* addr_ptr);
static PrgT_Boolean				ipv6_address_equal_until_length (const Ipv6T_Address* addr1_ptr, const Ipv6T_Address* addr2_ptr, int length);
static int					ipv6_address_compare (const Ipv6T_Address* addr1_ptr, const Ipv6T_Address* addr2_ptr);
static PrgT_Compcode		ipv6_address_increment (Ipv6T_Address* 	addr_ptr);
static unsigned int*			inet_ipv6_addr_mem_alloc (void);
static InetT_Ipv6_Addr_Field	ipv6_address_field_type_get (const char* field_str);
static unsigned int*			inet_ipv6_addr_mem_alloc (void);
static void					inet_ipv6_address_mask (const unsigned int* orig_addr_ptr, unsigned int* masked_addr_ptr, int prefix_length);
static void					ip_addr_error_abort (const char* str1, const char* str2);
static void					ip_addr_error_recover (const char* str1, const char* str2);


#if defined (__cplusplus)
extern "C" {
#endif

#if defined (__cplusplus)
} /* end of 'extern "C" {' */
#endif

/* Inernal Macros.		*/
#define ipv6_address_equal(_addr1_ptr, _addr2_ptr)		((PrgT_Boolean) (((_addr1_ptr)->addr32[0] == (_addr2_ptr)->addr32[0]) && \
																	((_addr1_ptr)->addr32[1] == (_addr2_ptr)->addr32[1]) && \
																	((_addr1_ptr)->addr32[2] == (_addr2_ptr)->addr32[2]) && \
																	((_addr1_ptr)->addr32[3] == (_addr2_ptr)->addr32[3])))
#define ipv6_address_greater_than(_a1_ptr,_a2_ptr)		((PrgT_Boolean) (ipv6_address_compare (_a1_ptr,_a2_ptr) > 0))
#define ipv6_address_less_than(_a1_ptr,_a2_ptr)			((PrgT_Boolean) (ipv6_address_compare (_a1_ptr,_a2_ptr) < 0))
/* The first 8 bits of a multicast address will be all ones (255)	*/
#define ipv6_address_is_multicast(_addr_ptr)			((PrgT_Boolean) (IP_ADDRESS_COMPONENT_GET ((_addr_ptr)->addr32[0], 0) == 0xff))
#define Ipv6I_Solicited_Node_Mcast_Addr_Prefix			(&(std_ipv6_addresses[6]))

/* Useful IP addresses. */
IpT_Address 		IpI_Broadcast_Addr = IPC_ADDR_INVALID;
IpT_Address 		IpI_Default_Addr = IPC_ADDR_INVALID;
InetT_Address 		InetI_Invalid_Addr;
InetT_Address 		InetI_Invalid_v4_Addr;
InetT_Address 		InetI_Default_v4_Addr;
InetT_Address 		InetI_Broadcast_v4_Addr;
InetT_Address_Range	InetI_Addr_Range_Invalid;

InetT_Address		InetI_Ipv6_All_Nodes_NL_Mcast_Addr;
InetT_Address		InetI_Ipv6_All_Nodes_LL_Mcast_Addr;
InetT_Address		InetI_Ipv6_All_Routers_NL_Mcast_Addr;
InetT_Address		InetI_Ipv6_All_Routers_LL_Mcast_Addr;
InetT_Address		InetI_Ipv6_All_Routers_SL_Mcast_Addr;
InetT_Address		InetI_Default_v6_Addr;

InetT_Address		OspfI_IPv6_All_Spf_Routers;
InetT_Address		OspfI_IPv6_All_Dr_Routers;

/***** Internal Globals. *****/

/* Array to store subnet masks of various lengths	*/
IpT_Address			ip_subnet_masks [33];

/* Array used to store the lengths corresponding to	*/
/* 8 bit masks.										*/
unsigned char		ip_mask_lengths [256];

/* Integer to store the EUI-64 interface ID to be	*/
/* used for IPv6 addresses. It will incremented		*/
/* for each interface. A random value has been		*/
/* chosen to reduce the chance of a conflict with	*/
/* an explicitly configured address.				*/
unsigned int		IpI_Interface_ID = 0x9C840000;

/* String representation of the various Address		*/
/* Families.										*/
const char* addr_family_strings [] = {"IPv4", "IPv6", "Unspecified", "Invalid"};

/* Array for standard IPv6 addresses. Not using		*/
/* dynamic memory allocation here to make sure we	*/
/* do not accidently free this memory.				*/
Ipv6T_Address		std_ipv6_addresses[7];

static Pmohandle 			Ipv6I_Addr_Pmh;
static Pmohandle 			InetI_Addr_Pmh;
static Pmohandle 			InetI_Addr_Range_Pmh;
static Pmohandle			InetI_Addr_Str_Pmh;
static Cmohandle			InetI_Addr_Htable_Cmh;
PrgT_Boolean			ip_addr_initialization_complete = PRGC_FALSE;

/***** Macros *****/
#define InetI_Link_Local_Addr_Prefix	((unsigned int) 0xFE800000)

/***** Functions *****/
DLLEXPORT void
ip_address_pkg_init (void)
	{
	int			i;

	/** Perform any necessary initializations for the IP address package.        **/
	/** Currently, this just initializes the commonly used IP special addresses. **/
	FIN (ip_address_pkg_init (void));

	/* Set the initialized flag to true								*/
	/* Note that this needs to be done at the start of the function	*/
	/* because this function calls ip_address_create, which would	*/
	/* call this function causing a loop unless the					*/
	/* initialization_complete flag is set.							*/
	ip_addr_initialization_complete = PRGC_TRUE;

	/* Only initialize the addresses if they haven't yet been set.	*/
	IpI_Broadcast_Addr = ip_address_create ("255.255.255.255");
	InetI_Broadcast_v4_Addr = inet_address_from_ipv4_address_create (IpI_Broadcast_Addr);

	IpI_Default_Addr = ip_address_create ("0.0.0.0");
	InetI_Default_v4_Addr = inet_address_from_ipv4_address_create (IpI_Default_Addr);

	InetI_Invalid_v4_Addr = inet_address_from_ipv4_address_create (IPC_ADDR_INVALID);

	/* structure representing an invalid IP address.				*/
	InetI_Invalid_Addr.addr_family = InetC_Addr_Family_Invalid;
	InetI_Invalid_Addr.address.ipv4_addr = IPC_ADDR_INVALID;
	
	InetI_Addr_Range_Invalid.addr_family = InetC_Addr_Family_Invalid;
	InetI_Addr_Range_Invalid.address.ipv4_addr = IPC_ADDR_INVALID;

	/* Pool memory handle for IPv6 addresses.						*/
	Ipv6I_Addr_Pmh = prg_pmo_define ("IPv6 Addresses", sizeof (Ipv6T_Address), 128);
	InetI_Addr_Pmh = prg_pmo_define ("Inet Addresses", sizeof (InetT_Address), 128);
	InetI_Addr_Range_Pmh = prg_pmo_define ("Inet Address Ranges", sizeof (InetT_Address_Range), 128);

	/* Pool memory handle for Inet address strings.					*/
	InetI_Addr_Str_Pmh = prg_pmo_define ("Inet Address strings", INETC_ADDR_STR_LEN * sizeof (char), 32);

	/* Categorized memory handle for Address hash tables.			*/
	InetI_Addr_Htable_Cmh = prg_cmo_define ("Inet Address hash tables");

	/* Define all the standard IPv6 addresses.						*/
	/* The All Nodes (node local) multicast address.				*/
	std_ipv6_addresses[0].addr32[0] = (unsigned int) 0xFF010000;
	std_ipv6_addresses[0].addr32[1] = 0;
	std_ipv6_addresses[0].addr32[2] = 0;
	std_ipv6_addresses[0].addr32[3] = 1;
	InetI_Ipv6_All_Nodes_NL_Mcast_Addr.addr_family = InetC_Addr_Family_v6;
	InetI_Ipv6_All_Nodes_NL_Mcast_Addr.address.ipv6_addr_ptr = &(std_ipv6_addresses[0]);

	/* The All Nodes (link local) multicast address.				*/
	std_ipv6_addresses[1].addr32[0] = (unsigned int) 0xFF020000;
	std_ipv6_addresses[1].addr32[1] = 0;
	std_ipv6_addresses[1].addr32[2] = 0;
	std_ipv6_addresses[1].addr32[3] = 1;
	InetI_Ipv6_All_Nodes_LL_Mcast_Addr.addr_family = InetC_Addr_Family_v6;
	InetI_Ipv6_All_Nodes_LL_Mcast_Addr.address.ipv6_addr_ptr = &(std_ipv6_addresses[1]);

	/* The All Routers (node local) multicast address.				*/
	std_ipv6_addresses[2].addr32[0] = (unsigned int) 0xFF010000;
	std_ipv6_addresses[2].addr32[1] = 0;
	std_ipv6_addresses[2].addr32[2] = 0;
	std_ipv6_addresses[2].addr32[3] = 2;
	InetI_Ipv6_All_Routers_NL_Mcast_Addr.addr_family = InetC_Addr_Family_v6;
	InetI_Ipv6_All_Routers_NL_Mcast_Addr.address.ipv6_addr_ptr = &(std_ipv6_addresses[2]);

	/* The All Routers (link local) multicast address.				*/
	std_ipv6_addresses[3].addr32[0] = (unsigned int) 0xFF020000;
	std_ipv6_addresses[3].addr32[1] = 0;
	std_ipv6_addresses[3].addr32[2] = 0;
	std_ipv6_addresses[3].addr32[3] = 2;
	InetI_Ipv6_All_Routers_LL_Mcast_Addr.addr_family = InetC_Addr_Family_v6;
	InetI_Ipv6_All_Routers_LL_Mcast_Addr.address.ipv6_addr_ptr = &(std_ipv6_addresses[3]);

	/* The All Routers (site local) multicast address.				*/
	std_ipv6_addresses[4].addr32[0] = (unsigned int) 0xFF050000;
	std_ipv6_addresses[4].addr32[1] = 0;
	std_ipv6_addresses[4].addr32[2] = 0;
	std_ipv6_addresses[4].addr32[3] = 2;
	InetI_Ipv6_All_Routers_SL_Mcast_Addr.addr_family = InetC_Addr_Family_v6;
	InetI_Ipv6_All_Routers_SL_Mcast_Addr.address.ipv6_addr_ptr = &(std_ipv6_addresses[4]);

	/* The IPv6 default address (all zeroes).						*/
	std_ipv6_addresses[5].addr32[0] = 0;
	std_ipv6_addresses[5].addr32[1] = 0;
	std_ipv6_addresses[5].addr32[2] = 0;
	std_ipv6_addresses[5].addr32[3] = 0;
	InetI_Default_v6_Addr.addr_family = InetC_Addr_Family_v6;
	InetI_Default_v6_Addr.address.ipv6_addr_ptr = &(std_ipv6_addresses[5]);

	/* Prefix for solicited node multicast addresses.				*/
	/* RFC 3513. Sec 2.7.											*/
	std_ipv6_addresses[6].addr32[0] = (unsigned int) 0xFF020000;
	std_ipv6_addresses[6].addr32[1] = 0;
	std_ipv6_addresses[6].addr32[2] = 1;
	std_ipv6_addresses[6].addr32[3] = (unsigned int) 0xFF000000;

	/* Array of subnet masks of various lengths.					*/
	ip_subnet_masks [0] = 0;
	for (i = 1; i <= 32; i++)
		{
		ip_subnet_masks [i] = ((IpT_Address) 0xffffffff) << (32 - i);
		}

	/* Array used to map between a 8 bit mask and its length.		*/
	/* i.e. ip_mask_lengths [0b00000000] = 0,						*/
	/* 		ip_mask_lengths [0b10000000] = 1,						*/
	/* 		ip_mask_lengths [0b11000000] = 2,						*/
	/*		ip_mask_lengths [0b11111111] = 8 etc.					*/
	ip_mask_lengths [0] = 0;
	for (i = 1; i <= 8; i++)
		{
		ip_mask_lengths [(0xff << (8 - i)) & 0xff] = i;
		}

	/* OSPF related v6 multicast addresses. */
	OspfI_IPv6_All_Spf_Routers.address.ipv6_addr_ptr = (Ipv6T_Address*) 
		prg_mem_alloc (sizeof (Ipv6T_Address));
	OspfI_IPv6_All_Spf_Routers.address.ipv6_addr_ptr->addr32[0] = (unsigned int) 0xFF020000;
	OspfI_IPv6_All_Spf_Routers.address.ipv6_addr_ptr->addr32[1] = (unsigned int) 0;
	OspfI_IPv6_All_Spf_Routers.address.ipv6_addr_ptr->addr32[2] = (unsigned int) 0;
	OspfI_IPv6_All_Spf_Routers.address.ipv6_addr_ptr->addr32[3] = (unsigned int) 5;
	OspfI_IPv6_All_Spf_Routers.addr_family = InetC_Addr_Family_v6;
		
	OspfI_IPv6_All_Dr_Routers.address.ipv6_addr_ptr = (Ipv6T_Address*) 
		prg_mem_alloc (sizeof (Ipv6T_Address));
	OspfI_IPv6_All_Dr_Routers.address.ipv6_addr_ptr->addr32[0] = (unsigned int) 0xFF020000;
	OspfI_IPv6_All_Dr_Routers.address.ipv6_addr_ptr->addr32[1] = (unsigned int) 0;
	OspfI_IPv6_All_Dr_Routers.address.ipv6_addr_ptr->addr32[2] = (unsigned int) 0;
	OspfI_IPv6_All_Dr_Routers.address.ipv6_addr_ptr->addr32[3] = (unsigned int) 6;
	OspfI_IPv6_All_Dr_Routers.addr_family = InetC_Addr_Family_v6;


	FOUT;
	}

DLLEXPORT IpT_Address*
ip_address_create_dynamic (IpT_Address addr)
	{
	IpT_Address*		addr_ptr;

	/* Create a dynamically allocated IP address. Useful for maintaining lists	*/
	/* of addresses, primarily.													*/
	FIN (ip_address_create_dynamic (addr))

	/* Allocate using standard memory allocation, since Pooled memory would 		*/
	/* have excessive overhead for such a small object; furthermore, these types	*/
	/* of addresses are not typically expected to be deallocated and exist in very	*/
	/* great numbers. 																*/
	addr_ptr = (IpT_Address*) prg_mem_alloc (sizeof (IpT_Address));

	/* Assignt the specified address. */
	*addr_ptr = addr;

	FRET (addr_ptr);
	}

DLLEXPORT void
ip_address_destroy_dynamic (IpT_Address* addr_ptr)
	{
	/* Release memory for a dynamically created IP address. See comments above	*/
	/* in ip_address_create_dynamic for more details. 							*/
	FIN (ip_address_destroy_dynamic (IpT_Address* addr_ptr))

	prg_mem_free (addr_ptr);

	FOUT;
	}

DLLEXPORT int
ip_address_compare (IpT_Address addr1, IpT_Address addr2)
	{
	/* Compares two IP addresses						*/

	FIN (ip_address_compare (addr1, addr2));

	if (addr1 < addr2)
		{
		FRET (-1);
		}
	else if (addr1 > addr2)
		{
		FRET (1);
		}
	else
		{
		FRET (0);
		}
	}

DLLEXPORT IpT_Address
ip_address_create (const char *addr_str)
	{
	IpT_Address		addr = 0;

	/** Create an IP address based on a string in dotted decimal **/
	/** notation.  For example, to create the IP address for     **/
	/** 127.0.0.1, use ip_address_create ("127.0.0.1").          **/
	/* of addresses are not typically expected to be deallocated and exist in very	*/
	/* great numbers. 																*/
	FIN (ip_address_create (addr_str));

	/* Make sure the IP package is initialized, because some clients appear to call 	*/
	/* this early and without first initializing the IP package. 						*/
	ip_address_pkg_initialize();

	/* Call the support package function that parses the string.	*/
	if (PrgC_Compcode_Failure == inet_addr_str_parse_v4 (addr_str, &addr))
		addr = IPC_ADDR_INVALID;

	FRET (addr);
	}
	
DLLEXPORT PrgT_Compcode
ip_net_address_increment (IpT_Address* addr_ptr, IpT_Address_Class address_class)
	{
	int					i, first_byte;
	unsigned char		class_boundary;
	PrgT_Compcode		code = PrgC_Compcode_Success;
	unsigned char		addr_comp;
	
	/* Increment an IP network address by one, assuming default subnet mask given	*/
	/* the specified class. If incrementation cannot be performed without going to	*/
	/* another class, then return a failure. 										*/
	FIN (ip_net_address_increment (IpT_Address* addr_ptr, IpT_Address_Class address_class))

	/* Based on the specified class, determine which is the first byte	*/
	/* at which incrementation can begin, and the highest possible		*/
	/* value for the high order byte. 									*/
	switch (address_class)
		{
		case IPC_ADDRESS_CLASS_C:
			{
			first_byte = 3;
			class_boundary = 223;
			break;
			}
		case IPC_ADDRESS_CLASS_B:
			{
			first_byte = 2;
			class_boundary = 191;
			break;
			}
		case IPC_ADDRESS_CLASS_A:
			{
			first_byte = 1;
			class_boundary = 128;
			break;
			}
		}

	/* Loop through each of the octets in the modifiable range. Note that the	*/
	/* octets are arranged big-endian style: the zeroth octet is the			*/
	/* most signifcant one. We want to start incrementing in the lower			*/
	/* part of the address. 													*/
	for (i = first_byte - 1; i >= 0; i--)
		{
		addr_comp = IP_ADDRESS_COMPONENT_GET (*addr_ptr, i);
		
		/* Update the address with the incremented octet					*/
		IP_ADDRESS_COMPONENT_SET (*addr_ptr, i, ++addr_comp);

		/* If incrementing the octet exceeds 255, then let it go to zero, 	*/
		/* and continue the loop to increment the next one. Otherwise, exit	*/
		/* the loop. I.e., unless there is a carry-over, this loop performs	*/
		/* exactly one iteration. 											*/
		if (addr_comp != 0)
			{
			break;
			}
		else if  (i == 0)
			{
			code = PrgC_Compcode_Failure;
			}
		}

	/* If the incrementation caused the address to exit the class range, return an error. */
	if (code == PrgC_Compcode_Success)
		{
		if (IP_ADDRESS_COMPONENT_GET (*addr_ptr, 0) > class_boundary)
			code = PrgC_Compcode_Failure;
		}

	FRET (code)
	}


DLLEXPORT PrgT_Compcode
ip_address_increment (IpT_Address* addr_ptr, IpT_Address subnet_mask)
	{
	/** Increment an IP address by one, but remain within the specified **/
	/** subnet range. If the address was the last possible one in the	**/
	/** range, then return a failure code. 								**/

	FIN (ip_address_increment (IpT_Address* addr_ptr, IpT_Address subnet_mask));

	/* Increment the address.											*/
	++(*addr_ptr);

	/* If all the host bits are now 1, we have now reached the broadcast*/
	/* address which may not be assigned to an interface. return failure*/
	if (ip_address_is_broadcast (*addr_ptr, subnet_mask))
		{
		/* All the host bits of the current address are one.			*/
		FRET (PrgC_Compcode_Failure);
		}

	/* Return success.													*/
	FRET (PrgC_Compcode_Success);
	}

static PrgT_Compcode
ipv6_address_increment (Ipv6T_Address* 	addr_ptr)
	{
	int					i;
	PrgT_Compcode			status = PrgC_Compcode_Success;
	
	/** Increments an IPv6 address by 1. Handles 128 bit address.	**/
	FIN (ipv6_address_increment (addr_ptr));
	
	for (i = 3; i >= 0; i--)
		{
		if (addr_ptr->addr32 [i] < UINT_MAX)
			{
			addr_ptr->addr32 [i] ++;
			
			/* Set all lower order bits as well: */
			if(i == 0)
				{
				addr_ptr->addr32 [1] = addr_ptr->addr32 [2] = addr_ptr->addr32 [3] = 0;
				}
			if(i == 1)
				{
				addr_ptr->addr32 [2] = addr_ptr->addr32 [3] = 0;
				}
			if(i == 2)
				{
				addr_ptr->addr32 [3] = 0;
				}
				
			break;
			}
		}
	
	/* Signal an error if we have reached the max IPv6 address.	*/
	if (i==-1)
		status = PrgC_Compcode_Failure;
	
	FRET (status);
	}
	

DLLEXPORT PrgT_Compcode
inet_address_increment (InetT_Address* addr_ptr, InetT_Subnet_Mask smask)
	{
	PrgT_Compcode	success;
	/**Increments a copy of the specified address.		**/
	/** For ipv4 addresses, signals a failure if a 		**/
	/** broadcast address is hit.						**/
	
	FIN (inet_address_increment (addr_ptr, mask));

	/* Call the appropriate function depending on the	*/
	/* address family.									*/
	switch (addr_ptr->addr_family)
		{
		case InetC_Addr_Family_v4:
			/* Use ip_address_increment.				*/
			success = ip_address_increment (&(addr_ptr->address.ipv4_addr), ip_smask_from_inet_smask_create (smask));
			break;
		case InetC_Addr_Family_v6:
			success = ipv6_address_increment (addr_ptr->address.ipv6_addr_ptr);
			break;
		default:
			/* Error									*/
			ip_addr_error_abort ("Error in ip_addr_v4 (inet_address_increment)", 
				"The specified address belongs to an invalid address family");
			break;
		}

	FRET (success);
	}

DLLEXPORT InetT_Address
inet_solicited_node_mcast_addr_create (const InetT_Address ipv6_address)
	{
	InetT_Address		solicited_node_address;

	/** Return the solicited node multicast address		**/
	/** corresponding to the given address.				**/

	FIN (inet_solicited_node_mcast_addr_create (ipv6_address));
	
	/* Set the address family to IPv6.					*/
	solicited_node_address.addr_family = InetC_Addr_Family_v6;

	/* Initialize the IPv6 address to the solicited node*/
	/* multcast address prefix.							*/
	solicited_node_address.address.ipv6_addr_ptr = ipv6_address_copy (Ipv6I_Solicited_Node_Mcast_Addr_Prefix);

	/* Set the last 24 bits of the address to the last	*/
	/* 24 bits of the given address. 					*/
	solicited_node_address.address.ipv6_addr_ptr->addr32[3] |=
		((unsigned int) 0x00FFFFFF) & ipv6_address.address.ipv6_addr_ptr->addr32[3];

	/* Return the solicited node multicast address.		*/
	FRET (solicited_node_address);
	}

DLLEXPORT PrgT_Boolean
ip_address_is_broadcast (IpT_Address addr, IpT_Address subnet_mask)
	{
	IpT_Address		comp_subnet_mask;

	/** Returns PRGC_TRUE if an addr is a broadcast address with respect to **/
	/** subnet_mask.  Determine this by creating the complement of the     **/
	/** subnet mask, and masking that with the address.  If the result is  **/
	/** the same as the subnet mask complement, then the address is a      **/
	/** broadcast address.                                                 **/
	FIN (ip_address_is_broadcast (addr, subnet_mask));

	/* When the subnet mask is 31 bits, then the subnet is incapable	*/
	/* of sending broadcasts.  Both addresses in the subnet are host	*/
	/* addresses. If the mask is 32 bits, then the single address is	*/
	/* a host address.													*/
	if (subnet_mask > ip_smask_from_length_create (30))
		{
		FRET (PRGC_FALSE);
		}

	/* calculate the complement of the subnet mask. */
	comp_subnet_mask = ip_address_complement (subnet_mask);
	
	/* Mask the address with the complement and compare the result with	*/
	/* the mask complement.												*/
	FRET ((addr & comp_subnet_mask) == comp_subnet_mask);
	}

DLLEXPORT PrgT_Boolean
ip_address_is_network (IpT_Address addr, IpT_Address subnet_mask)
	{
	unsigned int	comp_subnet_mask;
	unsigned int	mask;
	
	/** Returns PRGC_TRUE if an addr is a network address with	**/
	/** respect to the subnet_mask. Determine this by creating	**/
	/** the complement of the mask, and masking that with the	**/
	/** address. If the result is zero, then the address is a	**/
	/** network address.										**/
	FIN (ip_address_is_network (addr, subnet_mask));
	
	/* If the subnet mask is all 1s, then the host address is	*/
	/* not considered a broadcast address.						*/
	if (ip_address_equal (subnet_mask, IpI_Default_Addr))
		{
		FRET (PRGC_FALSE);
		}
	
	/* If the mask length is 31, then the address is NOT a network 	*/
	/* address. Both addresses are considered hosts. If the mask	*/
	/* length is 32, then the address is a host address.			*/
	if (ip_smask_length_count (subnet_mask) > 30)
		{
		FRET (PRGC_FALSE);
		}
	
	comp_subnet_mask = ip_address_complement (subnet_mask);
	
	mask = ip_address_mask (addr, comp_subnet_mask);
	
	FRET (mask == 0);
	}



DLLEXPORT void
ip_address_print (char *str, IpT_Address addr)
	{
	/** Accept an IP address, and a pre-allocated string, and return the dotted **/
	/** decimal notation of the address in the string.                          **/
	FIN (ip_address_print (str, addr));

	/* Catch case where addr is Invalid. */
	if (addr == IPC_ADDR_INVALID)
		{
		strcpy (str, "Invalid");
		}
	else
		{
		/* create an address string out of the 32-bit decimal representation. */
		sprintf (str, "%d.%d.%d.%d",
			(addr >> 24) & 0xFF,
			(addr >> 16) & 0xFF,
			(addr >> 8) & 0xFF,
			(addr) & 0xFF);
		}

	FOUT;
	}

DLLEXPORT void
ip_intf_address_print (char *str, IpT_Address addr)
	{
	/** Similar to ip_address_print. The only difference being that	**/
	/** this function handles interfaces with no ip address. An 	**/
	/** interface set to No IP address will be internally given an	**/
	/** ip address of "0.0.0.0" (IpI_No_Ip_Addr). If the ip address	**/
	/** is set to this value, this function would print out No IP	**/
	/** Address instead of printing out the address literally.		**/

	FIN (ip_intf_address_print (str, addr));

	if (ip_address_equal (addr, IpI_No_Ip_Address))
		{
		strcpy (str, IPC_NO_IP_ADDRESS);
		}
	else
		{
		ip_address_print (str, addr);
		}
	FOUT;
	}
	
DLLEXPORT void
ip_address_print_debug (IpT_Address addr)
	{
	char 		temp_str [IPC_ADDR_STR_LEN];

	/** Print IP address to standard output.  Useful when using a C debugger. **/
	FIN (ip_address_print_debug (addr));

	ip_address_print (temp_str, addr);
	printf ("%s\n", temp_str);

	FOUT;
	}

DLLEXPORT IpT_Address_Range *
ip_address_range_create (IpT_Address addr, IpT_Address subnet_mask)
	{
	IpT_Address_Range *		addr_range_ptr;

	/** Create an address range based on an address and subnet mask.  The address **/
	/** range represents all legal IP addresses for the given net and subnet      **/
	/** numbers in the address (defined by the subnet mask).  Note that the input **/
	/** address and mask are not copied into the address range.                   **/
	FIN (ip_address_range_create (addr, subnet_mask_ptr));

	/* Allocate range object. */
	addr_range_ptr = (IpT_Address_Range *) prg_mem_alloc (sizeof (IpT_Address_Range));

	/* Set the components of the range. */
	addr_range_ptr->address = addr;
	addr_range_ptr->subnet_mask = subnet_mask;
	
	FRET (addr_range_ptr);
	}

DLLEXPORT IpT_Address_Range *
ip_address_range_copy (IpT_Address_Range *addr_range_ptr)
	{
	IpT_Address_Range *		copy_addr_range_ptr;

	/** Creates and returns a copy of the address range. **/
	FIN (ip_address_range_copy (addr_range_ptr));

	copy_addr_range_ptr = ip_address_range_create (addr_range_ptr->address,
		addr_range_ptr->subnet_mask);

	FRET (copy_addr_range_ptr);
	}

DLLEXPORT int
ip_address_ptr_compare (IpT_Address* addr_ptr1, IpT_Address* addr_ptr2)
	{
	/** Function that compares two pointers to ip addresses	**/
	/** It returns 0 if the two addresses are equal and 1 if**/
	/** they are not										**/

	FIN (ip_address_ptr_compare (addr_ptr1, addr_ptr2));

	if (ip_address_equal (*addr_ptr1, *addr_ptr2))
		{
		FRET (0);
		}
	else if (*addr_ptr1 < *addr_ptr2)
		{
		FRET (1);
		}
	else
		{
		FRET (-1);
		}
	}
		
DLLEXPORT void
ip_address_range_destroy (IpT_Address_Range *addr_range_ptr)
	{
	/** Deallocate an address range. **/
	FIN (ip_address_range_destroy (addr_range_ptr));

	if (addr_range_ptr == PRGC_NIL)
		FOUT;

	/* Then deallocate structure. */
	prg_mem_free (addr_range_ptr);

	FOUT;
	}

DLLEXPORT void
ip_address_range_print (char *addr_str, char *subnet_mask_str, IpT_Address_Range *addr_range_ptr)
	{
	/** Accept an IP address and two pre-allocated strings, and return the **/
	/** dotted decimal notation for the address and mask in the strings.   **/
	FIN (ip_address_range_print (addr_str, subnet_mask_str, addr_range_ptr));

	/* Use ip_address_print () to create strings for each component. */
	ip_address_print (addr_str, addr_range_ptr->address);
	ip_address_print (subnet_mask_str, addr_range_ptr->subnet_mask);

	FOUT;
	}

DLLEXPORT PrgT_Boolean
ip_address_range_check (IpT_Address addr, IpT_Address_Range *addr_range_ptr)
	{
	unsigned int		addr1;
	unsigned int		addr2;

	/** Given a particular IP address, return PRGC_TRUE if that address falls  **/
	/** in the given range, PRGC_FALSE if the address falls outside the range. **/
	/** This test is accomplished by simply masking the address with the      **/
	/** range subnet mask and masking the base address of the range with the  **/
	/** range subnet mask.  If the results of the masking are the same, then  **/
	/** the address falls within the address range.                           **/
	FIN (ip_address_range_check (addr, addr_range_ptr));

	/* Handle the case of the default address first. This address appears 	*/
	/* frequently due to auto-addressing or other uninitialized addresses. 	*/
	/* If the base or mask of the range is this default address then do 	*/
	/* not perform the comparison; simply return 'no match'. 				*/
	if (ip_address_equal (addr_range_ptr->address, IpI_Default_Addr) ||
		ip_address_equal (addr_range_ptr->subnet_mask, IpI_Default_Addr))
		{
		FRET (PrgC_Compcode_Failure);
		}

	addr1 = ip_address_mask (addr, addr_range_ptr->subnet_mask);
	addr2 = ip_address_mask (addr_range_ptr->address, addr_range_ptr->subnet_mask);

	FRET (addr1 == addr2);
	}	

DLLEXPORT IpT_Address_Class
ip_address_class (IpT_Address addr)
	{
	IpT_Address_Class	address_class;
	unsigned char		high_order_comp;

	/**	Return the address class of the specified IP address.	**/
	FIN (ip_address_class (addr));

	/*	Determine the IP address class based on the value of 	*/
	/*	the fisrt byte of the address.							*/
	high_order_comp = IP_ADDRESS_COMPONENT_GET (addr, 0);

	/* Each of the classes is represented by a standards-defined constant. */
	if ((high_order_comp > 239) && (high_order_comp < 256))
		{
		address_class = IPC_ADDRESS_CLASS_E;
		}	
	else if ((high_order_comp > 223) && (high_order_comp < 240))
		{
		address_class = IPC_ADDRESS_CLASS_D;
		}
	
	else if ((high_order_comp > 191) && (high_order_comp < 224))
		{
		address_class = IPC_ADDRESS_CLASS_C;
		}

	else if ((high_order_comp > 127) && (high_order_comp < 192))
		{
		address_class = IPC_ADDRESS_CLASS_B;
		}

	else if ((high_order_comp > 0) && (high_order_comp < 128))
		{
		address_class = IPC_ADDRESS_CLASS_A;
		}

	else 
		{
		
		address_class = IPC_ADDRESS_CLASS_UNSPEC;
		}

	FRET (address_class);
	}


DLLEXPORT IpT_Address
ip_default_smask_from_class (IpT_Address_Class address_class)
	{
	static PrgT_Boolean		initialized = PRGC_FALSE;
	static IpT_Address	smask_class_A;
	static IpT_Address	smask_class_B;
	static IpT_Address  smask_class_C;
	static IpT_Address 	smask_invalid;
	IpT_Address			snet_mask;
	
	/* Returns a default subnet mask (as defined in ip_default_smask_create ())	*/
	/* for the specified network class. 										*/
	FIN (ip_default_smask_from_class (IpT_Address_Clas address_class));
	
	if (initialized == PRGC_FALSE)
		{
		initialized = PRGC_TRUE;
		smask_class_A = ip_address_create ("255.0.0.0");
		smask_class_B = ip_address_create ("255.255.0.0");
		smask_class_C = ip_address_create ("255.255.255.0");
		smask_invalid = ip_address_create ("0.0.0.0");
		}
		
	switch (address_class)
		{
		case IPC_ADDRESS_CLASS_A:
			{
			snet_mask = smask_class_A;
			break;
			}

		case IPC_ADDRESS_CLASS_B:
			{
			snet_mask = smask_class_B;
			break;
			}

		case IPC_ADDRESS_CLASS_C:
			{
			snet_mask = smask_class_C;
			break;
			}

		default:
			{
			/* This is an unexpected condition; return an invalid subnet mask. */
			snet_mask = smask_invalid;	
			break;
			}
		}

	FRET (snet_mask)
	}


DLLEXPORT IpT_Address
ip_default_smask_create (IpT_Address ip_addr)
	{
	IpT_Address_Class	address_class;
	IpT_Address			snet_mask;

	/**	This procedure returns the default subnetwork mask		**/
	/**	based on the type of network address. A default subnet	**/
	/**	mask for a particular IP address class represents an 	**/
	/**	assignment of "1" for the bits used to represent the	**/
	/**	network portion of the address and "0" for the bits		**/
	/**	representing the hosts.									**/
	FIN (ip_default_smask_create (ip_addr));

	/*	Determine the address class of the IP address for which	*/
	/*	the subnet mask needs to be created.					*/
	address_class = ip_address_class (ip_addr);

	/* Based on the class, generate the default subnet mask. 	*/
	snet_mask = ip_default_smask_from_class (address_class);

	FRET (snet_mask);
	}


DLLEXPORT PrgT_Boolean
ip_smask_address_verify (IpT_Address address, IpT_Address mask)
	{
	PrgT_Boolean			mask_ok = PRGC_TRUE;
	int				field1, field2, field3, field4;

	/* Determine if the specifeid subnet mask and address are consistent. 	*/
	FIN (ip_smask_address_verify (IpT_Address address, IpT_Address mask))

	/*	Break down the subnet mask into fields for field-by-field comparison 	*/
	/* with minimum legal subnet masks (i.e., default masks). 					*/
	ip_address_fields_obtain (mask, &field1, &field2, &field3, &field4);


	/* Obtain the class of the specified address and switch off of it to 	*/
	/* analyze mask for valid values. A mask is valid if it is at least as 	*/
	/* long as the default subnet mask for the given address. 				*/
	switch (ip_address_class (address))
		{
		case IPC_ADDRESS_CLASS_A:
			{
			if (field1 != 255)
				mask_ok = PRGC_FALSE;
			break;
			}

		case IPC_ADDRESS_CLASS_B:
			{
			if ((field1 != 255) || (field2 != 255))
				mask_ok = PRGC_FALSE;
			break;
			}

		case IPC_ADDRESS_CLASS_C:
			{
			if ((field1 != 255) || (field2 != 255) || (field3 != 255))
				mask_ok = PRGC_FALSE;
			break;
			}

		default:
			{
			mask_ok = PRGC_TRUE;
			break;
			}
		}

	FRET (mask_ok)
	}

DLLEXPORT void
ip_address_fields_obtain (IpT_Address addr, int *field1, int *field2, int *field3, int *field4)
	{
	/**	This procedure accepts an IP address and returns the	**/
	/**	integer values of the four octets in the IP address.	**/
	FIN (ip_address_fields_obtain (addr, field1, field2, field3, field4));

	/*	Based on the IP address, set the appropriate return		*/
	/*	values.													*/
	*field1 = IP_ADDRESS_COMPONENT_GET (addr, 0);
	*field2 = IP_ADDRESS_COMPONENT_GET (addr, 1);
	*field3 = IP_ADDRESS_COMPONENT_GET (addr, 2);
	*field4 = IP_ADDRESS_COMPONENT_GET (addr, 3);

	FOUT;
	}

DLLEXPORT int
ip_smask_length_count (IpT_Address smask)
    {
	unsigned char*		fields_ptr;

    /** This procedure accepts an IP subnet masks and returns the  **/
    /** count of the leading ones in the mask.                     **/
    FIN (ip_smask_length_count (smask));

	/* Get a pointer to the 8 bit fields of the address.			*/
	fields_ptr = (unsigned char*) (&smask);

	FRET (ip_mask_lengths [fields_ptr [0]] + ip_mask_lengths [fields_ptr [1]] +
		  ip_mask_lengths [fields_ptr [2]] + ip_mask_lengths [fields_ptr [3]]);
	}

DLLEXPORT IpT_Address
Ip_Next_Available_Mcast_Addr (void)
	{
	int						addr_component_4; 
	static IpT_Address		Next_Mcast_Addr;
	static PrgT_Boolean			initial = PRGC_FALSE;
	
	/** Returns a valid next available multicast address.		**/
	/** A valid next available multicast address is a Class D	**/
	/** address, which lies between 224.0.0.13 and 224.0.0.255.	**/
	/** If a valid address is not available, IPC_ADDR_INVALID	**/
	/** is returned. 											**/
	FIN (Ip_Next_Available_Mcast_Addr());

	/** We start with the largest reserved multicast	**/
	/** address (PIM-SM uses 224.0.0.13), increment		**/
	/** the 4th component (13) and return the new		**/
	/** address. The largest address that can be		**/
	/** returned is 224.0.0.255.						**/
	
	/* Set the multicast address to start from.	*/
	if (initial == PRGC_FALSE)
		{
		Next_Mcast_Addr = IPC_INITIAL_MCAST_ADDR;
		initial = PRGC_TRUE;
		}

	/* Get the 4th component of the address.	*/
	addr_component_4 = IP_ADDRESS_COMPONENT_GET (Next_Mcast_Addr, 3);

	/* If it is with in the range, increment it	 */
	/* and assign the new address to be returned.*/
	if (addr_component_4 < 255)
		{
		addr_component_4 +=1;
		IP_ADDRESS_COMPONENT_SET (Next_Mcast_Addr, 3, addr_component_4);
		FRET (Next_Mcast_Addr);
		}
	else
		{
		/* All the multicast addresses in the	*/
		/* reserved range are assigned. Return	*/
		/* IPC_ADDR_INVALID.					*/
		FRET (IPC_ADDR_INVALID);
		}
	}

/* Ipv6T_Address related functions.			*/
static Ipv6T_Address*
ipv6_address_create (const char* orig_addr_str, int length)
	{
	Ipv6T_Address*		ret_addr_ptr;
	unsigned int*			v6_addr_array;
	
	/** Create an IPv6 address from the given string			**/
	/** representation. The address must be represented in one	**/
	/** of the forms described in 								**/
	/** RFC 2373 Sec.2.2 Text Representation of Addresses		**/
	/** The string can represent either a complete 128 bit 		**/
	/** address (length = 128) or a 64 bit prefix (length = 64)	**/
	
	FIN (ipv6_address_create (ip_addr_str));

	/* Call the support package function that will parse the	*/
	/* string and give us an array of 4 integers (128 bits).	*/
	if (inet_addr_str_parse_v6 (orig_addr_str, length, &v6_addr_array) == PrgC_Compcode_Success)
		{
		ret_addr_ptr = ipv6_address_mem_alloc ();

		/* Copy the 128 bits into the address.	*/
		prg_mem_copy (v6_addr_array, ret_addr_ptr->addr32, 16) ;

		/* Free the memory returned by the support package.	*/
		prg_mem_free (v6_addr_array);
		}
	else
		{
		ret_addr_ptr = PRGC_NIL;
		}

	/* Return the newly created address.						*/
	FRET (ret_addr_ptr);
	}

static Ipv6T_Address*
ipv6_address_create_from_intf_id (const char* prefix_str, int intf_id)
	{
	Ipv6T_Address*			ipv6_addr_ptr;

	/** Create an IPv6 address with the first 64 bits set	**/
	/** to the specified network prefix and the remaining	**/
	/** 64 bits set to the intf_id.							**/

	FIN (ipv6_address_create_from_intf_id (prefix_str, intf_id));

	/* Call the function that will parse the string and set	*/
	/* the first 64 bits of the address correctly.			*/
	ipv6_addr_ptr = ipv6_address_create (prefix_str, 64);

	/* If an error occurred, return NIL.					*/
	if (PRGC_NIL == ipv6_addr_ptr)
		{
		FRET (PRGC_NIL);
		}

	/* Set the lower order 64 bits of the address to the	*/
	/* interface ID.										*/
	ipv6_addr_ptr->addr32[2] = 0;
	ipv6_addr_ptr->addr32[3] = intf_id;

	FRET (ipv6_addr_ptr);
	}

static PrgT_Boolean
ipv6_address_string_test (const char* ip_addr_str)
	{
	Ipv6T_Address*		ipv6_addr_ptr;

	/** Checks whether the given string is a valid IPv6 address	**/
	/** or not.													**/

	FIN (ipv6_address_string_test (ip_addr_str));

	/* Call ipv6_address_create to parse the string. If the		*/
	/* string is invalid, it will return NIL.					*/
	ipv6_addr_ptr = ipv6_address_create (ip_addr_str, 128);

	if (PRGC_NIL == ipv6_addr_ptr)
		{
		/* Invalid string. Return false.						*/
		FRET (PRGC_FALSE);
		}
	else
		{
		/* Deallocate the memory allocated to the address and	*/
		/* return true.											*/
		prg_mem_free (ipv6_addr_ptr);

		FRET (PRGC_TRUE);
		}
	}

static Ipv6T_Address*
ipv6_address_mask (const Ipv6T_Address* addr_ptr, InetT_Subnet_Mask subnet_mask)
	{
	Ipv6T_Address*			masked_addr_ptr;
	int						mask_length;
	/** Apply the specified mask to the address	**/
	
	FIN (ipv6_address_mask (addr_ptr, subnet_mask));

	/* Get the length of the subnet mask.					*/
	mask_length = inet_smask_length_count (subnet_mask);

	/* Allocate enough memory to hold the masked address	*/
	masked_addr_ptr = ipv6_address_mem_alloc ();

	inet_ipv6_address_mask (addr_ptr->addr32, masked_addr_ptr->addr32, mask_length);

	/* Return the masked address							*/
	FRET (masked_addr_ptr);
	}

static PrgT_Boolean
ipv6_address_equal_until_length (const Ipv6T_Address* addr1_ptr, const Ipv6T_Address* addr2_ptr, int length)
	{
	PrgT_Boolean		addr_equal;
	int			i, num32bit_fields;
	
	/** Checks whether the first length bits of the two		**/
	/** addresses are the same.								**/

	FIN (ipv6_address_equal_until_length (addr1_ptr, addr2_ptr, length));

	/* If the length is exactly 128, just compare the		*/
	/* address directly.									*/
	if (128 == length)
		{
		addr_equal = ipv6_address_equal (addr1_ptr, addr2_ptr);
		FRET (addr_equal);
		}

	/* Find out the number of full 32 fields in the mask	*/
	num32bit_fields = length >> 5;

	/* Compare the full fields								*/
	for (i = 0; i < num32bit_fields; i++)
		{
		if (addr1_ptr->addr32[i] != addr2_ptr->addr32[i])
			{
			FRET (PRGC_FALSE);
			}
		}

	/* All the full 32 bit fields matched. Mask the next	*/
	/* field with a mask of length mask_length % 32 and 	*/
	/* compare the result.									*/
	addr_equal = (PrgT_Boolean) ((addr1_ptr->addr32[num32bit_fields] & ip_smask_from_length_create (length & 0x1F)) ==
							(addr2_ptr->addr32[num32bit_fields] & ip_smask_from_length_create (length & 0x1F)));
	
	FRET(addr_equal);
	}

static int
ipv6_address_compare (const Ipv6T_Address* addr1_ptr, const Ipv6T_Address* addr2_ptr)
	{
	int			i;

	/** This function returns +1, 0 or -1 if the address 	**/
	/** pointed to by addr1_ptr is greater than, equal to,	**/
	/** or less than the address pointed to by addr2_ptr	**/
	/** respectively.										**/
	
	FIN (ipv6_address_compare (addr1_ptr, addr2_ptr));

	/* Compare the 4 32 bit components of the address		*/
	/* starting with the most significant.					*/
	for (i = 0; i < 4; i++)
		{
		if (addr1_ptr->addr32[i] > addr2_ptr->addr32[i])
			{
			/* addr1 > addr 2. Return +1.					*/
			FRET (1);
			}
		else if (addr1_ptr->addr32[i] < addr2_ptr->addr32[i])
			{
			/* addr1 < addr 2. Return -1.					*/
			FRET (-1);
			}
		}

	/* If we are here it means that the two addresses are	*/
	/* equal. Return 0.										*/
	FRET (0);
	}

static Ipv6T_Address*
ipv6_address_copy (const Ipv6T_Address* addr_ptr)
	{
	Ipv6T_Address*		copy_addr_ptr;

	/** Creates a new IPv6 address structure with the same	**/
	/** contents as the given address.						**/

	FIN (ipv6_address_copy (addr_ptr));

	/* Allocate memory for a new IPv6 address structure.	*/
	copy_addr_ptr = ipv6_address_mem_alloc ();

	/* Copy the specified address into the new structure.	*/
	*copy_addr_ptr = *addr_ptr;

	/* Return the newly created address.					*/
	FRET (copy_addr_ptr);
	}

static Ipv6T_Address*
ipv6_address_complement (const Ipv6T_Address* addr_ptr)
	{
	Ipv6T_Address*		complement_addr_ptr;

	/** Creates a new IPv6 address structure whose value is	**/
	/** the complement of the given address.				**/

	FIN (ipv6_address_complement (addr_ptr));

	/* Allocate memory for a new IPv6 address structure.	*/
	complement_addr_ptr = ipv6_address_mem_alloc ();

	/* Set the fields in the new structure to be the		*/
	/* complements of the corresponding fields in the		*/
	/* original structure.									*/
	complement_addr_ptr->addr32[0] = ~(addr_ptr->addr32[0]);
	complement_addr_ptr->addr32[1] = ~(addr_ptr->addr32[1]);
	complement_addr_ptr->addr32[2] = ~(addr_ptr->addr32[2]);
	complement_addr_ptr->addr32[3] = ~(addr_ptr->addr32[3]);

	/* Return the newly created address.					*/
	FRET (complement_addr_ptr);
	}

static Ipv6T_Address*
ipv6_address_and (const Ipv6T_Address* addr1_ptr, const Ipv6T_Address* addr2_ptr)
	{
	Ipv6T_Address*		result;

	/** Creates a new IPv6 address structure whose value is	**/
	/** the complement of the given address.				**/

	FIN (ipv6_address_and (addr_ptr));

	/* Allocate memory for a new IPv6 address structure.	*/
	result = ipv6_address_mem_alloc ();

	/* Set the fields in the new structure to be the		*/
	/* complements of the corresponding fields in the		*/
	/* original structure.									*/
	result->addr32[0] = addr1_ptr->addr32[0] & addr2_ptr->addr32[0];
	result->addr32[1] = addr1_ptr->addr32[1] & addr2_ptr->addr32[1];
	result->addr32[2] = addr1_ptr->addr32[2] & addr2_ptr->addr32[2];
	result->addr32[3] = addr1_ptr->addr32[3] & addr2_ptr->addr32[3];

	/* Return the newly created address.					*/
	FRET (result);
	}

static Ipv6T_Address*
ipv6_address_or (const Ipv6T_Address* addr1_ptr, const Ipv6T_Address* addr2_ptr)
	{
	Ipv6T_Address*		result;

	/** Creates a new IPv6 address structure whose value is	**/
	/** the complement of the given address.				**/

	FIN (ipv6_address_or (addr_ptr));

	/* Allocate memory for a new IPv6 address structure.	*/
	result = ipv6_address_mem_alloc ();

	/* Set the fields in the new structure to be the		*/
	/* complements of the corresponding fields in the		*/
	/* original structure.									*/
	result->addr32[0] = addr1_ptr->addr32[0] | addr2_ptr->addr32[0];
	result->addr32[1] = addr1_ptr->addr32[1] | addr2_ptr->addr32[1];
	result->addr32[2] = addr1_ptr->addr32[2] | addr2_ptr->addr32[2];
	result->addr32[3] = addr1_ptr->addr32[3] | addr2_ptr->addr32[3];

	/* Return the newly created address.					*/
	FRET (result);
	}

static Ipv6T_Address*
ipv6_address_mem_alloc (void)
	{
	Ipv6T_Address*		addr_ptr;

	/** Allocates memory for an IPv6 address structure	**/
	
	FIN (ipv6_address_mem_alloc (void));

	/* Allocate memory from the pool memory handle.		*/
	addr_ptr = (Ipv6T_Address*) prg_pmo_alloc (Ipv6I_Addr_Pmh);


	FRET (addr_ptr);
	}

static char*
ipv6_address_print (char* addr_str, const Ipv6T_Address* addr_ptr)
	{
	unsigned short		msw, lsw;
	char*				addr_str_end_ptr;
	int					i;

	/** Creates a string representation of the given	**/
	/** IPv6 address.									**/
	/** The address will be represented as 8 colon		**/
	/** separated fields. Each field representing 16	**/
	/** in hexadecimal format.							**/
	/** The function returns a pointer to the given		**/
	/** string.											**/
	/*****************************************************/
	/** !!!!!!!!!!!!!  W A R N I N G  !!!!!!!!!!!!!!!!	**/
	/** Simulation code assumes that this function is	**/
	/** going to return 8 colon separated fields, i.e.	**/
	/** A:B:C:D:E:F:G:H. If this function compacts zero	**/
	/** fields using the double colon notation, or if	**/
	/** it prints embedded v4 addresses in v4 notation,	**/
	/** then the depending sim code will break. A 		**/
	/** different function must be written for that.	**/  
	/*****************************************************/
	FIN (ipv6_address_print (addr_str, addr_ptr));

	/* Initialize the string to a null string.			*/
	addr_str [0] = '\0';

	/* The local variable addr_str_end_ptr always points*/
	/* to the null character at the end of the string.	*/
	/* Initialize it also.								*/
	addr_str_end_ptr = addr_str;

	/* Loop through the fields and print them one by one*/
	for (i = 0; i < 4; i++)
		{
		/* Get the most significant 16 bits into one variable and the 	*/
		/* least significant bits into another variable. We cannot 		*/
		/* break it into 16 bit chunks by casting the address into 		*/
		/* OpT_uInt16* because this will flip the order in little-endian*/
		/* systems (e.g. pc_intel_win32, pc_intel_linux).				*/
		msw = addr_ptr->addr32 [i] >> 16;
		lsw = addr_ptr->addr32 [i] & 0x0000ffff;
		addr_str_end_ptr += sprintf (addr_str_end_ptr,
			"%X:%X:", msw, lsw);
		}

	/* Remove the trailing : that would have been printed.	*/
	*(addr_str_end_ptr - 1) = '\0';
	
	FRET (addr_str);
	}

/* InetT_Address related functions			*/
DLLEXPORT InetT_Address
inet_address_create (const char* ip_addr_str, InetT_Addr_Family addr_family)
	{
	InetT_Address			address;
	/** Function to create a address from a string.	**/

	FIN (inet_address_create (ip_addr_str, addr_family));

	/* Initialize the address package if it has not been*/
	/* done already.									*/
	ip_address_pkg_initialize();

	/* If the address family is not specified, try to	*/
	/* figure out if it is an IPv4 or an IPv6 address.	*/
	/* The easiest check is to look for a ':'. If a 	*/
	/* colon is present, it is an IPv6 address, 		*/
	/* otherwise an IPv4 address.						*/
	if (InetC_Addr_Family_Unknown == addr_family)
		{
		if (NULL != strchr (ip_addr_str, ':'))
			{
			addr_family = InetC_Addr_Family_v6;
			}
		else
			{
			addr_family = InetC_Addr_Family_v4;
			}
		}

	switch (addr_family)
		{
		case InetC_Addr_Family_v4:
			/* Set the addr_family field appropriately	*/
			address.addr_family = InetC_Addr_Family_v4;

			/* Use ip_address_create to parse the IPv4	*/
			/* address string.							*/
			address.address.ipv4_addr = ip_address_create (ip_addr_str);

			/* If there was an error, return an invalid	*/
			/* address.									*/
			if (ip_address_equal (address.address.ipv4_addr, IPC_ADDR_INVALID))
				{
				address.addr_family = InetC_Addr_Family_Invalid;
				}
			break;
		case InetC_Addr_Family_v6:
			/* Set the addr_family field appropriately	*/
			address.addr_family = InetC_Addr_Family_v6;

			/* Use ipv6_address_create to parse the IPv6*/
			/* address string.							*/
			address.address.ipv6_addr_ptr = ipv6_address_create (ip_addr_str, 128);

			/* If there was an error, return an invalid	*/
			/* address.									*/
			if (PRGC_NIL == address.address.ipv6_addr_ptr)
				{
				address.addr_family = InetC_Addr_Family_Invalid;
				}
			break;
		default:
			/* Error: Terminate simulation.				*/
			ip_addr_error_abort ("Error in ip_addr_v4 (inet_address_create):",
			   	"The value specified for addr_family is invalid.");
			break;
		}

	/* Return the address we created.					*/
	FRET (address);
	}


PrgT_Compcode
inet_addr_str_parse_v4 (const char* addr_str, unsigned int* v4_addr_ptr)
	{
	unsigned int		addr = 0;
	int				addr_int_component;
	int				current_addr_comp;
	int				current_comp_str_pos;
	int				str_pos;

	/********************************************************************/
	/* Purpose: 	Parse an IP v4 address string and return value.		*/
	/*																	*/
	/* Inputs:		string to check										*/
	/*																	*/
	/* Outputs:		PrgC_Compcode_Success or FAILURE					*/
	/*				32 bit v4 address 									*/
	/*																	*/
	/********************************************************************/
	FIN (inet_addr_str_parse_v4 ());
	
	current_addr_comp = 0;
	current_comp_str_pos = 0;

	for (str_pos = 0; ;str_pos++)
		{
		/* Look for the end of the current component, indicated by a separator or termination. */
		if (addr_str [str_pos] == '.' || addr_str [str_pos] == 0)
			{
			/* The current component has ended; process it first as an integer to test its range. 	*/
			/* Check to see if it's a proper range. 												*/
			addr_int_component = atoi (addr_str + current_comp_str_pos);
			if ((addr_int_component > 255) || (addr_int_component < 0))
				{
				FRET (PrgC_Compcode_Failure);
				}

			/* It's valid: load the component into the address at the current position. */
			Inet_Ipv4_Addr_Component_Set (addr, current_addr_comp, addr_int_component);
			
			/* Move on to the next component. */
			current_addr_comp ++;

			/* If we have attained the number of supported address components, stop here. 	*/
			/* We won't consider it an error that there is more there -- we will just		*/
			/* ignore it for performance's sake. 											*/
			if (current_addr_comp == IPC_IPV4_ADDR_NUM_COMPONENTS)
				{
				break;
				}

			/* Move on to the next part of the string, which begins right after the separator "." */
			current_comp_str_pos = str_pos + 1;
			}
		/* commenting out the following code because we want to allow addr/prefix-len notation */
		else
			{
			/* Not a dot or end of string. Must be a digit or '/' (CIDR notation).	!!!*/
			if (!isdigit ((unsigned char) addr_str [str_pos]) && 
				(addr_str [str_pos] != '/'))
				{
				FRET (PrgC_Compcode_Failure);
				}
			}
		/* If we have processed the terminating character of the string, we are done. */
		if (addr_str [str_pos] == 0)
			break;
		}

	/* Note: If we exit the loop before building a full address, this is a so-called zero			*/
	/* extension form of the address. I.e., 123.45.6 represents 123.45.6.0. This is automatically	*/
	/* handled because the address is zero-initialized in its declaration above. 					*/
	*v4_addr_ptr = addr;	
	FRET (PrgC_Compcode_Success);
	}

PrgT_Compcode
inet_addr_str_parse_v6 (const char* addr_str, int length, unsigned int** v6_addr_ptr)
	{
	char				addr_str_copy [256];
	int					i, num_fields;
    unsigned int			*ret_addr_ptr;
	OpT_uInt16			addr_field_arr [8];
	int					addr_index;
	char*				ith_field;
	InetT_Ipv6_Addr_Field	ith_field_type;
	char*				next_field;
	int					double_colon_field_index = -1;
	PrgT_Boolean		error = PRGC_FALSE;
	unsigned int			ipv4_address;

	/********************************************************************/
	/* Purpose: 	Parse an IP v6 address string and return value.		*/
	/*																	*/
	/* Inputs:		string to check										*/
	/*				prefix length 										*/
	/*					- 64 for EUI-64 addresses						*/			
	/*					- 128 for non-EUI-64 addresses					*/
	/*																	*/
	/* Outputs:		PrgC_Compcode_Success or FAILURE					*/
	/*				pointer to a 4 int array v6 addr 					*/
	/*																	*/
	/********************************************************************/
	FIN (inet_addr_str_parse_v6 ());

	/* Create a local copy of the address string.				*/
	strcpy (addr_str_copy, addr_str);

	/* Remove any white space characters from the string.		*/
	oms_string_white_space_trim (addr_str_copy);

	/* An IPv6 address consists of a number of fields separated	*/
	/* by colons. Each field can be in one of three possible 	*/
	/* forms.													*/
	/* 1) A 16 bit value represented in hexadecimal format.		*/
	/*	  Leading zeros may be left out.						*/
	/* 2) An empty string indicating multiple groups of 16-bits	*/
	/*	  of zeros. This type of field can occur only once		*/
	/* 3) A 32 bit IPv4 address. This can only be the last field*/
	/*	  in the string.										*/

	/* Count the number of colons in the strings. The number of	*/
	/* fields will be one more than the number of colons.		*/
	num_fields = oms_string_char_count (addr_str_copy, ':') + 1;

	/* The maximum number of fields possible is (length/16 + 2).*/
	/* The two additonal fields could be because of a double	*/
	/* colon as in ::0:0:0:0:0:0:0:0.							*/
	if (num_fields > ((length >> 4) + 2))
		{
		/* Error. Return failure.								*/
		*v6_addr_ptr = PRGC_NIL;
		FRET (PrgC_Compcode_Failure);
		}

	/* Initialize the pointer to the ith_field to the start of	*/
	/* the string.												*/
	ith_field = addr_str_copy;

	/* Loop through the fields one by one.						*/
	for (i = 0, addr_index = 0; i < num_fields; i++)
		{
		/* The end of the field will be indicated by a colon.	*/

		/* Get a pointer to this character.						*/
		next_field = strchr (ith_field, ':');

		/* If this the last field strchr will return null, If 	*/
		/* this is the case no need to do anything more. If not,*/
		/* replace the colon with a null character so that		*/
		/* ith_field points to just the current field. Also 	*/
		/* point the next_field to the character just following	*/
		/* the colon so that it points to the start of the		*/
		/* next field.											*/
		if (NULL != next_field)
			{
			*next_field = '\0';
			++next_field;
			}

		/* Find out what type of field it is.					*/
		ith_field_type = ipv6_address_field_type_get (ith_field);

		switch (ith_field_type)
			{
			case InetC_Ipv6_Addr_Field_Hex_Number:
				/* Make sure we haven't set all the fields		*/
				/* already.										*/
				if (addr_index >= (length >> 4))
					{
					/* Flag an error.							*/
					error = PRGC_TRUE;
					break;
					}

				/* Parse the string and set the	appropriate 16	*/
				/* bits of the address.							*/
				addr_field_arr [addr_index] = strtoul (ith_field, NULL, 16);

				/* Increment the addr_index.					*/
				++addr_index;
				break;

			case InetC_Ipv6_Addr_Field_Double_Colon:
				/* If the double colon occurs at the beginning	*/
				/* or end of the string we will have a bogus	*/
				/* empty field. Ignore such fields				*/
				if ((i ==0) || (i == num_fields - 1))
					{
					/* Ignore.									*/
					break;
					}

				/* The double colon can occur only once. If it	*/
				/* has already occurred, signal an error.		*/
				if (-1 != double_colon_field_index)
					{
					error = PRGC_TRUE;
					break;
					}

				/* This is a valid double colon. Store its index*/
				double_colon_field_index = addr_index;
				break;

			case InetC_Ipv6_Addr_Field_Ipv4_Address:
				/* Make sure there are atleast two unset fields	*/
				/* in the address.								*/
				if (addr_index >= 7)
					{
					/* Flag an error.							*/
					error = PRGC_TRUE;
					break;
					}

				/* Set the last 32 bits in the address to the	*/
				/* IPv4 address.								*/
				/* But note that we cannot directly cast the	*/
				/* pointer to (unsigned int*) because it might	*/
				/* not be at a 4 byte boundary and hence could	*/
				/* cause a bus error. Even memcopy is not enough*/
				/* since we want the IPv6 address to be in		*/
				/* network byte order, but the IPv4 address is	*/
				/* stored in host byte order.					*/
				/* So set the higher and lower order 16 bits	*/
				/* separately.									*/
				/* Create the IPv4 address corresponding to the	*/
				/* given string.								*/
				inet_addr_str_parse_v4 (ith_field, &ipv4_address);

				/* Store the higher order 16 bits.				*/
				/* Shift the IPv4 address to the right by 16	*/
				/* get the higher order 16 bits.				*/
				addr_field_arr [addr_index] = (OpT_uInt16) (ipv4_address >> 16);

				/* Now the lower order 16 bits.					*/
				/* Mask the address with 0x0000ffff to get the	*/
				/* lower order 16 bits.							*/
				addr_field_arr [addr_index + 1] = (OpT_uInt16) (ipv4_address & 0x0000ffff);

				/* Increment the addr_index by 2 to indicate 	*/
				/* that two 16 bit fields in the address were	*/
				/* set.											*/
				addr_index += 2;

				break;

				default:
				/* Illegal field. Flag an error.				*/
				error = PRGC_TRUE;
				break;
			} /* switch (...)	*/

		/* If there was an error, free memory and return NIL	*/
		if (error)
			{
			*v6_addr_ptr = PRGC_NIL;
			FRET (PrgC_Compcode_Failure);
			}

		/* Make ith_field point to the next field.				*/
		ith_field = next_field;
		} /* for (i = 0; ... )	*/

	/* If the required number of fields are not present and		*/
	/* there was not double colon field, it is an error.		*/
	if ((addr_index != (length >> 4)) && (-1 == double_colon_field_index))
		{
		/* Free memory and return NIL.							*/
		*v6_addr_ptr = PRGC_NIL;
		FRET (PrgC_Compcode_Failure);
		}

	/* If a double colon field was present and the specified	*/
	/* address did not have enough fields, insert enough zeros	*/
	/* at the appropriate position.								*/
	if ((addr_index != (length >> 4)) && (-1 != double_colon_field_index))
		{
		/* The number of 16-bit zero fields to be inserted is	*/
		/* (length/16 - addr_index) and they need to be inserted*/
		/* starting at position double_colon_field_index.		*/

		/* Shift all the fields starting from the one at		*/
		/* double_colon_field_index by							*/
		/* ((length >> 4) - addr_index) fields.					*/
		
		/* The arguments to memmove will be as follows.			*/
		/* To: addr_field_arr + double_colon_field_index + 		*/
		/*						((length >> 4) - addr_index)	*/
		/* From: addr_field_arr + double_colon_field_index		*/
		/* Size: (addr_index - double_colon_field_index)*2 bytes*/

		memmove (addr_field_arr + (double_colon_field_index + (length >> 4) - addr_index),
				 addr_field_arr + double_colon_field_index, (addr_index - double_colon_field_index) * 2);

		/* Now set the ((length >> 4) - addr_index) fields	 	*/
		/* starting at double_colon_field_index to 0.			*/
		memset (addr_field_arr + double_colon_field_index, 0, ((length >> 4) - addr_index) * 2);
		}


	/* Allocate enough memory to hold the address.				*/
	ret_addr_ptr = inet_ipv6_addr_mem_alloc ();

	/* Build the v6 address from the array of 16 bit integers.	*/
	/* Each of the four integers is the concatenation of two 	*/
	/* contiguous 16-bit numbers.								*/
	/* We cannot directly do a memcpy because that will swap	*/	
	/* the lower order and higher order word in little-endian	*/
	/* systems.													*/
	for (i = 0; i < 4; i++)
		{
		OpT_uInt32 			msw, lsw;
		msw = addr_field_arr [2*i];
		lsw = addr_field_arr [2*i + 1];
		ret_addr_ptr [i] = (msw << 16 | lsw);
		}
	   	
	/* Return the newly created address.						*/
	*v6_addr_ptr = ret_addr_ptr;
	FRET (PrgC_Compcode_Success);
	}

static unsigned int*
inet_ipv6_addr_mem_alloc (void)
	{
	static PrgT_Boolean		first_time = PRGC_TRUE;
	static Cmohandle		ipv6_cmh;
	
	/* Allocate memory for 4 integers (128 bit IPv6 address).	*/
	FIN (inet_ipv6_addr_mem_alloc ());

	if (first_time)
		{
		ipv6_cmh = prg_cmo_define ("ip_addr_v4-128 bit v6 addr");
		first_time = PRGC_FALSE;
		}
	
	FRET ((unsigned int *) (prg_cmo_alloc (ipv6_cmh, 4 * sizeof (unsigned int))));
	}
	
static InetT_Ipv6_Addr_Field
ipv6_address_field_type_get (const char* field_str)
	{
	int	i = 0;
	size_t length = 0;
	InetT_Ipv6_Addr_Field	addr_field;
	
	/** An IPv6 address consists of colon separated fields		**/
	/** There are three types of fields.						**/
	/** 1) A 16 bit value represented in hexadecimal format.	**/
	/**	  Leading zeros may be left out.						**/
	/** 2) An empty string indicating multiple groups of 16-bits**/
	/**	  of zeros. 											**/
	/** 3) A 32 bit IPv4 address. 								**/
	/** This function is used by the ipv6_address_create		**/
	/** function to find out the type of a given field 			**/

	FIN (ipv6_address_field_type_get (field_str));

	/* The easiest way of identifying the field is to look at	*/
	/* the length of the string.								*/
	/* Hexadecimal number fields: 1-4 (both incl)				*/
	/* Double colon field: 0									*/
	/* IPv4 address field: 7-15.								*/

	/* Get the length of the field.								*/
	length = strlen (field_str);

	/* Determine the field type based on the length.			*/
	if (0 == length)
		{
		/* Double colon.										*/
		addr_field = InetC_Ipv6_Addr_Field_Double_Colon;
		}
	else if (length <= 4)
		{
		/* Has to be hexadecimal number. Make sure all the		*/
		/* characters are hexadecimal digits.					*/
		for (i = 0; i < length; i++)
			{
			if (! isxdigit ((unsigned char) field_str [i]))
				{
				/* Error.										*/
				FRET (InetC_Ipv6_Addr_Field_Invalid);
				}
			}
		/* A valid hexadecimal number.							*/
		addr_field = InetC_Ipv6_Addr_Field_Hex_Number;
		}
	else if ((length >= 7) && (length <= 15))
		{
		/* Make sure the string represents a valid IP address	*/
		if (inet_addr_str_is_valid_v4 (field_str))
			{
			addr_field = InetC_Ipv6_Addr_Field_Ipv4_Address;
			}
		else
			{
			/* Error.											*/
			addr_field = InetC_Ipv6_Addr_Field_Invalid;
			}
		}
	else
		{
		/* Invalid length. Error.								*/
		addr_field = InetC_Ipv6_Addr_Field_Invalid;
		}

	FRET (addr_field);
	}

DLLEXPORT InetT_Address
inet_ipv6_address_from_bits_create (unsigned int* addr_array)
    {
    InetT_Address           addr;

    FIN (inet_ipv6_address_from_bits_create ());

    addr.addr_family = InetC_Addr_Family_v6;
    addr.address.ipv6_addr_ptr = ipv6_address_mem_alloc ();
    prg_mem_copy (addr_array, addr.address.ipv6_addr_ptr->addr32, IPC_V6_ADDR_LEN_BYTES);

    FRET (addr);
    }

DLLEXPORT InetT_Address
inet_address_create_from_intf_id (const char* prefix_str, int intf_id)
	{
	InetT_Address				address;

	/** Create an IPv6 address with the first 64 bits set	**/
	/** to the specified network prefix and the remaining	**/
	/** 64 bits set to the intf_id.							**/

	FIN (inet_address_create_from_intf_id (ip_addr_str, intf_id));

	/* Make sure the IP package is initialized, because some clients appear to call 	*/
	/* this early and without first initializing the IP package. 						*/
	ip_address_pkg_initialize();

	/* Set the ipv6 address also correctly.					*/
	address.address.ipv6_addr_ptr = ipv6_address_create_from_intf_id (prefix_str, intf_id);

	/* If an error occured Return an invalid address.		*/
	if (PRGC_NIL == address.address.ipv6_addr_ptr)
		{
		FRET (INETC_ADDRESS_INVALID);
		}

	/* Set the address family correctly.					*/
	address.addr_family = InetC_Addr_Family_v6;

	FRET (address);
	}

DLLEXPORT InetT_Address
inet_ipv6_link_local_addr_create (int intf_id)
	{
	InetT_Address		link_local_addr;

	/** Create an IPv6 link local address with the specified**/
	/** interface ID.										**/

	FIN (inet_ipv6_link_local_addr_create (intf_id));

	/* Make sure the IP package is initialized, because some clients appear to call 	*/
	/* this early and without first initializing the IP package. 						*/
	ip_address_pkg_initialize();

	/* Set the address family of the address to IPv6.		*/
	link_local_addr.addr_family = InetC_Addr_Family_v6;

	/* Allocate enough memory for the address.				*/
	link_local_addr.address.ipv6_addr_ptr = ipv6_address_mem_alloc ();

	/* Set the first 32 bits of the address to the standard	*/
	/* link local address prefix FE80:: RFC 2373 Sec. 2.5.8	*/
	link_local_addr.address.ipv6_addr_ptr->addr32[0] = InetI_Link_Local_Addr_Prefix;

	/* Set the next 32 bits to 0.							*/
	link_local_addr.address.ipv6_addr_ptr->addr32[1] = 0;

	/* Set the remaining 64 bits to the interface ID.		*/
	link_local_addr.address.ipv6_addr_ptr->addr32[2] = 0;
	link_local_addr.address.ipv6_addr_ptr->addr32[3] = intf_id;

	FRET (link_local_addr);
	}

DLLEXPORT PrgT_Boolean
ipv6_addr_is_link_local (struct Ipv6T_Address* ipv6_addr_ptr)
	{
	/** Checks whether a given IPv6 address is a link	**/
	/** local address or not. The first 64 bits of a 	**/
	/** link-local address will be set to FE80::.		**/

	FIN (ipv6_addr_is_link_local (ipv6_addr_ptr));

	FRET ((PrgT_Boolean) ((ipv6_addr_ptr->addr32[0] == InetI_Link_Local_Addr_Prefix) &&
					 (ipv6_addr_ptr->addr32[1] == 0)));
	}

DLLEXPORT InetT_Address*
inet_address_mem_alloc (void)
	{
	InetT_Address*			addr_ptr;

	/** Allocates memory for an InetT_Address structure.	**/
	
	FIN (inet_address_mem_alloc (void));

	/* Allocate memory from the pool memory object.			*/
	addr_ptr = (InetT_Address*) prg_pmo_alloc (InetI_Addr_Pmh);

	FRET (addr_ptr);
	}

DLLEXPORT char*
inet_address_str_mem_alloc (void)
	{
	char*			addr_str;

	/** Allocates memory for an address string.			**/
	
	FIN (inet_address_str_mem_alloc (void));

	/* Allocate memory from the pool memory object.		*/
	addr_str = (char*) prg_pmo_alloc (InetI_Addr_Str_Pmh);

	FRET (addr_str);
	}

DLLEXPORT InetT_Address*
inet_address_create_dynamic (InetT_Address address)
	{
	InetT_Address*			copy_addr_ptr;

	/** Allocates enough memory for an address structure	**/
	/** and copies the contents of the existing structure 	**/
	/** into it.											**/

	FIN (inet_address_create_dynamic (addr_ptr));

	/* Allocate enough memory first.					*/
	copy_addr_ptr = inet_address_mem_alloc ();

	/* First copy the address family value.				*/
	copy_addr_ptr->addr_family = address.addr_family;

	/* For IPv6 addresses, do a deep copy on the address*/
	if (InetC_Addr_Family_v6 == address.addr_family)
		{
		copy_addr_ptr->address.ipv6_addr_ptr = ipv6_address_copy (address.address.ipv6_addr_ptr);
		}
	else
		{
		copy_addr_ptr->address.ipv4_addr = ip_address_copy (address.address.ipv4_addr);
		}

	FRET (copy_addr_ptr);
	}
	

DLLEXPORT InetT_Address*
inet_address_copy_dynamic (InetT_Address* addr_ptr)
	{
	InetT_Address*			copy_addr_ptr;

	/** Allocates enough memory for an address structure	**/
	/** and copies the contents of the existing structure 	**/
	/** into it.											**/

	FIN (inet_address_copy_dynamic (addr_ptr));

	/* Allocate enough memory first.					*/
	copy_addr_ptr = inet_address_mem_alloc ();

	/* First copy the address family value.				*/
	copy_addr_ptr->addr_family = addr_ptr->addr_family;

	/* For IPv6 addresses, do a deep copy on the address*/
	if (InetC_Addr_Family_v6 == addr_ptr->addr_family)
		{
		copy_addr_ptr->address.ipv6_addr_ptr = ipv6_address_copy (addr_ptr->address.ipv6_addr_ptr);
		}
	else
		{
		copy_addr_ptr->address.ipv4_addr = ip_address_copy (addr_ptr->address.ipv4_addr);
		}

	FRET (copy_addr_ptr);
	}
	
DLLEXPORT InetT_Address
inet_address_from_ipv4_address_create (IpT_Address ipv4_address)
	{
	InetT_Address		ip_addr;
	/** Converts an IpT_Address to an InetT_Address		**/
	
	FIN (inet_address_from_ipv4_address_create (ipv4_address));

	/* Set the address family to IPv4					*/
	ip_addr.addr_family = InetC_Addr_Family_v4;

	/* Store the actulal address.						*/
	ip_addr.address.ipv4_addr = ipv4_address;

	/* Return the address.								*/
	FRET (ip_addr);
	}

DLLEXPORT InetT_Address
inet_address_from_ipv4_address_create_invalid_check (IpT_Address ipv4_address)
	{
	InetT_Address		ip_addr;

	/** Same as inet_address_from_ipv4_address_create	**/
	/** except for the fact that if the given address	**/
	/** is IPC_ADDR_INVALID, the address returned will	**/
	/** be INETC_ADDRESS_INVALID.						**/
	
	FIN (inet_address_from_ipv4_address_create_invalid_check (ipv4_address));

	/* Check for IPC_ADDR_INVALID.						*/
	if (ip_address_equal (ipv4_address, IPC_ADDR_INVALID))
		{
		FRET (INETC_ADDRESS_INVALID);
		}

	/* Set the address family to IPv4					*/
	ip_addr.addr_family = InetC_Addr_Family_v4;

	/* Store the actulal address.						*/
	ip_addr.address.ipv4_addr = ipv4_address;

	/* Return the address.								*/
	FRET (ip_addr);
	}

DLLEXPORT void
inet_address_destroy_dynamic (InetT_Address* addr_ptr)
	{
	/** Frees any memory allocated to the contents of	**/
	/** the InetT_Address structure. and the structure	**/
	/** itself. 										**/

	FIN (inet_address_destroy_dynamic (addr_ptr));

	/* Dynamic memory allocation will be used only for	*/
	/* IPv6 addresses.									*/
	if (InetC_Addr_Family_v6 == addr_ptr->addr_family)
		{
		prg_mem_free (addr_ptr->address.ipv6_addr_ptr);
		}

	/* Free the memory allocated to the structure itself*/
	prg_mem_free (addr_ptr);
	
	FOUT;
	}

DLLEXPORT PrgT_Boolean
inet_address_ptr_equal (const InetT_Address* addr1_ptr, const InetT_Address* addr2_ptr)
	{
	PrgT_Boolean			ret_value;

	/** Returns true if the two addresses are equal		**/

	FIN (inet_address_ptr_equal (addr1_ptr, addr2_ptr));

	/* If the addr_family values are not the same, 		*/
	/* return false.									*/
	if (addr1_ptr->addr_family != addr2_ptr->addr_family)
		{
		FRET (PRGC_FALSE);
		}

	/* Address family is the same. Handle appropriately	*/
	/* based on whether it is IPv4 address or an IPv6	*/
	switch (addr1_ptr->addr_family)
		{
		case InetC_Addr_Family_v4:
			ret_value = ip_address_equal (addr1_ptr->address.ipv4_addr, addr2_ptr->address.ipv4_addr);
			break;
		case InetC_Addr_Family_v6:
			ret_value = ipv6_address_equal (addr1_ptr->address.ipv6_addr_ptr, addr2_ptr->address.ipv6_addr_ptr);
			break;
		default:
			/* Both addresses are invalid. Return true	*/
			ret_value = PRGC_TRUE;
			break;
		}

	FRET (ret_value);
	}

DLLEXPORT int
inet_address_ptr_compare_proc (const void* a_ptr, const void* b_ptr)
	{
	InetT_Address* addr1_ptr = PRGC_NIL;
	InetT_Address* addr2_ptr = PRGC_NIL;		
	int ret_value = 0;
	
	/** This is a wrapper function only **/
	FIN (inet_address_ptr_compare_proc (const void* a_ptr, const void* b_ptr));
	
	addr1_ptr = (InetT_Address*) a_ptr;
	addr2_ptr = (InetT_Address*) b_ptr;
	
	ret_value = inet_address_ptr_compare (addr1_ptr, addr2_ptr);
	
	FRET (ret_value);
	}

DLLEXPORT int
inet_address_ptr_compare (const InetT_Address* addr1_ptr, const InetT_Address* addr2_ptr)
	{
	int					ret_value;

	/** Function used to compare two ip addresses		**/
	/** Returns -1, if addr1 is less than addr2			**/
	/** Returns  0, if addr1 is equal to  addr2			**/
	/** Returns  1, if addr1 is greater than addr2		**/

	FIN (inet_address_ptr_compare (addr1_ptr, addr2_ptr));

	/* Check the address families are the same.			*/
	if (addr1_ptr->addr_family == addr2_ptr->addr_family)
		{
		/* Address family is the same. Handle appropriately	*/
		/* based on whether it is IPv4 address or an IPv6	*/
		switch (addr1_ptr->addr_family)
			{
			case InetC_Addr_Family_v4:
				ret_value = ip_address_compare (addr1_ptr->address.ipv4_addr, addr2_ptr->address.ipv4_addr);
				break;
			case InetC_Addr_Family_v6:
				ret_value = ipv6_address_compare (addr1_ptr->address.ipv6_addr_ptr, addr2_ptr->address.ipv6_addr_ptr);
				break;
			default:
				/* Error									*/
				ip_addr_error_abort ("Error in ip_addr_v4 (inet_address_compare)", 
					"One or both of the addresses belong to an invalid address family.");
				break;
			}
		}
	/* The address families are different, compare the	*/
	/* address families themselves.						*/
	else if (addr1_ptr->addr_family < addr2_ptr->addr_family)
		{
		ret_value = -1;
		}
	else /* (addr1_ptr->addr_family > addr2_ptr->addr_family) */
		{
		ret_value = +1;
		}

	FRET (ret_value);
	}

DLLEXPORT PrgT_Boolean
inet_address_ptr_less_than (const InetT_Address* addr1_ptr, const InetT_Address* addr2_ptr)
	{
	PrgT_Boolean				is_less_than;

	/** Function used to compare two ip addresses		**/
	/** Returns true if addr1 is less than addr2.		**/

	FIN (inet_address_ptr_less_than (addr1_ptr, addr2_ptr));

	/* If the address families are not the same we		*/
	/* cannot perform a comparison. Stop the simulation	*/
	if (addr1_ptr->addr_family != addr2_ptr->addr_family)
		{
		ip_addr_error_abort ("Error in ip_addr_v4 (inet_address_ptr_less_than):", 
			"The two addresses do not belong to the same family.");
		}
	
	/* Address family is the same. Handle appropriately	*/
	/* based on whether it is IPv4 address or an IPv6	*/
	switch (addr1_ptr->addr_family)
		{
		case InetC_Addr_Family_v4:
			FRET ((PrgT_Boolean) (addr1_ptr->address.ipv4_addr < addr2_ptr->address.ipv4_addr));
		case InetC_Addr_Family_v6:
			is_less_than = ipv6_address_less_than (addr1_ptr->address.ipv6_addr_ptr, addr2_ptr->address.ipv6_addr_ptr);
			FRET (is_less_than);
		default:
			/* Error									*/
			ip_addr_error_abort ("Error in ip_addr_v4 (inet_address_ptr_less_than):", 
				"The two addresses belong to an invalid address family.");
			break;
		}

	FRET (PRGC_FALSE);
	}

DLLEXPORT PrgT_Boolean
inet_address_ptr_greater_than (const InetT_Address* addr1_ptr, const InetT_Address* addr2_ptr)
	{
	PrgT_Boolean				is_greater_than;

	/** Function used to compare two ip addresses		**/
	/** Returns true if addr1 is less than addr2.		**/

	FIN (inet_address_ptr_greater_than (addr1_ptr, addr2_ptr));

	/* If the address families are not the same we		*/
	/* cannot perform a comparison. Stop the simulation	*/
	if (addr1_ptr->addr_family != addr2_ptr->addr_family)
		{
		ip_addr_error_abort ("Error in ip_addr_v4 (inet_address_ptr_greater_than):",
			"he two addresses do not belong to the same family.");
		}
	
	/* Address family is the same. Handle appropriately	*/
	/* based on whether it is IPv4 address or an IPv6	*/
	switch (addr1_ptr->addr_family)
		{
		case InetC_Addr_Family_v4:
			FRET ((PrgT_Boolean) (addr1_ptr->address.ipv4_addr > addr2_ptr->address.ipv4_addr));
		case InetC_Addr_Family_v6:
			is_greater_than = ipv6_address_greater_than (addr1_ptr->address.ipv6_addr_ptr, addr2_ptr->address.ipv6_addr_ptr);
			FRET (is_greater_than);
		default:
			/* Error									*/
			ip_addr_error_abort ("Error in ip_addr_v4 (inet_address_ptr_greater_than):", 
				"The two addresses belong to an invalid address family.");
			break;
		}

	FRET (PRGC_FALSE);
	}

DLLEXPORT InetT_Address
inet_ipv6_address_ptr_copy (const InetT_Address* addr_ptr)
	{
	InetT_Address		copy_addr;
	
	/** Creates a copy of the specified address.		**/
	
	FIN (inet_ipv6_address_ptr_copy (addr_ptr));

	/* First copy the address family value.				*/
	copy_addr.addr_family = InetC_Addr_Family_v6;

	/* Do a deep copy of the address.					*/
	copy_addr.address.ipv6_addr_ptr = ipv6_address_copy (addr_ptr->address.ipv6_addr_ptr);

	FRET (copy_addr);
	}

DLLEXPORT PrgT_Boolean
inet_address_string_test (const char* ip_addr_str, InetT_Addr_Family addr_family)
	{
	PrgT_Boolean			ret_value;

	/** Returns true if the given string represents a 	**/
	/** valid address of the specified family.			**/

	FIN (inet_address_string_test (ip_addr_str, addr_family));

	/* Call the appropriate function based on the		*/
	/* address family.									*/
	switch (addr_family)
		{
		case InetC_Addr_Family_v4:
			/* Use ip_address_string_test.				*/
			ret_value = ip_address_string_test (ip_addr_str);
			break;
		case InetC_Addr_Family_v6:
			/* Use ipv6_address_string_test.			*/
			ret_value = ipv6_address_string_test (ip_addr_str);
			break;
		case InetC_Addr_Family_Unknown:
			/* Return true if it is either a v4 or a v6	*/
			/* address.									*/
			ret_value = ((PrgT_Boolean) (ip_address_string_test (ip_addr_str) || ipv6_address_string_test (ip_addr_str)));
			break;
		default:
			/* Error									*/
			ip_addr_error_abort ("Error in ip_addr_v4 (inet_address_string_test):", 
				"The address family specified is invalid");
		}

	FRET (ret_value);
	}

DLLEXPORT InetT_Address
inet_address_ptr_complement (const InetT_Address* addr_ptr)
	{
	InetT_Address		complement_addr;

	/** Returns the complement of the given address		**/

	FIN (inet_address_ptr_complement (addr_ptr));

	/* The complement address will belong to the same	*/
	/* family as the original address.					*/
	complement_addr.addr_family = addr_ptr->addr_family;

	/* Call the appropriate function based on the		*/
	/* address family.									*/
	switch (complement_addr.addr_family)
		{
		case InetC_Addr_Family_v4:
			/* Use ip_address_string_test.				*/
			complement_addr.address.ipv4_addr = ip_address_complement (addr_ptr->address.ipv4_addr);
			break;
		case InetC_Addr_Family_v6:
			/* Use ipv6_address_string_test.			*/
			complement_addr.address.ipv6_addr_ptr = ipv6_address_complement (addr_ptr->address.ipv6_addr_ptr);
			break;
		default:
			/* Error									*/
			ip_addr_error_abort ("Error in ip_addr_v4 (inet_address_ptr_complement):", 
				"The address family specified is invalid");
		}

	/* Return the complement address.					*/
	FRET (complement_addr);
	}
	
DLLEXPORT InetT_Address
inet_address_ptr_mask (const InetT_Address* addr_ptr, InetT_Subnet_Mask subnet_mask)
	{
	InetT_Address			masked_addr;
	IpT_Address				ipv4_subnet_mask;

	/** Applies the specified subnet mask to the address**/
	/** Note: The address passed to this function is not**/
	/** modified. Additional memory might be allocated	**/
	/** for the address structure that is returned. 	**/
	/** inet_address_destroy must be called on the		**/
	/** value returned once its use is over.			**/
	
	FIN (inet_address_mask (addr_ptr, subnet_mask));

	/* Copy the address family value.					*/
	masked_addr.addr_family = addr_ptr->addr_family;

	/* Call the appropriate function based on the		*/
	/* address family.									*/
	switch (addr_ptr->addr_family)
		{
		case InetC_Addr_Family_v4:
			/* Create a v4 subnet mask of the specified	*/
			/* length.									*/
			ipv4_subnet_mask = ip_smask_from_inet_smask_create (subnet_mask);
			/* Apply the mask to the given address		*/
			masked_addr.address.ipv4_addr = ip_address_mask (addr_ptr->address.ipv4_addr, ipv4_subnet_mask);
			break;
		case InetC_Addr_Family_v6:
			/* Use ipv6_address_mask					*/
			masked_addr.address.ipv6_addr_ptr = ipv6_address_mask (addr_ptr->address.ipv6_addr_ptr, subnet_mask);
			break;
		default:
			/* Error									*/
			ip_addr_error_abort ("Error in ip_addr_v4 (inet_address_mask):", 
				"The address family specified is invalid");

		}

	/* Return the masked address.						*/
	FRET (masked_addr);
	}

DLLEXPORT char*
inet_address_ptr_print (char* addr_str, const InetT_Address* addr_ptr)
	{
	/** Creates a string representation of the given	**/
	/** address.										**/
	/** This function returns the given string.			**/

	FIN (inet_address_ptr_print (addr_str, addr_ptr));

	/* Call the appropriate function based on the		*/
	/* address family.									*/
	switch (addr_ptr->addr_family)
		{
		case InetC_Addr_Family_v4:
			/* Call ip_address_print					*/
			ip_address_print (addr_str, addr_ptr->address.ipv4_addr);
			break;
		case InetC_Addr_Family_v6:
			/* Use ipv6_address_print					*/
			ipv6_address_print (addr_str, addr_ptr->address.ipv6_addr_ptr);
			break;
		default:
			/* Invalid IP Address.						*/
			strcpy (addr_str, "Invalid IP Address");
			break;
		}

	/* Return the addr_str								*/
	FRET (addr_str);
	}

DLLEXPORT void
inet_address_print_debug (const InetT_Address* address_ptr)
	{
	char				addr_str [INETC_ADDR_STR_LEN];

	/** Prints the given IP address to the console.		*/
	/** Useful for debugging.							*/

	FIN (inet_address_print_debug (address));

	/* Call inet_address_ptr_print to print the address	*/
	inet_address_ptr_print (addr_str, address_ptr);

	/* Print the string to the console.					*/
	printf ("%s\n", addr_str);

	FOUT;
	}

DLLEXPORT void
inet_address_ici_field_print (void* addr_ptr, PrgT_List* field_value_str_lptr)
	{
	char*				addr_str;

	/** This function can be used to print InetT_Address**/
	/** fiels in Icis.									**/

	FIN (inet_address_ici_field_print (addr_ptr, field_value_str_lptr));

	/* Allocate enough memory to hold the address string*/
	addr_str = (char*) prg_mem_alloc (INETC_ADDR_STR_LEN * sizeof (char));

	/* Call inet_address_ptr_print to print the address	*/
	inet_address_ptr_print (addr_str, (InetT_Address*) addr_ptr);

	/* Append a new line character to the string.		*/
	strcat (addr_str, "\n");

	/* Add this string to the list.						*/
	prg_list_insert (field_value_str_lptr, addr_str, PRGC_LISTPOS_TAIL);

	FOUT;
	}

DLLEXPORT PrgT_Boolean
inet_address_ptr_is_multicast (const InetT_Address* addr_ptr)
	{
	PrgT_Boolean			is_multicast;
	
	/** Creates a copy of the specified address.		**/
	
	FIN (inet_address_ptr_is_multicast (addr_ptr));

	/* Call the appropriate function depending on the	*/
	/* address family.									*/
	switch (addr_ptr->addr_family)
		{
		case InetC_Addr_Family_v4:
			/* Use ip_address_is_multicast.				*/
			is_multicast = ip_address_is_multicast (addr_ptr->address.ipv4_addr);
			break;
		case InetC_Addr_Family_v6:
			is_multicast = ipv6_address_is_multicast (addr_ptr->address.ipv6_addr_ptr);
			break;
		default:
			/* Error									*/
			ip_addr_error_abort ("Error in ip_addr_v4 (inet_address_ptr_is_multicast):", 
				"The address family specified is invalid");
			break;
		}

	FRET (is_multicast);
	}

DLLEXPORT InetT_Address
inet_address_ptr_and (const InetT_Address* addr1_ptr, const InetT_Address* addr2_ptr)
	{
	InetT_Address			result;

	/** Returns true if the two addresses are equal		**/

	FIN (inet_address_ptr_and (addr1_ptr, addr2_ptr));

	/* If the addr_family values are not the same, 		*/
	/* return an invalid address.						*/
	if (addr1_ptr->addr_family != addr2_ptr->addr_family)
		{
		FRET (INETC_ADDRESS_INVALID);
		}
	
	/* Set the address family in the result.			*/
	result.addr_family = addr1_ptr->addr_family;

	/* Address family is the same. Handle appropriately	*/
	/* based on whether it is IPv4 address or an IPv6	*/
	switch (addr1_ptr->addr_family)
		{
		case InetC_Addr_Family_v4:
			result.address.ipv4_addr = ip_address_mask (addr1_ptr->address.ipv4_addr, addr2_ptr->address.ipv4_addr);
			break;
		case InetC_Addr_Family_v6:
			result.address.ipv6_addr_ptr = ipv6_address_and (addr1_ptr->address.ipv6_addr_ptr, addr2_ptr->address.ipv6_addr_ptr);
			break;
		default:
			/* Error: Return an Invalid address			*/
			result = INETC_ADDRESS_INVALID;
			break;
		}

	FRET (result);
	}

DLLEXPORT InetT_Address
inet_address_ptr_or (const InetT_Address* addr1_ptr, const InetT_Address* addr2_ptr)
	{
	InetT_Address			result;

	/** Returns true if the two addresses are equal		**/

	FIN (inet_address_ptr_or (addr1_ptr, addr2_ptr));

	/* If the addr_family values are not the same, 		*/
	/* return an invalid address.						*/
	if (addr1_ptr->addr_family != addr2_ptr->addr_family)
		{
		FRET (INETC_ADDRESS_INVALID);
		}
	
	/* Set the address family in the result.			*/
	result.addr_family = addr1_ptr->addr_family;

	/* Address family is the same. Handle appropriately	*/
	/* based on whether it is IPv4 address or an IPv6	*/
	switch (addr1_ptr->addr_family)
		{
		case InetC_Addr_Family_v4:
			result.address.ipv4_addr = addr1_ptr->address.ipv4_addr | addr2_ptr->address.ipv4_addr;
			break;
		case InetC_Addr_Family_v6:
			result.address.ipv6_addr_ptr = ipv6_address_or (addr1_ptr->address.ipv6_addr_ptr, addr2_ptr->address.ipv6_addr_ptr);
			break;
		default:
			/* Error: Return an Invalid address			*/
			result = INETC_ADDRESS_INVALID;
			break;
		}

	FRET (result);
	}

DLLEXPORT InetT_Address
inet_ipv4_compat_addr_from_ipv4_addr_create (const InetT_Address ipv4_addr)
	{
	InetT_Address			ipv4_compat_addr;

	/** Create an IPv4 compatible IPv6 address from the given	**/
	/** IPv4 address. The first 96 bits of an IPv4 compatible 	**/
	/** will be zeros and the remaining 32 bits will be the IPv4**/
	/** address.												**/

	FIN (inet_ipv4_compat_addr_from_ipv4_addr_create (ipv4_addr));

	/* Set the address family of the return address correctly.	*/
	ipv4_compat_addr.addr_family = InetC_Addr_Family_v6;

	/* Allocate enough memory to hold the IPv6 address.			*/
	ipv4_compat_addr.address.ipv6_addr_ptr = ipv6_address_mem_alloc ();

	/* Set the first 96 bits to zero.							*/
	ipv4_compat_addr.address.ipv6_addr_ptr->addr32[0] = 0;
	ipv4_compat_addr.address.ipv6_addr_ptr->addr32[1] = 0;
	ipv4_compat_addr.address.ipv6_addr_ptr->addr32[2] = 0;

	/* Store the IPv4 address in the remaining 32 bits.			*/
	ipv4_compat_addr.address.ipv6_addr_ptr->addr32[3] = ipv4_addr.address.ipv4_addr;

	/* Return the newly created address.						*/
	FRET (ipv4_compat_addr);
	}
	
DLLEXPORT InetT_Address
inet_ipv4_mapped_addr_from_ipv4_addr_create (const InetT_Address ipv4_addr)
	{
	InetT_Address			ipv4_compat_addr;

	/** Create an IPv4 mapped IPv6 address from the given		**/
	/** IPv4 address. The first 80 bits of an IPv4 compatible 	**/
	/** will be zeros, next 16 are all 1s, and the remaining 32 **/
	/**	bits will be the IPv4 address.							**/

	FIN (inet_ipv4_mapped_addr_from_ipv4_addr_create (ipv4_addr));

	/* Set the address family of the return address correctly.	*/
	ipv4_compat_addr.addr_family = InetC_Addr_Family_v6;

	/* Allocate enough memory to hold the IPv6 address.			*/
	ipv4_compat_addr.address.ipv6_addr_ptr = ipv6_address_mem_alloc ();

	/* Set the first 96 bits to zero.							*/
	ipv4_compat_addr.address.ipv6_addr_ptr->addr32[0] = 0;
	ipv4_compat_addr.address.ipv6_addr_ptr->addr32[1] = 0;
	ipv4_compat_addr.address.ipv6_addr_ptr->addr32[2] = 0xFFFF;

	/* Store the IPv4 address in the remaining 32 bits.			*/
	ipv4_compat_addr.address.ipv6_addr_ptr->addr32[3] = ipv4_addr.address.ipv4_addr;

	/* Return the newly created address.						*/
	FRET (ipv4_compat_addr);
	}

DLLEXPORT InetT_Address
inet_ipv4_addr_from_ipv4_compat_addr_get (const InetT_Address* ipv4_compat_addr_ptr)
	{
	InetT_Address		ipv4_addr;

	/** Obtain the IPv4 address from the given IPv4 compatible	**/
	/** IPv6 address											**/
	/** An IPv4 compatible address will have the first 96 bits	**/
	/** set to 0 and the last 32 bits set to the IPv4 address	**/

	FIN (inet_ipv4_addr_from_ipv4_compat_addr_get (ipv4_compat_addr_ptr));

	/* Set the address family correctly in the address.			*/
	ipv4_addr.addr_family = InetC_Addr_Family_v4;

	/* Set the address 											*/
	ipv4_addr.address.ipv4_addr = (IpT_Address) (ipv4_compat_addr_ptr->address.ipv6_addr_ptr->addr32[3]);

	FRET (ipv4_addr);
	}

DLLEXPORT InetT_Address
inet_ipv4_addr_from_ipv4_mapped_addr_get (const InetT_Address* ipv4_mapped_addr_ptr)
	{
	InetT_Address		ipv4_addr;

	/** Obtain the IPv4 address from the given IPv4 mapped		**/
	/** IPv6 address											**/
	/** An IPv4 mapped address will have the first 80 bits		**/
	/** set to 0, next 16 set to 1 and the last 32 bits set to 	**/
	/** the IPv4 address.										**/

	FIN (inet_ipv4_addr_from_ipv4_mapped_addr_get (ipv4_mapped_addr_ptr));

	/* Set the address family correctly in the address.			*/
	ipv4_addr.addr_family = InetC_Addr_Family_v4;

	/* Set the address 											*/
	ipv4_addr.address.ipv4_addr = (IpT_Address) (ipv4_mapped_addr_ptr->address.ipv6_addr_ptr->addr32[3]);

	FRET (ipv4_addr);
	}


DLLEXPORT InetT_Address
inet_ipv4_addr_from_6to4_addr_get (const InetT_Address* sixtofour_addr_ptr)
	{
	InetT_Address		ipv4_addr;
	OpT_uInt16			msb, lsb;

	/** Obtain the IPv4 address from the given 6to4 address		**/
	/** The IPv4 address will be stored in bits 17-48 of the	**/
	/** address.												**/

	FIN (inet_ipv4_addr_from_6to4_addr_get (ipv4_compat_addr_ptr));

	/* Set the address family correctly in the address.			*/
	ipv4_addr.addr_family = InetC_Addr_Family_v4;

	/* The more significant 16 bits of the IPv4 address are		*/
	/* stored as the less significant bits in the first 32 bit	*/
	/* component of the 6to4 address.							*/
	msb = sixtofour_addr_ptr->address.ipv6_addr_ptr->addr32[0] & 0xffff;

	/* The less significant 16 bits of the IPv4 address are		*/
	/* stored as the more significant bits in the second 32 bit	*/
	/* component of the 6to4 address.							*/
	lsb = sixtofour_addr_ptr->address.ipv6_addr_ptr->addr32[1] >> 16;

	/* Create the IPv4 address from the msb and lsb.			*/
	ipv4_addr.address.ipv4_addr = (((IpT_Address) msb) << 16) | ((IpT_Address) lsb);

	FRET (ipv4_addr);
	}

DLLEXPORT PrgT_Boolean
inet_address_is_6to4 (const InetT_Address* addr_ptr)
	{
	/** Checks whether a given address is a 6to4 address.		**/

	FIN (inet_address_is_6to4 (addr_ptr));

	/* 6to4 addresses have their first 16 bits set to 0x2002.	*/
	FRET ((PrgT_Boolean) ((InetC_Addr_Family_v6 == inet_address_family_get (addr_ptr)) &&
					 (0x2002 == (addr_ptr->address.ipv6_addr_ptr->addr32[0] >> 16))));
	}

DLLEXPORT PrgT_Boolean
inet_address_is_ipv4_compat (const InetT_Address* addr_ptr)
    {
    /** Checks whether a given address is an IPv4 compatible    **/
    /** IPv6 address.                                           **/

    FIN (inet_address_is_ipv4_compat (addr_ptr));

    /* The first 96 bits of an IPv4 compatible address will be 0*/
    /* RFC 2893 Sec. 5.1.                                       */
    FRET ((PrgT_Boolean) ((InetC_Addr_Family_v6 == inet_address_family_get (addr_ptr)) &&
                     (0 == (addr_ptr->address.ipv6_addr_ptr->addr32[0])) &&
                     (0 == (addr_ptr->address.ipv6_addr_ptr->addr32[1])) &&
                     (0 == (addr_ptr->address.ipv6_addr_ptr->addr32[2]))))
    }

DLLEXPORT PrgT_Boolean
inet_address_is_ipv4_mapped (const InetT_Address* addr_ptr)
    {
    /** Checks whether a given address is an IPv4 mapped	    **/
    /** IPv6 address.                                           **/

    FIN (inet_address_is_ipv4_mapped (addr_ptr));

	/* First 80 bits are zero. Next 16 bits are all 1s.			*/	
    FRET ((PrgT_Boolean) ((InetC_Addr_Family_v6 == inet_address_family_get (addr_ptr)) &&
                     (0 == (addr_ptr->address.ipv6_addr_ptr->addr32[0])) &&
                     (0 == (addr_ptr->address.ipv6_addr_ptr->addr32[1])) &&
                     (0xFFFF == (addr_ptr->address.ipv6_addr_ptr->addr32[2]))))
    }


DLLEXPORT void*
inet_address_addr_ptr_get (InetT_Address* inet_addr_ptr)
	{
	/** Returns a pointer to the raw address. Useful for the	**/
	/** OMS Patricia tree package.								**/

	FIN (inet_address_addr_ptr_get (addr_ptr));

	/* Copy the actual address based on the version				*/
	switch (inet_addr_ptr->addr_family)
		{
		case InetC_Addr_Family_v4:
			/* Return a pointer to the IPv4 address.			*/
			FRET (&(inet_addr_ptr->address.ipv4_addr));
		case InetC_Addr_Family_v6:
			/* Return a pointer to the IPv6 address.			*/
			FRET (inet_addr_ptr->address.ipv6_addr_ptr);
		default:
			/* Error.											*/
			break;
		}

	FRET (PRGC_NIL);
	}
	
DLLEXPORT const void*
inet_address_addr_ptr_get_const (const InetT_Address* inet_addr_ptr)
	{
	/** Returns a pointer to the raw address. Useful for the	**/
	/** OMS Patricia tree package.								**/

	FIN (inet_address_addr_ptr_get_const (addr_ptr));

	/* Copy the actual address based on the version				*/
	switch (inet_addr_ptr->addr_family)
		{
		case InetC_Addr_Family_v4:
			/* Return a pointer to the IPv4 address.			*/
			FRET (&(inet_addr_ptr->address.ipv4_addr));
		case InetC_Addr_Family_v6:
			/* Return a pointer to the IPv6 address.			*/
			FRET (inet_addr_ptr->address.ipv6_addr_ptr);
		default:
			/* Error.											*/
			break;
		}
	FRET (PRGC_NIL);
	}
	
DLLEXPORT InetT_Subnet_Mask
inet_default_smask_for_addr_ptr_create (const InetT_Address* addr_ptr)
	{
	IpT_Address_Class		address_class;
	InetT_Subnet_Mask		snet_mask;

	/** Returns the default subnet mask for the given address	**/
	/** applicable to IPv4 addresses only.						**/

	FIN (inet_default_smask_for_addr_ptr_create (addr_ptr));

	/* If the address is 0.0.0.0, then the mask is also 0.	*/
	if (inet_address_equal (*addr_ptr, InetI_Default_v4_Addr))
		snet_mask = (InetT_Subnet_Mask) 0;

	else
		{
		/*	Determine the address class of the IP address for which	*/
		/*	the subnet mask needs to be created.					*/
		address_class = ip_address_class (addr_ptr->address.ipv4_addr);

		/* Based on the class, generate the default subnet mask. 	*/
		snet_mask = inet_default_smask_from_class (address_class);
		}

	FRET (snet_mask);
	}

DLLEXPORT InetT_Subnet_Mask
inet_default_smask_from_class (IpT_Address_Class addr_class)
	{
	InetT_Subnet_Mask		snet_mask;

	/** Returns the default subnet mask for the specified	**/
	/** class in InetT_Subnet_Mask format.					**/

	FIN (inet_default_smask_from_class (addr_class));

	switch (addr_class)
		{
		case IPC_ADDRESS_CLASS_A:
			{
			snet_mask = (InetT_Subnet_Mask) 8;
			break;
			}

		case IPC_ADDRESS_CLASS_B:
			{
			snet_mask = (InetT_Subnet_Mask) 16;
			break;
			}

		case IPC_ADDRESS_CLASS_C:
			{
			snet_mask = (InetT_Subnet_Mask) 24;
			break;
			}
		/* Class E is reserved (starting with 240.0.0.0).	*/
		/* At least one user has configured this address.	*/
		/* Consider these host addresses (/32).				*/	
		case IPC_ADDRESS_CLASS_D:
		case IPC_ADDRESS_CLASS_E:
			{
			snet_mask = (InetT_Subnet_Mask) 32;
			break;
			}

		default:
			{
			ip_addr_error_abort ("Error in ip_addr_v4 (inet_default_smask_from_class):", 
				"The address class specified is invalid.");
			break;
			}
		}

	FRET (snet_mask);
	}

DLLEXPORT InetT_Address_Range
inet_address_range_create (InetT_Address addr, InetT_Subnet_Mask subnet_mask)
	{
	InetT_Address_Range		address_range;

	/** Creates an address range from an address and	**/
	/** subnet mask.									**/
	
	FIN (inet_address_range_create (addr, subnet_mask));

	/* Set the address family correctly.				*/
	address_range.addr_family = addr.addr_family;

	/* Set the subnet mask.								*/
	address_range.subnet_mask = subnet_mask;

	/* Copy the actual address based on the version		*/
	switch (address_range.addr_family)
		{
		case InetC_Addr_Family_v4:
			/* Copy the IPv4 address					*/
			address_range.address.ipv4_addr = addr.address.ipv4_addr;
			break;
		case InetC_Addr_Family_v6:
			/* Create a copy of the IPv6 address.		*/
			address_range.address.ipv6_addr_ptr = ipv6_address_copy (addr.address.ipv6_addr_ptr);
			break;
		default:
			/* Error.									*/
			ip_addr_error_abort ("Error in ip_addr_v4 (inet_address_range_create):", 
				"The address family specified is invalid");
		}

	/* Return the address range created.				*/
	FRET (address_range);
	}

DLLEXPORT InetT_Address_Range		
inet_address_range_str_parse (const char* prefix_str, InetT_Addr_Family addr_family)
	{
	char				prefix_str_copy [256];
	char*				prefix_len_ptr;
	InetT_Address		address;
	InetT_Address_Range	addr_range;
	PrgT_Boolean				error;
	int					prefix_length;

	/** Parse a string of the format <addr>/<length>	**/
	/** If the length field is left out, the length is	**/
	/** assumed to be 32 for IPv4 addresses and 128 for	**/
	/** IPv6 addresses.									**/

	FIN (inet_address_range_str_parse (prefix_str, addr_family));

	/* Create a local copy of the string.				*/
	strcpy (prefix_str_copy, prefix_str);

	/* Look for a / in the string.						*/
	prefix_len_ptr = strchr (prefix_str_copy, '/');

	/* If a / character was found, replace it with a	*/
	/* character so that the original string now points	*/
	/* to the address part alone.						*/
	if (NULL != prefix_len_ptr)
		{
		*prefix_len_ptr = '\0';

		/* Increment the prefix_len_ptr by 1 so that it	*/
		/* now points to the prefix length itself.		*/
		++prefix_len_ptr;
		}

	/* Create an InetT_Address representation of the	*/
	/* address.											*/
	address = inet_address_create (prefix_str_copy, addr_family);

	/* The address family of the address range is going	*/
	/* to be same as the address fmaily of the address.	*/
	/* Note we cannot use the addr_family argument		*/
	/* because it might be set to unknown.				*/
	addr_range.addr_family = address.addr_family;

	/* Check the address family of the address.			*/
	switch (address.addr_family)
		{
		case InetC_Addr_Family_v4:
			/* Store the address.						*/
			addr_range.address.ipv4_addr = address.address.ipv4_addr;

			/* Determine the prefix length.				*/
			if (NULL != prefix_len_ptr)
				{
				/* Use the specified prefix length.		*/
				prefix_length = oms_string_to_int (prefix_len_ptr, 0, 32, &error);
				}
			else
				{
				/* The prefix length was not specifed.	*/
				/* Use a length of 32.					*/
				prefix_length = 32;

				/* Reset the error flag.				*/
				error = PRGC_FALSE;
				}
			break;

		case InetC_Addr_Family_v6:
			/* Store the address.						*/
			addr_range.address.ipv6_addr_ptr = address.address.ipv6_addr_ptr;

			/* Determine the prefix length.				*/
			if (NULL != prefix_len_ptr)
				{
				/* Use the specified prefix length.		*/
				prefix_length = oms_string_to_int (prefix_len_ptr, 0, 128, &error);
				}
			else
				{
				/* The prefix length was not specifed.	*/
				/* Use a length of 128.					*/
				prefix_length = 128;

				/* Reset the error flag.				*/
				error = PRGC_FALSE;
				}
			break;

		default:
			/* The address was specified incorrectly	*/
			/* Return an invalid address range.			*/
			addr_range = INETC_ADDR_RANGE_INVALID;
			break;
		}

	/* Check for errors.								*/
	if (error)
		{
		inet_address_range_destroy (&addr_range);
		addr_range = INETC_ADDR_RANGE_INVALID;
		}
	else
		{
		/* Set the subnet mask field in the address range*/
		addr_range.subnet_mask = inet_smask_from_length_create (prefix_length);
		}

	/* Return address range.							*/
	FRET (addr_range);
	}

DLLEXPORT InetT_Address_Range
inet_address_range_create_fast (InetT_Address addr, InetT_Subnet_Mask subnet_mask)
	{
	InetT_Address_Range		address_range;

	/** Similar to inet_address_range_create except		**/
	/** that for IPv6 addresses the memory is not		**/
	/** duplicated.										**/
	
	FIN (inet_address_range_create_fast (addr, subnet_mask));

	/* Set the address family correctly.				*/
	address_range.addr_family = addr.addr_family;

	/* Set the subnet mask.								*/
	address_range.subnet_mask = subnet_mask;

	/* Copy the actual address based on the version		*/
	switch (address_range.addr_family)
		{
		case InetC_Addr_Family_v4:
			/* Copy the IPv4 address					*/
			address_range.address.ipv4_addr = addr.address.ipv4_addr;
			break;
		case InetC_Addr_Family_v6:
			/* Do not duplicate the memory used to hold	*/
			/* the IPv6 address.						*/
			address_range.address.ipv6_addr_ptr = addr.address.ipv6_addr_ptr;
			break;
		default:
			/* Error.									*/
			ip_addr_error_abort ("Error in ip_addr_v4 (inet_address_range_create_fast):", 
				"The address family specified is invalid");
		}

	/* Return the address range created.				*/
	FRET (address_range);
	}

DLLEXPORT InetT_Address_Range
inet_address_range_network_create (InetT_Address addr, InetT_Subnet_Mask subnet_mask)
	{
	InetT_Address_Range		address_range;

	/* Similar to inet_address_range_create except that	*/
	/* the address is masked before storing.			*/
	
	FIN (inet_address_range_network_create (addr, subnet_mask));

	/* Set the address family correctly.				*/
	address_range.addr_family = addr.addr_family;

	/* Set the subnet mask.								*/
	address_range.subnet_mask = subnet_mask;

	/* Copy the actual address based on the version		*/
	switch (address_range.addr_family)
		{
		case InetC_Addr_Family_v4:
			/* Copy the IPv4 address					*/
			address_range.address.ipv4_addr = ip_address_mask (addr.address.ipv4_addr,
				ip_smask_from_inet_smask_create (subnet_mask));
			break;
		case InetC_Addr_Family_v6:
			/* Create a copy of the IPv6 address.		*/
			address_range.address.ipv6_addr_ptr = ipv6_address_mask (addr.address.ipv6_addr_ptr, subnet_mask);
			break;
		default:
			/* Error.									*/
			ip_addr_error_abort ("Error in ip_addr_v4 (inet_address_range_network_create):", 
				"The address family specified is invalid");
		}

	/* Return the address range created.				*/
	FRET (address_range);
	}

DLLEXPORT InetT_Address_Range
inet_address_range_network_from_addr_range_create (InetT_Address_Range* addr_range_ptr)
	{
	InetT_Address_Range		address_range;

	/** Creates a new address range whose address is	**/
	/** set to the network address of the given address	**/
	/** range and whose subnet mask is set to that of	**/
	/** given address range itself.						**/

	FIN (inet_address_range_network_from_addr_range_create (addr_range_ptr));

	/* Set the address family correctly.				*/
	address_range.addr_family = addr_range_ptr->addr_family;

	/* Set the subnet mask.								*/
	address_range.subnet_mask = addr_range_ptr->subnet_mask;

	/* Copy the actual address based on the version		*/
	switch (address_range.addr_family)
		{
		case InetC_Addr_Family_v4:
			/* Copy the IPv4 address					*/
			address_range.address.ipv4_addr = ip_address_mask (addr_range_ptr->address.ipv4_addr,
				ip_smask_from_inet_smask_create (address_range.subnet_mask));
			break;
		case InetC_Addr_Family_v6:
			/* Create a copy of the IPv6 address.		*/
			address_range.address.ipv6_addr_ptr =
				ipv6_address_mask (addr_range_ptr->address.ipv6_addr_ptr, address_range.subnet_mask);
			break;
		default:
			/* Error.									*/
			ip_addr_error_abort ("Error in ip_addr_v4 (inet_address_range_network_from_addr_range_create):", 
				"The address family specified is invalid");

		}

	/* Return the address range created.				*/
	FRET (address_range);
	}

DLLEXPORT InetT_Address_Range
inet_ipv6_address_range_from_intf_id_create (InetT_Address addr, unsigned int intf_id, InetT_Subnet_Mask subnet_mask)
	{
	InetT_Address_Range			address_range;

	/** Create an IPv6 address range from the specified	**/
	/** prefix address, interface ID and subnet mask.	**/

	FIN (inet_ipv6_address_range_from_intf_id_create (addr, intf_id,  subnet_mask));

	/* Set the address family of the address range to IPv6*/
	address_range.addr_family = InetC_Addr_Family_v6;

	/* Set the subnet mask to the specified value.		*/
	address_range.subnet_mask = subnet_mask;

	/* Create an IPv6 address from the given network	*/
	/* address and subnet mask.							*/
	address_range.address.ipv6_addr_ptr = ipv6_address_mem_alloc ();

	/* Set the first 64 bits of the address to the		*/
	/* corresponding bits in the given address and the	*/
	/* remaining 64 bits to the interface ID.			*/
	address_range.address.ipv6_addr_ptr->addr32[0] = addr.address.ipv6_addr_ptr->addr32[0];
	address_range.address.ipv6_addr_ptr->addr32[1] = addr.address.ipv6_addr_ptr->addr32[1];
	address_range.address.ipv6_addr_ptr->addr32[2] = 0;
	address_range.address.ipv6_addr_ptr->addr32[3] = intf_id;

	FRET (address_range);
	}

DLLEXPORT InetT_Address_Range	
inet_address_range_ptr_copy (const InetT_Address_Range* addr_range_ptr)
	{
	InetT_Address_Range		copy_addr_range;

	/** Returns a copy of the given address range.		**/

	FIN (inet_address_range_ptr_copy (addr_range_ptr));
	
	/* First copy the address family value and mask.	*/
	copy_addr_range.addr_family = addr_range_ptr->addr_family;
	copy_addr_range.subnet_mask = addr_range_ptr->subnet_mask;

	/* Copy the rest of the structure based on the value*/
	/* of the address family							*/
	switch (addr_range_ptr->addr_family)
		{
		case InetC_Addr_Family_v4:
			/* Copy the value of the IPv4 address.		*/
			copy_addr_range.address.ipv4_addr = ip_address_copy (addr_range_ptr->address.ipv4_addr);
			break;
		case InetC_Addr_Family_v6:
			copy_addr_range.address.ipv6_addr_ptr = ipv6_address_copy (addr_range_ptr->address.ipv6_addr_ptr);
			break;
		default:
			/* No need to do anything.					*/
			break;
		}

	FRET (copy_addr_range);
	}

DLLEXPORT void
inet_address_range_destroy (InetT_Address_Range* addr_range_ptr)
	{
	/** Frees any memory allocated to the contents of	**/
	/** the InetT_Address_Range structure. Note that	**/
	/** does not free the memory occupied by the		**/
	/** structure itself. If this memory was allocated	**/
	/** dynamically, it must be freed separately. This	**/
	/** also means that calls like 						**/
	/** inet_address_range_destroy (&addr_range)		**/
	/** where addr_range is of type InetT_Address_Range	**/
	/** are valid.										**/

	FIN (inet_address_range_destroy (addr_range_ptr));

	/* Dynamic memory allocation will be used only for	*/
	/* IPv6 addresses.									*/
	if (InetC_Addr_Family_v6 == addr_range_ptr->addr_family)
		{
		prg_mem_free (addr_range_ptr->address.ipv6_addr_ptr);
		}
	
	FOUT;
	}

DLLEXPORT PrgT_Boolean
inet_address_range_check (InetT_Address addr, const InetT_Address_Range* addr_range_ptr)
	{
	IpT_Address				ipv4_subnet_mask;
	PrgT_Boolean					ret_value;

	/** Checks whether a particular address falls in	**/
	/** an address range or not.						**/

	FIN (inet_address_range_check (addr_range_ptr, addr_ptr));

	/* First make sure that the address and the address	*/
	/* range are of the same type.						*/
	if (addr_range_ptr->addr_family != addr.addr_family)
		{
		/* Return False.								*/
		FRET (PRGC_FALSE);
		}

	/* Call the appropriate function based on the 		*/
	/* address family.									*/
	switch (addr_range_ptr->addr_family)
		{
		case InetC_Addr_Family_v4:
			/* Create an IpT_Address representation of	*/
			/* the subnet mask.							*/
			ipv4_subnet_mask = ip_smask_from_inet_smask_create (addr_range_ptr->subnet_mask);
			/* Apply the mask to both the specified		*/
			/* address and the address in the address	*/
			/* range and compare the results.			*/
			ret_value = ip_address_equal (ip_address_mask (addr.address.ipv4_addr, ipv4_subnet_mask),
										  ip_address_mask (addr_range_ptr->address.ipv4_addr, ipv4_subnet_mask));
			break;
		case InetC_Addr_Family_v6:
			/* The above approach would work for IPv6	*/
			/* addresses also. But it would be 			*/
			/* inefficient. Use another approach.		*/
			/* Compare the first 'subnet mask' bits of	*/
			/* the two addresses.						*/
			ret_value = ipv6_address_equal_until_length (addr_range_ptr->address.ipv6_addr_ptr,
							addr.address.ipv6_addr_ptr, inet_smask_length_count (addr_range_ptr->subnet_mask));
			break;
		default:
			/* Error.									*/
			ip_addr_error_abort ("Error in ip_addr_v4 (inet_address_range_check):", 
				"The address family specified is invalid");
		}

	/* Return the computed result.						*/
	FRET (ret_value);
	}

DLLEXPORT PrgT_Boolean
inet_address_range_address_equal (const InetT_Address_Range* addr_range_ptr, const InetT_Address* addr_ptr)
	{
	PrgT_Boolean			ret_value;
	/** Checks whether the two address ranges are equal	**/

	FIN (inet_address_range_address_equal (addr_range1_ptr, addr_range2_ptr));

	/* If the addr_family or subnet mask values are not	*/
	/* the same, return false.							*/
	if (addr_range_ptr->addr_family != addr_ptr->addr_family)
		{
		FRET (PRGC_FALSE);
		}

	/* Address family is the same. Handle appropriately	*/
	/* based on whether it is IPv4 address or an IPv6	*/
	switch (addr_range_ptr->addr_family)
		{
		case InetC_Addr_Family_v4:
			ret_value = ip_address_equal (addr_range_ptr->address.ipv4_addr, addr_ptr->address.ipv4_addr);
			break;
		case InetC_Addr_Family_v6:
			ret_value = ipv6_address_equal (addr_range_ptr->address.ipv6_addr_ptr, addr_ptr->address.ipv6_addr_ptr);
			break;
		default:
			/* Error: Return false						*/
			ret_value = PRGC_FALSE;
			break;
		}

	FRET (ret_value);
	}

DLLEXPORT PrgT_Boolean
inet_address_range_network_address_equal (const InetT_Address_Range* addr_range_ptr, const InetT_Address* ntwk_addr_ptr)
	{
	PrgT_Boolean			ret_value;
	/** Checks whether the two address ranges are equal	**/

	FIN (inet_address_range_network_address_equal (addr_range1_ptr, addr_range2_ptr));

	/* If the addr_family or subnet mask values are not	*/
	/* the same, return false.							*/
	if (addr_range_ptr->addr_family != ntwk_addr_ptr->addr_family)
		{
		FRET (PRGC_FALSE);
		}

	/* Address family is the same. Handle appropriately	*/
	/* based on whether it is IPv4 address or an IPv6	*/
	switch (addr_range_ptr->addr_family)
		{
		case InetC_Addr_Family_v4:
			/* Apply the mask to the address of the		*/
			/* address range and compare the result with*/
			/* the given network address.				*/
			ret_value = ip_address_equal
				(ip_address_mask (addr_range_ptr->address.ipv4_addr, ip_smask_from_inet_smask_create (addr_range_ptr->subnet_mask)),
				 ntwk_addr_ptr->address.ipv4_addr);
			FRET (ret_value);
		case InetC_Addr_Family_v6:
			/* Make sure the first mask_length bits of	*/
			/* the address are the same as that of the	*/
			/* network address.							*/
			ret_value = ipv6_address_equal_until_length (addr_range_ptr->address.ipv6_addr_ptr,
				ntwk_addr_ptr->address.ipv6_addr_ptr, inet_smask_length_count (addr_range_ptr->subnet_mask));
			FRET (ret_value);
		default:
			/* Error: Return false						*/
			ret_value = PRGC_FALSE;
			break;
		}

	FRET (ret_value);
	}

DLLEXPORT PrgT_Boolean
inet_address_range_equal (const InetT_Address_Range* addr_range1_ptr, const InetT_Address_Range* addr_range2_ptr)
	{
	PrgT_Boolean			ret_value;
	/** Checks whether the two address ranges are equal	**/

	FIN (inet_address_range_equal (addr_range1_ptr, addr_range2_ptr));

	/* If the addr_family or subnet mask values are not	*/
	/* the same, return false.							*/
	if ((addr_range1_ptr->addr_family != addr_range2_ptr->addr_family) ||
		(addr_range1_ptr->subnet_mask != addr_range2_ptr->subnet_mask))
		{
		FRET (PRGC_FALSE);
		}

	/* Address family is the same. Handle appropriately	*/
	/* based on whether it is IPv4 address or an IPv6	*/
	switch (addr_range1_ptr->addr_family)
		{
		case InetC_Addr_Family_v4:
			ret_value = ip_address_equal (addr_range1_ptr->address.ipv4_addr, addr_range2_ptr->address.ipv4_addr);
			break;
		case InetC_Addr_Family_v6:
			ret_value = ipv6_address_equal (addr_range1_ptr->address.ipv6_addr_ptr, addr_range2_ptr->address.ipv6_addr_ptr);
			break;
		default:
			/* Error: Return false						*/
			ret_value = PRGC_FALSE;
			break;
		}

	FRET (ret_value);
	}

DLLEXPORT InetT_Address
inet_address_range_addr_get (const InetT_Address_Range* addr_range_ptr)
	{
	InetT_Address			address;

	/** Return the address part of the address range.	**/
	
	FIN (inet_address_range_addr_get (addr_range_ptr));

	/* Copy the address family.							*/
	address.addr_family = addr_range_ptr->addr_family;

	/* Copy the address also.							*/
	switch (addr_range_ptr->addr_family)
		{
		case InetC_Addr_Family_v4:
			address.address.ipv4_addr = ip_address_copy (addr_range_ptr->address.ipv4_addr);
			break;
		case InetC_Addr_Family_v6:
			address.address.ipv6_addr_ptr = ipv6_address_copy (addr_range_ptr->address.ipv6_addr_ptr);
			break;
		default:
			/* Error: Return false						*/
			ip_addr_error_abort ("Error in ip_addr_v4 (inet_address_range_addr_get):", 
				"The address family specified is invalid");
			break;
		}
	/* Return the address.								*/
	FRET (address);
	}

DLLEXPORT void*
inet_address_range_addr_ptr_get (InetT_Address_Range* addr_range_ptr)
	{
	/** Returns a pointer to the raw address. Useful for the	**/
	/** OMS Patricia tree package.								**/

	FIN (inet_address_range_addr_ptr_get (addr_range_ptr));

	/* Copy the actual address based on the version				*/
	switch (addr_range_ptr->addr_family)
		{
		case InetC_Addr_Family_v4:
			/* Return a pointer to the IPv4 address.			*/
			FRET (&(addr_range_ptr->address.ipv4_addr));
		case InetC_Addr_Family_v6:
			/* Return a pointer to the IPv6 address.			*/
			FRET (addr_range_ptr->address.ipv6_addr_ptr);
		default:
			/* Error.											*/
			break;
		}

	FRET (PRGC_NIL);
	}
	
DLLEXPORT InetT_Address
inet_address_range_addr_get_fast (const InetT_Address_Range* addr_range_ptr)
	{
	InetT_Address			address;

	/** Similar to inet_address_range_addr_get except	**/
	/** for the fact that for IPv6 addresses the address**/
	/** is not copied.									**/
	FIN (inet_address_range_addr_get_fast (addr_range_ptr));

	/* Copy the address family.							*/
	address.addr_family = addr_range_ptr->addr_family;

	/* Copy the address also.							*/
	switch (addr_range_ptr->addr_family)
		{
		case InetC_Addr_Family_v4:
			address.address.ipv4_addr = addr_range_ptr->address.ipv4_addr;
			FRET (address);
		case InetC_Addr_Family_v6:
			address.address.ipv6_addr_ptr = addr_range_ptr->address.ipv6_addr_ptr;
			FRET (address);
		default:
			/* Error: Return false						*/
			ip_addr_error_abort ("Error in ip_addr_v4 (inet_address_range_addr_get_fast):", 
				"The address family specified is invalid");
			break;
		}
	/* Return the address.								*/
	FRET (INETC_ADDRESS_INVALID);
	}

DLLEXPORT InetT_Address
inet_address_range_broadcast_addr_get (const InetT_Address_Range* addr_range_ptr)
	{
	InetT_Address			address;

	/** Returns the broadcast address of the specified	**/
	/** address range. Applicable to only IPv4 addresses**/

	FIN (inet_address_range_broadcast_addr_get (addr_range_ptr));

	/* Make sure the address family is IPv4.			*/
	if (InetC_Addr_Family_v4 != addr_range_ptr->addr_family)
		{
		/* An IPv6 broadcast is defined as a link layer	*/
		/* multicast on all interfaces.					*/
		address = inet_address_copy (InetI_Ipv6_All_Nodes_LL_Mcast_Addr);
		}
	else
		{
		/* To get the broadcast address do a logical OR of	*/
		/* the address and the reverse subnet_mask			*/
		address.addr_family = InetC_Addr_Family_v4;
		address.address.ipv4_addr = (addr_range_ptr->address.ipv4_addr) | (((IpT_Address) 0xffffffff) >> (addr_range_ptr->subnet_mask));
		}

	/* Return the address.								*/
	FRET (address);
	}

DLLEXPORT InetT_Address
inet_address_range_network_addr_get (const InetT_Address_Range* addr_range_ptr)
	{
	InetT_Address			address;

	/** Returns the network address of the specified	**/
	/** address range.									**/

	FIN (inet_address_range_network_addr_get (addr_range_ptr));

	switch (addr_range_ptr->addr_family)
		{
		case InetC_Addr_Family_v4:
			/* To get the address, apply the subnet mask*/
			/* to the interface address.				*/
			address.addr_family = InetC_Addr_Family_v4;
			address.address.ipv4_addr = ip_address_mask (addr_range_ptr->address.ipv4_addr,
				ip_smask_from_inet_smask_create (addr_range_ptr->subnet_mask));
			break;
		case InetC_Addr_Family_v6:
			/* To get the address, apply the subnet mask*/
			/* to the interface address.				*/
			address.addr_family = InetC_Addr_Family_v6;
			address.address.ipv6_addr_ptr = ipv6_address_mask (addr_range_ptr->address.ipv6_addr_ptr, addr_range_ptr->subnet_mask);
			break;
		default:
			ip_addr_error_abort ("Error in ip_addr_v4 (inet_address_range_network_addr_get):", 
				"The address family specified is invalid");
		}

	/* Return the address.								*/
	FRET (address);
	}

DLLEXPORT PrgT_Boolean
inet_address_range_addr_is_broadcast (const InetT_Address_Range* addr_range_ptr, const InetT_Address* addr_ptr)
	{
	PrgT_Boolean			is_broadcast;

	/** Checks whether the given address is the broadcast	**/
	/** address for the specified address range.			**/
	/** Note that this function assumes that the given		**/
	/** address falls in the address range.					**/
	
	FIN (inet_address_range_addr_is_broadcast (addr_range_ptr, addr_ptr));

	/* First make sure that we are looking at IPv4 addresses*/
	if ((InetC_Addr_Family_v4 != addr_range_ptr->addr_family) ||
		(InetC_Addr_Family_v4 != addr_ptr->addr_family))
		{
		is_broadcast = PRGC_FALSE;
		}
	else
		{
		is_broadcast = ip_address_is_broadcast (addr_ptr->address.ipv4_addr,
				ip_smask_from_inet_smask_create (addr_range_ptr->subnet_mask));
		}

	FRET (is_broadcast);
	}

DLLEXPORT char*
inet_address_range_print (char* addr_range_str, const InetT_Address_Range* addr_range_ptr)
	{
	char				mask_str [8];

	/** Creates a string representation of the given	**/
	/** address.										**/
	/** This function returns the given string.			**/

	FIN (inet_address_range_print (addr_range_str, addr_ptr));

	/* Call the appropriate function based on the		*/
	/* address family.									*/
	switch (addr_range_ptr->addr_family)
		{
		case InetC_Addr_Family_v4:
			/* Call ip_address_print					*/
			ip_address_print (addr_range_str, addr_range_ptr->address.ipv4_addr);
			break;
		case InetC_Addr_Family_v6:
			/* Use ipv6_address_print					*/
			ipv6_address_print (addr_range_str, addr_range_ptr->address.ipv6_addr_ptr);
			break;
		default:
			/* Error									*/
			ip_addr_error_abort ("Error in ip_addr_v4 (inet_address_range_print):", 
				"The address family specified is invalid");
		}

	/* Create a mask string.							*/
	sprintf (mask_str, "/%d", addr_range_ptr->subnet_mask);

	/* Append the mask string to the address_string		*/
	strcat (addr_range_str, mask_str);

	/* Return the addr_str								*/
	FRET (addr_range_str);
	}

DLLEXPORT char*
inet_address_range_address_print (char* addr_str, const InetT_Address_Range* addr_range_ptr)
	{
	/** Creates a string representation of the given	**/
	/** address.										**/
	/** This function returns the given string.			**/

	FIN (inet_address_range_address_print (addr_str, addr_range_ptr));

	/* Call the appropriate function based on the		*/
	/* address family.									*/
	switch (addr_range_ptr->addr_family)
		{
		case InetC_Addr_Family_v4:
			/* Call ip_address_print					*/
			ip_address_print (addr_str, addr_range_ptr->address.ipv4_addr);
			break;
		case InetC_Addr_Family_v6:
			/* Use ipv6_address_print					*/
			ipv6_address_print (addr_str, addr_range_ptr->address.ipv6_addr_ptr);
			break;
		default:
			/* Error									*/
			ip_addr_error_abort ("Error in ip_addr_v4 (inet_address_range_address_print):", 
				"The address family specified is invalid");
		}

	/* Return the addr_str								*/
	FRET (addr_str);
	}

DLLEXPORT InetT_Address_Range	
inet_ipv4_address_range_create (IpT_Address addr, IpT_Address subnet_mask)
	{
	InetT_Address_Range		addr_range;

	/** Create an address range from the given IPv4		**/
	/** address and mask.								**/

	FIN (inet_ipv4_address_range_create (addr, subnet_mask));

	addr_range.addr_family	= InetC_Addr_Family_v4;
	addr_range.address.ipv4_addr	= addr;
	addr_range.subnet_mask	= inet_smask_from_ipv4_smask_create (subnet_mask);

	FRET (addr_range);
	}

DLLEXPORT InetT_Address_Range	
inet_ipv4_address_range_network_create (IpT_Address addr, IpT_Address subnet_mask)
	{
	InetT_Address_Range		addr_range;

	/** Create an address range from the given IPv4		**/
	/** address and mask.								**/

	FIN (inet_ipv4_address_range_network_create (addr, subnet_mask));

	addr_range.addr_family	= InetC_Addr_Family_v4;
	addr_range.address.ipv4_addr	= ip_address_mask (addr, subnet_mask);
	addr_range.subnet_mask	= inet_smask_from_ipv4_smask_create (subnet_mask);

	FRET (addr_range);
	}

DLLEXPORT IpT_Address_Range
inet_ipv4_address_range_get (const InetT_Address_Range* addr_range_ptr)
	{
	IpT_Address_Range		ipv4_addr_range;

	/** Convert the address range into IpT_Address_Range format	**/
	FIN (inet_ipv4_address_range_get (addr_range_ptr));

	ipv4_addr_range.address = addr_range_ptr->address.ipv4_addr;
	ipv4_addr_range.subnet_mask = ip_smask_from_inet_smask_create (addr_range_ptr->subnet_mask);

	FRET (ipv4_addr_range);
	}

DLLEXPORT void
inet_address_range_ici_field_print (void* addr_range_ptr, PrgT_List* field_value_str_lptr)
	{
	char*				addr_str;

	/** This function can be used to print InetT_Address**/
	/** fiels in Icis.									**/
	FIN (inet_address_range_ici_field_print (addr_range_ptr, field_value_str_lptr));

	/* Allocate enough memory to hold the address string*/
	addr_str = (char*) prg_mem_alloc (INETC_ADDR_STR_LEN * sizeof (char));

	/* Call inet_address_ptr_print to print the address	*/
	inet_address_range_print (addr_str, (InetT_Address_Range*) addr_range_ptr);

	/* Add this string to the list.						*/
	prg_list_insert (field_value_str_lptr, addr_str, PRGC_LISTPOS_TAIL);

	FOUT;
	}


DLLEXPORT int
inet_address_range_compare (const InetT_Address_Range* addr_range1_ptr,
	const InetT_Address_Range* addr_range2_ptr)
	{
	int					ret_value;

	/** Function used to compare two ip addresses		**/
	/** Returns -1, if addr1 is less than addr2			**/
	/** Returns  0, if addr1 is equal to  addr2			**/
	/** Returns  1, if addr1 is greater than addr2		**/

	FIN (inet_address_range_compare (addr_range1_ptr, addr_range2_ptr));

	/* Check the address families are the same.			*/
	if (addr_range1_ptr->addr_family == addr_range2_ptr->addr_family)
		{
		/* Address family is the same. Handle appropriately	*/
		/* based on whether it is IPv4 address or an IPv6	*/
		switch (addr_range1_ptr->addr_family)
			{
			case InetC_Addr_Family_v4:
				ret_value = ip_address_compare (addr_range1_ptr->address.ipv4_addr, addr_range2_ptr->address.ipv4_addr);
				break;
			case InetC_Addr_Family_v6:
				ret_value = ipv6_address_compare (addr_range1_ptr->address.ipv6_addr_ptr, addr_range2_ptr->address.ipv6_addr_ptr);
				break;
			default:
				/* Error									*/
				ip_addr_error_abort ("Error in ip_addr_v4 (inet_address_range_compare):", 
				"The address family specified is invalid");
				break;
			}

		/* If the addresses are the same, compare the	*/
		/* subnet masks.								*/
		if (0 == ret_value)
			{
			if (addr_range1_ptr->subnet_mask < addr_range2_ptr->subnet_mask)
				{
				ret_value = -1;
				}
			else if (addr_range1_ptr->subnet_mask > addr_range2_ptr->subnet_mask)
				{
				ret_value = 1;
				}
			/* else the subnet masks are also equal leave the	*/
			/* return value as 0.								*/
			}
		}
	/* The address families are different, compare the	*/
	/* address families themselves.						*/
	else if (addr_range1_ptr->addr_family < addr_range2_ptr->addr_family)
		{
		ret_value = -1;
		}
	else /* (addr_range1_ptr->addr_family > addr_range2_ptr->addr_family) */
		{
		ret_value = +1;
		}

	FRET (ret_value);
	}

DLLEXPORT InetT_Address_Range*
inet_address_range_mem_alloc (void)
	{
	InetT_Address_Range*		addr_range_ptr;

	/** Allocates memory for an InetT_Address_Range structure.	**/
	
	FIN (inet_address_range_mem_alloc (void));

	/* Allocate memory from the pool memory object.			*/
	addr_range_ptr = (InetT_Address_Range*) prg_pmo_alloc (InetI_Addr_Range_Pmh);

	FRET (addr_range_ptr);
	}

DLLEXPORT IpT_Address
ip_smask_create (const char *smask_str)
	{
	IpT_Address 		subnet_mask = IPC_ADDR_INVALID;
	PrgT_Boolean 			error;
	InetT_Subnet_Mask	smask_length;
		
	/* This function creates a subnet mask from the subnet		*/
	/* mask string. The string can be either a prefix length 	*/
	/* or can be a regular subnet address format				*/
	FIN (ip_smask_create (const char *smask_str));
	
	/* Check if the mask is in subnet address notation			*/
	if (ip_address_string_test (smask_str))
		subnet_mask = ip_address_create (smask_str);
	else
		{
		/* The mask is in prefix length notation				*/
		smask_length = (InetT_Subnet_Mask) oms_string_to_int(smask_str, 0, 32, &error);
		
		/* Check if the prefix length is valid, i.e. if it is	*/
		/* in between 0 and 32. If not then consider it to be	*/
		/* 32, else get subnet mask from valid prefix length	*/
		if (error)
			subnet_mask = IpI_Broadcast_Addr;
		else
			subnet_mask = ip_smask_from_length_create (smask_length);
		}
		
	FRET (subnet_mask);
	}

DLLEXPORT IpT_Address
ip_address_node_broadcast_create (IpT_Address address, IpT_Address subnet_mask)
	{
	IpT_Address			broadcast_addr;

	/** Create the subnet level broadcast address corresponding	**/
	/** to the given address and mask.							**/

	FIN (ip_address_node_broadcast_create (address, subnet_mask));

	/* /31 subnet masks require special handling. Since			*/
	/* broadcasting in not really possible in such subnets,		*/
	/* return the other address in the subnet. The other		*/
	/* address can be easily constructed by flipping the last	*/
	/* bit in the IP address.									*/
	if (ip_address_equal (subnet_mask, ip_smask_from_length_create (31)))
		{
		/* Flip the last bit in the address and return the		*/
		/* result.												*/
		broadcast_addr = address ^ ((IpT_Address) 0x1);
		}
	else
		{
		/* Regular subnet mask. Set all the host bits of the	*/
		/* address to 1 by ORing the address with the			*/
		/* complement of the subnet mask.						*/
		broadcast_addr = ip_address_complement_create (subnet_mask) | address;
		}

	/* Return the broadcast address that we created.			*/
	FRET (broadcast_addr);
	}

DLLEXPORT InetT_Address_Hash_Table*
inet_addr_hash_table_create (int ipv4_htable_size, int ipv6_htable_size)
	{
	InetT_Address_Hash_Table*	hash_table_ptr;

	/** Create an Inet Address hash table structure of the		**/
	/** specified size.											**/

	FIN (inet_addr_hash_table_create (ipv4_htable_size, ipv6_htable_size));

	/* Allocate enough memory for the hash table.				*/
	hash_table_ptr = (InetT_Address_Hash_Table*) prg_cmo_alloc
		(InetI_Addr_Htable_Cmh, sizeof (InetT_Address_Hash_Table));

	/* If the IPv4 hash table size is non-zero, create the IPv4	*/
	/* hash table.												*/
	if (ipv4_htable_size > 0)
		{
		hash_table_ptr->ipv4_hash_table = prg_bin_hash_table_create
			((int) ceil (log ((double) ipv4_htable_size)/M_LN2), sizeof (IpT_Address));
		}
	else
		{
		/* There won't be any IPv4 entries.						*/
		hash_table_ptr->ipv4_hash_table = PRGC_NIL;
		}

	/* If the IPv6 hash table size is non-zero, create the IPv4	*/
	/* hash table.												*/
	if (ipv6_htable_size > 0)
		{
		hash_table_ptr->ipv6_hash_table = prg_bin_hash_table_create
			((int) ceil (log ((double) ipv6_htable_size)/M_LN2), sizeof (Ipv6T_Address));
		}
	else
		{
		/* There won't be any IPv4 entries.						*/
		hash_table_ptr->ipv6_hash_table = PRGC_NIL;
		}

	FRET (hash_table_ptr);
	}

DLLEXPORT void
inet_addr_hash_table_destroy (InetT_Address_Hash_Table* hash_table_ptr,
	InetT_Address_Hash_Elem_Dealloc_Proc dealloc_proc)
	{
	/** Free the memory allocated to an Inet Address hash table	**/
	/** and its contents.										**/

	FIN (inet_addr_hash_table_destroy (hash_table_ptr, dealloc_proc));

	/* If the IPv4 hash table is not NIL, destroy it.			*/
	if (hash_table_ptr->ipv4_hash_table != PRGC_NIL)
		{
		prg_bin_hash_table_destroy (hash_table_ptr->ipv4_hash_table, dealloc_proc);
		}

	/* If the IPv6 hash table is not NIL, destroy it.			*/
	if (hash_table_ptr->ipv6_hash_table != PRGC_NIL)
		{
		prg_bin_hash_table_destroy (hash_table_ptr->ipv6_hash_table, dealloc_proc);
		}

	/* Free the memory allocated to the Inet Address hash table	*/
	/* itself.													*/
	prg_mem_free (hash_table_ptr);

	FOUT;
	}

DLLEXPORT void*
inet_addr_hash_table_item_get (InetT_Address_Hash_Table* hash_table_ptr,
	const InetT_Address* key)
	{
	void*					value_ptr;

	/** Lookup the entry corresponding to the specified address.**/

	FIN (inet_addr_hash_table_item_get (hash_table_ptr, key));

	/* Pick the appropriate hash table based on the address		*/
	/* family.													*/
	switch (inet_address_family_get (key))
		{
		case InetC_Addr_Family_v4:
			/* Make sure that the IPv4 hash table exists.		*/
			if (PRGC_NIL == hash_table_ptr->ipv4_hash_table)
				{
				value_ptr = PRGC_NIL;
				}
			else
				{
				/* Lookup the entry in the IPv4 hash table.		*/
				value_ptr = prg_bin_hash_table_item_get (hash_table_ptr->ipv4_hash_table, &(key->address.ipv4_addr));
				}
			break;

		case InetC_Addr_Family_v6:
			/* Make sure that the IPv6 hash table exists.		*/
			if (PRGC_NIL == hash_table_ptr->ipv6_hash_table)
				{
				value_ptr = PRGC_NIL;
				}
			else
				{
				/* Lookup the entry in the IPv6 hash table.		*/
				value_ptr = prg_bin_hash_table_item_get (hash_table_ptr->ipv6_hash_table, (key->address.ipv6_addr_ptr));
				}
			break;

		default:
			/* Invalid address. Return NIL.						*/
			value_ptr = PRGC_NIL;
		}

	/* Return the value corresponding to key.					*/
	FRET (value_ptr);
	}

DLLEXPORT void
inet_addr_hash_table_item_insert (InetT_Address_Hash_Table* hash_table_ptr,
	const InetT_Address* key, void* value_ptr, void ** old_value_ptr)
	{
	/** Insert an entry into an Inet Address hash table.		**/

	FIN (inet_addr_hash_table_item_insert (hash_table_ptr, key, value_ptr, old_value_ptr));

	/* Pick the appropriate hash table based on the address		*/
	/* family.													*/
	switch (inet_address_family_get (key))
		{
		case InetC_Addr_Family_v4:
			/* Make sure that the IPv4 hash table exists.		*/
			if (PRGC_NIL == hash_table_ptr->ipv4_hash_table)
				{
				/* Generate a recoverable error.				*/
				ip_addr_error_recover ("Error in inet_addr_hash_table_item_insert",
					"IPv4 hash table was not created");
				}
			else
				{
				/* Insert the entry into the IPv4 hash table.	*/
				prg_bin_hash_table_item_insert (hash_table_ptr->ipv4_hash_table,
					&(key->address.ipv4_addr), value_ptr, old_value_ptr);
				}
			break;

		case InetC_Addr_Family_v6:
			/* Make sure that the IPv6 hash table exists.		*/
			if (PRGC_NIL == hash_table_ptr->ipv6_hash_table)
				{
				/* Generate a recoverable error.				*/
				ip_addr_error_recover ("Error in inet_addr_hash_table_item_insert",
					"IPv6 hash table was not created");
				}
			else
				{
				/* Insert the entry into the IPv6 hash table.	*/
				prg_bin_hash_table_item_insert (hash_table_ptr->ipv6_hash_table,
					(key->address.ipv6_addr_ptr), value_ptr, old_value_ptr);
				}
			break;

		default:
			/* Invalid address. Generate an RE.					*/
			ip_addr_error_recover ("Error in inet_addr_hash_table_item_insert",
				"The specified address is invalid");
		}

	/* Return.	*/
	FOUT;
	}

DLLEXPORT void*
inet_addr_hash_table_item_remove (InetT_Address_Hash_Table* hash_table_ptr,
	const InetT_Address * key)
	{
	void*		value_ptr;

	/** Remove an element from an Inet Address hash table.		**/

	FIN (inet_addr_hash_table_item_remove (hash_table_ptr, key));

	/* Pick the appropriate hash table based on the address		*/
	/* family.													*/
	switch (inet_address_family_get (key))
		{
		case InetC_Addr_Family_v4:
			/* Make sure that the IPv4 hash table exists.		*/
			if (PRGC_NIL == hash_table_ptr->ipv4_hash_table)
				{
				value_ptr = PRGC_NIL;
				}
			else
				{
				/* Remove the entry from the IPv4 hash table.	*/
				value_ptr = prg_bin_hash_table_item_remove (hash_table_ptr->ipv4_hash_table, &(key->address.ipv4_addr));
				}
			break;

		case InetC_Addr_Family_v6:
			/* Make sure that the IPv6 hash table exists.		*/
			if (PRGC_NIL == hash_table_ptr->ipv6_hash_table)
				{
				value_ptr = PRGC_NIL;
				}
			else
				{
				/* Remove the entry from the IPv6 hash table.	*/
				value_ptr = prg_bin_hash_table_item_remove (hash_table_ptr->ipv6_hash_table, (key->address.ipv6_addr_ptr));
				}
			break;

		default:
			/* Invalid address. Return NIL.						*/
			value_ptr = PRGC_NIL;
		}

	/* Return the value corresponding to key.					*/
	FRET (value_ptr);
	}


DLLEXPORT PrgT_List*
inet_addr_hash_table_item_list_get (InetT_Address_Hash_Table* hash_table_ptr, InetT_Addr_Family key_addr_family)
	{
	PrgT_List*		item_lptr;

	/** Provide the list of the items in the Hash table of		**/
	/** requested address family.								**/
	
	FIN (inet_addr_hash_table_item_list_get (hash_table_ptr, key_addr_family));

	/* Pick the appropriate hash table based on the address		*/
	/* family.													*/
	switch (key_addr_family)
		{
		case InetC_Addr_Family_v4:
			/* Make sure that the IPv4 hash table exists.		*/
			if (PRGC_NIL == hash_table_ptr->ipv4_hash_table)
				{
				item_lptr = PRGC_NIL;
				}
			else
				{
				/* Remove the entry from the IPv4 hash table.	*/
				item_lptr = prg_bin_hash_table_item_list_get (hash_table_ptr->ipv4_hash_table);
				}
			break;

		case InetC_Addr_Family_v6:
			/* Make sure that the IPv6 hash table exists.		*/
			if (PRGC_NIL == hash_table_ptr->ipv6_hash_table)
				{
				item_lptr = PRGC_NIL;
				}
			else
				{
				/* Remove the entry from the IPv6 hash table.	*/
				item_lptr = prg_bin_hash_table_item_list_get (hash_table_ptr->ipv6_hash_table);
				}
			break;

		default:
			/* Invalid address. Return NIL.						*/
			item_lptr = PRGC_NIL;
		}

	/* Return the value corresponding to key.					*/
	FRET (item_lptr);
	}

DLLEXPORT PrgT_IP_Address
inet_ipv4_addr_range_to_prg_ip_address (InetT_Address_Range* addr_range_ptr)
	{
	PrgT_IP_Address				prg_addr;
	IpT_Address					v4_addr, v4_mask;
	
	FIN (inet_ipv4_addr_range_to_prg_ip_address ());
	
	v4_addr = inet_ipv4_address_get (inet_address_range_addr_get_fast (addr_range_ptr));
	v4_mask = ip_smask_from_length_create (inet_address_range_mask_get (addr_range_ptr));
	
	/* Initialize the prg_addr									*/
	prg_ip_address_parse ("Auto Assigned", OPC_NIL, &prg_addr);					
	prg_addr = prg_ip_address_value_set (prg_addr, v4_addr);
	prg_addr = prg_ip_address_mask_set (prg_addr, v4_mask);

	/* The address type is set to host always so that the prg_ip_address package	*/
	/* does not apply masks automatically. The Ip_Aa package (the only client of 	*/
	/* this function) has its own logic to handle subnets, and it does not rely	on	*/
	/* the address type present in the prg_ip_address structure. Set the type		*/
	/* after setting the value, since the prg_ip_address package tries to adjust	*/
	/* the type based on the value and mask.										*/
	prg_addr = prg_ip_address_type_set (prg_addr, PrgC_IP_Address_Type_Host);

	FRET (prg_addr);
	}
	
DLLEXPORT InetT_Address_Range
inet_ipv4_addr_range_from_prg_ip_address (PrgT_IP_Address* prg_addr_ptr)
	{
	InetT_Address_Range			addr_range;
	PrgT_IP_Address_Type		type;
	
	FIN (inet_ipv4_addr_range_to_prg_ip_address ());

	/* We must handle the special cases where the prg address is neither of type	*/
	/* host nor of type subnet.														*/
	type = prg_ip_address_type_get (*prg_addr_ptr);
	
	if (PrgC_IP_Address_Type_Auto == type)
		{
		addr_range = inet_address_range_copy (INETC_ADDR_RANGE_INVALID);
		}
	else if ((PrgC_IP_Address_Type_No_IP_Address == type) && (PrgC_IP_Address_Type_Unnumbered == type))
		{
		addr_range = inet_address_range_create (InetI_No_Ipv4_Address, 0);
		}
	else
		{
		addr_range = inet_ipv4_address_range_create (prg_ip_address_value_get (*prg_addr_ptr),
			prg_ip_address_mask_get (*prg_addr_ptr));
		}

	FRET (addr_range);
	}

DLLEXPORT PrgT_Compcode
inet_address_subnet_next (InetT_Address_Range* addr_range_ptr)
	{
	PrgT_Compcode		status;
	PrgT_IP_Address		prg_addr;
	int					prefix_len, int_to_modify;	
	IpT_Address			smask_to_use;
	
	
	/********************************************************************/
	/* Purpose: 	Returns the next subnet from the given class 		*/
	/*																	*/
	/* Inputs:		Pointer to address range							*/
	/*																	*/
	/* Outputs:		Input arg is modified (incremented)					*/
	/*				A success or failure code is also returned			*/
	/* 																	*/
	/********************************************************************/
	FIN (inet_address_subnet_next ());

	switch (addr_range_ptr->addr_family)
		{
		case InetC_Addr_Family_v4: 
			prg_addr = inet_ipv4_addr_range_to_prg_ip_address (addr_range_ptr);
			status = prg_ip_address_subnet_next (&prg_addr);

			if (PrgC_Compcode_Success == status)
				{
				inet_address_range_destroy (addr_range_ptr);
				*addr_range_ptr = inet_ipv4_addr_range_from_prg_ip_address (&prg_addr);
				}
			break;

		case InetC_Addr_Family_v6:
			/* We are optimistic.	*/
			status = PrgC_Compcode_Success;
			
			/* First determine the integer that needs to be modified.	*/
			prefix_len = inet_smask_length_count (inet_address_range_mask_get (addr_range_ptr));
			
			if (prefix_len % 32 == 0)
				{
				/* The network portion is at an integer boundary. All	*/
				/* we need to do is increment the appropriate integer.	*/
				/* If the mask is 32, we increment first int; if the 	*/
				/* mask is 64, we increment the second int, and so on.	*/
				int_to_modify = (prefix_len / 32) - 1;
				addr_range_ptr->address.ipv6_addr_ptr->addr32 [int_to_modify]++;
				}
			else
				{
				/* If the mask is 31, we must increment the first integer.	*/
				/* but if the mask is 33, we must increment the second 		*/
				/* integer.													*/
				int_to_modify = (prefix_len) / 32;
				
				/* Treat this integer as an IP address and increment to the	*/
				/* next subnet, based on the appropriate mask. The mask	is	*/
				/* the remainder when divided by 32. For example, if the	*/
				/* actual mask length is 48, we must apply a mask of 16 to	*/
				/* the second int. If the mask is 71, we must apply a mask	*/
				/* of 7 to the third int.									*/
				smask_to_use = ip_smask_from_length_create (prefix_len % 32);				

				/* Initialize the prg_addr									*/
				prg_addr = prg_ip_address_type_set (prg_addr, PrgC_IP_Address_Type_Subnet);
				prg_addr = prg_ip_address_mask_set (prg_addr, smask_to_use);

				if(addr_range_ptr->address.ipv6_addr_ptr->addr32 [int_to_modify] != 0)
					{
					prg_addr = prg_ip_address_value_set (prg_addr,
						addr_range_ptr->address.ipv6_addr_ptr->addr32 [int_to_modify]);
					status = prg_ip_address_subnet_next (&prg_addr);
					}
				else
					{
					/* If the int to be incremented has a value of 0, which is equivalent to the
					 * IP address of 0.0.0.0, this is interpreted by the prg_ip_address API as the
					 * special value of "Auto Assigned". This creates a problem since this special
					 * case also forces the subnet mask to "Auto Assigned" as well, which is a hard-
					 * coded value of 255.255.0.0, so the correct subnet mask will not be applied
					 * for the increment.  Therefore we must perform our own arithmetic.  Since the
					 * value is always 0, we do not need any bounds checking, and only need to simply
					 * set the correct bit in the 32 bit address to 1.
					 */
					prg_addr = prg_ip_address_value_set (prg_addr,
						(unsigned int)pow(2.0, (32 - (prefix_len % 32))));
					status = PrgC_Compcode_Success;
					}				
				
				/* If we are successful in our endeavor, all we need to do is 	*/
				/* to set this modified int back in the address.				*/
				if (PrgC_Compcode_Success == status)
					{
					addr_range_ptr->address.ipv6_addr_ptr->addr32 [int_to_modify] = prg_ip_address_value_get (prg_addr);
					}
				else
					{
					/* We were unable to get the next subnet using this int.	*/
					/* Move to the next higher subnet using the more 			*/
					/* significant ints in the v6 address.						*/
					if (int_to_modify == 0)
						{
						/* No more significant ints to increment.	*/
						status = PrgC_Compcode_Failure;
						}
					else
						{
						addr_range_ptr->address.ipv6_addr_ptr->addr32 [int_to_modify] = 0;
						int_to_modify--;
						addr_range_ptr->address.ipv6_addr_ptr->addr32 [int_to_modify]++;
						}
					}
				}

			/* Overflow checking.	*/
			while (addr_range_ptr->address.ipv6_addr_ptr->addr32 [int_to_modify] == 0)
				{
				if (int_to_modify == 0)
					{
					/* The most significant int has failed. Cannot find the	*/
					/* next subnet.											*/
					status =  PrgC_Compcode_Failure;
					break;
					}
				else
					{
					int_to_modify--;
					addr_range_ptr->address.ipv6_addr_ptr->addr32 [int_to_modify]++;
					}
				}

			break;
		
		default:
			status = PrgC_Compcode_Failure;
		}

	FRET (status);
	}

DLLEXPORT PrgT_Boolean
inet_addr_str_is_valid_v4 (const char* addr_str)
	{
	unsigned int			ipv4_address;
	
	/********************************************************************/
	/* Purpose:		Determine whether string is a valid IPv4 address.	*/
	/*																	*/
	/* Inputs:		string to check										*/
	/*																	*/
	/* Outputs:		PRGC_TRUE or PRGC_FALSE								*/
	/********************************************************************/
	FIN (inet_addr_str_is_valid_v4 ());

	FRET (inet_addr_str_parse_v4 (addr_str, &ipv4_address) == PrgC_Compcode_Success);
	}
	
	
DLLEXPORT PrgT_Boolean
inet_addr_str_is_valid_v6 (const char* addr_str, InetT_Ipv6_Addr_Type addr_type)
	{
	unsigned int*			ipv6_addr_ptr;
	int						addr_len;
	
	/********************************************************************/
	/* Purpose:		Determine whether string is a valid IPv6 address.	*/
	/*																	*/
	/* Inputs:		string to check										*/
	/*																	*/
	/* Outputs:		PRGC_TRUE or PRGC_FALSE								*/
	/********************************************************************/
	FIN (inet_addr_str_is_valid_v6 ());

	addr_len = (InetC_Ipv6_Addr_Type_EUI_64 == addr_type) ? 64 : 128;
	
	if (inet_addr_str_parse_v6 (addr_str, addr_len, &ipv6_addr_ptr) == PrgC_Compcode_Success)
		{
		prg_mem_free (ipv6_addr_ptr);
		FRET (PRGC_TRUE);
		}
	else
		{
		FRET (PRGC_FALSE);
		}
	}

DLLEXPORT InetT_Address_Range
inet_ipv6_address_range_parse (const char* addr_str, int prefix_len, InetT_Ipv6_Addr_Type type)
	{
	InetT_Address_Range	addr_range;
	InetT_Address		addr;

	/********************************************************************/
	/* Purpose: 	Parse an IP v6 address string and return value in	*/
	/*				the form of a InetT_Address_Range					*/
	/*																	*/
	/* Inputs:		string to check										*/
	/*				Type of address (EUI-64?) to determine addr_len		*/
	/*				mask length (to place in InetT_Address_Range struct	*/
	/*																	*/
	/* Outputs:		v6 address in InetT_Address_Range format			*/
	/*																	*/
	/********************************************************************/	
	FIN (inet_ipv6_address_range_parse ());
	
	if (InetC_Ipv6_Addr_Type_Non_EUI_64 == type)
		{
		addr = inet_address_create (addr_str, InetC_Addr_Family_v6);
		}
	else
		{
		/* This is an EUI-64 address. Since we do not have an intf id,	*/
		/* we will use a value of zero. This will essentially return	*/
		/* the subnet address, which is suitable for the client (at 	*/
		/* this time, the only client of this package is the ETS file	*/
		/* Ip_Auto_Addr_Support.ets.c.									*/
		addr = inet_address_create_from_intf_id (addr_str, 0);
		}
	
	if (inet_address_valid (addr))
		{
		addr_range = inet_address_range_create_fast (addr, inet_smask_from_length_create (prefix_len));
		}
	else
		addr_range = inet_address_range_copy (INETC_ADDR_RANGE_INVALID);
	
	FRET (addr_range);
	}


DLLEXPORT InetT_Address_Range
inet_ipv4_address_range_parse (const char* addr_str, const char* mask_str)
	{
	InetT_Address_Range		addr_range;
	PrgT_IP_Address			prg_addr;
	
	/********************************************************************/
	/* Purpose: 	Creates an address structure from IPv4 address and	*/
	/*				mask strings.										*/
	/*																	*/
	/* Inputs:		Address string and mask string (v4 only)			*/
	/*																	*/
	/* Output:		InetT_Address_Range									*/
	/*																	*/
	/********************************************************************/	
	FIN (inet_ipv4_address_range_parse ());
	
	/* The prg_ip_address package can handle special values for address	*/
	/* and mask (No IP Address, Class Based...).						*/
	prg_ip_address_parse (addr_str, mask_str, &prg_addr);
	
	addr_range = inet_ipv4_addr_range_from_prg_ip_address (&prg_addr);
	
	FRET (addr_range);
	}

DLLEXPORT InetT_Address_Range 
inet_address_range_parse (const char* cidr_range_str)
	{
	char	addr_range_str [128];
	int		mask_length = -1;
	char*	mask_start;
	InetT_Addr_Family	addr_family;
	InetT_Address		addr;
	InetT_Address_Range	addr_range;
	
	FIN (inet_address_range_parse ());

	/* Make local copy so that it can be modified.	*/
	strcpy (addr_range_str, cidr_range_str);

	/* If colons are present in the address, it is IPv6.	*/
	addr_family = (strchr (addr_range_str, ':') != OPC_NIL) ? InetC_Addr_Family_v6 : InetC_Addr_Family_v4;

	mask_start = strchr (addr_range_str, '/');

	/* If only address is specified, a host mask is assumed.	*/
	if (mask_start == OPC_NIL)
		mask_length = (addr_family == InetC_Addr_Family_v4) ? 32 : 128;
	else
		{
		*mask_start = '\0';
		mask_start++;

		mask_length = atoi (mask_start);
		}

	/* Convert address portion of the string into internal representation.	*/
	addr = inet_address_create (addr_range_str, addr_family);
	
	addr_range = inet_address_range_create_fast (addr, inet_smask_from_length_create (mask_length));

	FRET (addr_range);
	}
	
static void
inet_ipv6_address_mask (const unsigned int* orig_addr_ptr, unsigned int* masked_addr_ptr, int prefix_length)
	{
	int 			i, num32bit_fields;
	/********************************************************************/
	/* Purpose: 	Masks an IPv6 address based on the given mask 		*/
	/*				length.												*/
	/*																	*/				
	/* Inputs:		Pointer to orig address (4 int array)				*/
	/* 				mask length											*/
	/*																	*/
	/* Outputs:		Pointer to masked address 							*/
	/*   			(4 int array - output arg)							*/
	/* 				(Memory must be allocated by calling function)		*/
	/*																	*/
	/********************************************************************/
	FIN (inet_ipv6_address_mask ());

	/* If the mask length is exactly 128, just copy the		*/
	/* given address.										*/
	if (128 == prefix_length)
		{
		/* Cannot use prg_mem_copy because it does not take const arg.	*/
		memcpy (masked_addr_ptr, orig_addr_ptr, IPC_V6_ADDR_LEN_BYTES);
		FOUT; 
		}

	/* Find out the number of full 32 fields in the mask	*/
	num32bit_fields = prefix_length >> 5;

	/* Copy the full fields over.							*/
	for (i = 0; i < num32bit_fields; i++)
		{
		masked_addr_ptr [i] = orig_addr_ptr [i];
		}

	/* Mask the next field with a mask of length			*/
	/* prefix_length % 32 i.e. (prefix_length & 0x1F).			*/
	masked_addr_ptr [num32bit_fields] = orig_addr_ptr [num32bit_fields]
		& ip_smask_from_length_create (prefix_length & 0x1F);

	/* Set the remaining fields to 0.						*/
	for (i = num32bit_fields + 1; i < IPC_V6_ADDR_LEN_INTS; i++)
		{
		masked_addr_ptr [i] = 0;
		}
	FOUT;
	}

DLLEXPORT PrgT_Boolean
inet_address_subnets_overlap (const InetT_Address_Range* addr_range_ptr1, const InetT_Address_Range* addr_range_ptr2)
	{
	PrgT_Boolean			overlap;
	InetT_Address			addr1, addr2, masked_addr1, masked_addr2;
	InetT_Subnet_Mask		mask1, mask2, less_specific_mask;
	/********************************************************************/
	/* Purpose: 	Determine if two subnets have any common addresses,	*/
	/*				based on the mask lengths present inside the 		*/
	/*				InetT_Address_Range structure						*/
	/*																	*/
	/* Inputs:		pointers to two InetT_Address_Range					*/
	/*																	*/
	/* Outputs:		PRGC_TRUE or PRGC_FALSE								*/
	/*																	*/
	/********************************************************************/
	FIN (inet_address_subnets_overlap ());

	/* Subnets can overlap only if they belong to the same family.	*/
	if (addr_range_ptr1->addr_family != addr_range_ptr2->addr_family)
		FRET (PRGC_FALSE);

	if ((addr_range_ptr2->addr_family != InetC_Addr_Family_v4) && (addr_range_ptr2->addr_family != InetC_Addr_Family_v6))
		FRET (PRGC_FALSE);

	/* Mask both addresses with the less specific mask and see if the 	*/
	/* resulting networks are same. This is the test for overlap.		*/
	mask1 = inet_address_range_mask_get (addr_range_ptr1);
	mask2 = inet_address_range_mask_get (addr_range_ptr2);
	
 	less_specific_mask = (inet_smask_length_count (mask1) < inet_smask_length_count (mask2)) ? mask1 : mask2;
			
	addr1 = inet_address_range_addr_get_fast (addr_range_ptr1);
	addr2 = inet_address_range_addr_get_fast (addr_range_ptr2);
	masked_addr1 = inet_address_mask (addr1, less_specific_mask);
	masked_addr2 = inet_address_mask (addr2, less_specific_mask);

	overlap = inet_address_equal (masked_addr1, masked_addr2);

	/* Destroy only masked addresses. addr1 and addr2 are not copies since	*/
	/* we used the fast version of address_range_addr_get.					*/
	inet_address_destroy (masked_addr1);
	inet_address_destroy (masked_addr2);

	FRET (overlap);
	}

DLLEXPORT void
inet_address_range_mask_set (InetT_Address_Range* addr_range_ptr, InetT_Subnet_Mask mask)
	{
	FIN (inet_address_range_mask_set ());

	addr_range_ptr->subnet_mask = mask;

	FOUT;
	}


DLLEXPORT PrgT_Compcode
inet_ipv6_type_specific_addr_str_create (char* orig_addr_str, PrgT_Boolean eui_64, char* return_addr_str)
	{
	InetT_Address		addr;
	char*				colon_ptr;
	PrgT_Compcode		ret_code = PrgC_Compcode_Failure;
	
	/** This function takes an input string (IPv6 address)	**/
	/** and formats it based on the address type. If the	**/
	/** type is EUI-64, then an address in the format		**/
	/** A:B:C:D is returned, otherwise an address in the 	**/
	/** format A:B:C:D:E:F:G:H is returned.					**/
	/**														**/
	/** The input string can have double colon, embedded	**/
	/** IP addresses etc.									**/
	/**														**/
	/** This function can be used by any client that wants	**/
	/** an address string in which the address type (EUI-64	**/
	/** or non) is implicit. One such client is the active	**/
	/** attribute handler for traffic_flow/ping demand		**/
	/** src/dest, where there is no placeholder for addr	**/
	/** type.												**/		
	FIN (inet_ipv6_type_specific_addr_str_create ());

	/* Use different functions for different address types.	*/
	if (eui_64)
		{
		/* If the address is of type EUI-64, we need to specify	*/
		/* an interface portion, since only the network portion	*/
		/* is known. It does not matter what we specify as the	*/
		/* interface ID, since we will strip the address down	*/
		/* to the network portion (first 64 bits).				*/
		addr = inet_address_create_from_intf_id (orig_addr_str, 0);
		if (inet_address_valid (addr))
			{		
			ret_code = PrgC_Compcode_Success;
			inet_address_print (return_addr_str, addr);

			/* Chop the address after 4 fields (64 bits). That is	*/
			/* the network portion of the EUI-64 address.			*/
			colon_ptr = strchr (return_addr_str, ':');
			colon_ptr = strchr (colon_ptr + 1, ':');
			colon_ptr = strchr (colon_ptr + 1, ':');
			colon_ptr = strchr (colon_ptr + 1, ':');

			/* Truncate the string at the fourth colon.	*/
			*colon_ptr = '\0';
			}
		}
	else
		{
		/* Create the entire 128 bit address and print it back.	*/
		addr = inet_address_create (orig_addr_str, InetC_Addr_Family_v6);
		if (inet_address_valid (addr))
			{
			inet_address_print (return_addr_str, addr);
			ret_code = PrgC_Compcode_Success;
			}
		}
	
	inet_address_destroy (addr);

	FRET (ret_code);
	}

DLLEXPORT int
ip_addr_matching_prefix_len_get (IpT_Address addr1, IpT_Address addr2)
	{
	int			matching_prefix_len = 0;
	unsigned int	xor_product;
	
	/** Compares two addresses and returns the prefix length of the common portion.	**/
	/** Can be used in some circumstances to determine which of a number of addrs 	**/
	/** belongs to the same subnet as a given address (given that one of the 		**/
	/** candidates does indeed belong to the same subnet).							**/
	FIN (ip_addr_matching_prefix_len_get ());

	/* This function assumes that the IpT_Address structure is an unsigned int. A	*/
	/* function that wraps around this must be written to support the inet version	*/
	/* of this function.															*/

	/* Logic: If the two addresses are XORed, then the common prefix portion will	*/
	/* become all zero. Then we count the number of leftmost zero bits. This is the	*/
	/* prefix length that we are looking for.										*/
	xor_product = addr1 ^ addr2;

	/* The number of common bits is the number of left-most zeroes.	The check for 	*/
	/* leftmost zero is that the integer should be less than 2 power 15.			*/
	while (xor_product < 0x80000000)
		{
		matching_prefix_len++;
		xor_product = xor_product << 1;
		}

	FRET (matching_prefix_len);
	}
	
static void
ip_addr_error_abort (const char* str1, const char* str2)
	{
	FIN (ip_addr_error_abort ());

	/* If this code is being used inside DES, call op_sim_error.	*/
#ifdef OPD_DES
	op_sim_error (OPC_SIM_ERROR_ABORT, str1, str2);
#else
	/* If this code is being executed outside DES, op_sim_error is not	*/
	/* available. Use other means to report error.						*/
	/* Allocate zero memory (invalid) to create a recoverable error		*/
	/* so that we have the FCS of the error.							*/
	prg_mem_alloc (0); 
	fprintf (stderr, "%s\n%s\n", str1, str2);	
#endif

	FOUT;
	}

static void
ip_addr_error_recover (const char* str1, const char* str2)
	{
	FIN (ip_addr_error_recover ());

	/* If this code is being used inside DES, call op_sim_error.	*/
#ifdef OPD_DES
	op_sim_error (OPC_SIM_ERROR_RECOVER, str1, str2);
#else
	/* If this code is being executed outside DES, op_sim_error is not	*/
	/* available. Use other means to report error.						*/
	/* Allocate zero memory (invalid) to create a recoverable error		*/
	/* so that we have the FCS of the error.							*/
	prg_mem_alloc (0); 
	fprintf (stderr, "%s\n%s\n", str1, str2);	
#endif

	FOUT;
	}


void
inet_ipv6_addr_bits_from_range_get (InetT_Address_Range* range_ptr, int* int_array)
	{
	/** Copies the 128-bit v6 address from the opaque structure	**/
	/** into an integer array. Used for binary hash keys.		**/
	FIN (inet_ipv6_addr_bits_from_range_get ());

	prg_mem_copy (range_ptr->address.ipv6_addr_ptr, int_array, 16);

	FOUT;
	}

void
inet_ipv6_addr_bits_from_addr_get (InetT_Address* addr_ptr, OpT_uInt32* int_array)
	{
	/** Copies the 128-bit v6 address from the opaque structure	**/
	/** into an integer array. Used for binary hash keys.		**/
	FIN (inet_ipv6_addr_bits_from_range_get ());

	prg_mem_copy (addr_ptr->address.ipv6_addr_ptr, int_array, 16);

	FOUT;
	}

DLLEXPORT 
InetT_Subnet_Mask inet_smask_create (const char* mask_str)
	{
	IpT_Address				ip_smask;
	int						prefix_length;
	InetT_Subnet_Mask		inet_smask = INETC_SMASK_INVALID;
	Boolean					error = PRGC_FALSE;
	
	/** Mask can be specified as address (v4) or prefix length (v4 or v6).	**/
	
	FIN (inet_smask_create ());

	/* Try for prefix check first, because integers are considered valid IP	*/
	/* addresses.															*/
	prefix_length = oms_string_to_int (mask_str, 0, 128, &error);	

	if (!error)
		inet_smask = inet_smask_from_length_create (prefix_length);

	else
		{
		ip_smask = ip_smask_create (mask_str);

		if (!ip_address_equal (ip_smask, IPC_ADDR_INVALID))
			{
			/* Valid IPv4 subnet mask.	*/
			inet_smask = inet_smask_from_ipv4_smask_create (ip_smask);
			}
		}

	FRET (inet_smask);
	}
