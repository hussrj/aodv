/* ip_addr_v4.h: Header file for IP addressing scheme. */

/****************************************/
/*      Copyright (c) 1987-2009       */
/*      by OPNET Technologies, Inc.     */
/*       (A Delaware Corporation)       */
/*    7255 Woodmont Av., Suite 250      */
/*     Bethesda, MD 20814, U.S.A.       */
/*       All Rights Reserved.           */
/****************************************/

/****************************************/
/* !!!!! WARNING !!!!!					*/	
/* THIS FILE IS USED BY SIM AND ETS.	*/
/* THERE SHOULD BE NO OPC CONSTANTS		*/
/* IN THIS FILE.						*/
/****************************************/

#ifndef HEADER_FILE_ip_addr_h
#define HEADER_FILE_ip_addr_h

#include <prg_ip_address.h>
#include <prg_bin_hash.h>


#if defined (__cplusplus)
extern "C" {
#endif

/*	IP address constants.	*/
#define IPC_ADDR_SIZE				4								/* Size of IP address, change for IPv6. 		*/
#define IPC_ADDR_STR_LEN			32								/* Useful size for an IP address string. 		*/
#define IPC_ADDR_DOT_DECIMAL_STR_LEN    16							/* IP address string of the form XXX.XXX.XXX.XXX */

#define IPC_ADDR_NUM_DIGIT_MAX		10								/* Max number of digits in integer form of IP address */
#define IPC_ADDR_INVALID			4278124286U						/* 254.254.254.254 is used as an invalid		*/
																	/* address, as 0.0.0.0 is reserved for other	*/
																	/* address class. 								*/
#define INETC_ADDR_STR_LEN			40								/* Useful size for an IPv4/IPv6 address string. 		*/
#define INETC_ADDR_RANGE_STR_LEN	44								/* Useful size for an IPv4/IPv6 address and mask string.*/

/* Address lengths in bits.			*/
#define IPC_V4_ADDR_LEN				32
#define IPC_V6_ADDR_LEN				128

#define IPC_V6_ADDR_LEN_BYTES		16
#define IPC_V6_ADDR_LEN_INTS		4	

/*	Default Subnet Mask for specified network address class.	*/
#define IPC_SMASK_CLASS_A		4278190080U
#define IPC_SMASK_CLASS_B		4294901760U
#define IPC_SMASK_CLASS_C		4294967040U

/* Special IP Multicast addresses */
#define IPC_ALL_SYSTEMS_MULTICAST_ADDR		"224.0.0.1"
#define	IPC_ALL_ROUTERS_MULTICAST_ADDR		"224.0.0.2"
#define IPC_ALL_PIM_ROUTERS_MULTICAST_ADDR	"224.0.0.13"

/* Special address representing a request for auto-addressing. This     */
/* special value is also defined symbolically by the IP process models  */
/* and  the two must be kept consistent.                                */
#define IPC_AUTO_ADDRESS    				"Auto Assigned"
#define IPC_UNNUMBERED_ADDRESS    			"Unnumbered"
#define IPC_NO_IP_ADDRESS    				"No IP Address"
#define IPC_SUBINTF_ADDR_UNSPEC				"Specify..."	
#define IPC_STUB_NETWORK 					"255.255.255.255"
#define IPC_DEFAULT_RTE_AUTOASSIGNED		"Auto Assigned"	
#define IPC_ROUTER_ID_STR_AUTOASSIGNED		"Auto Assigned"

/* Special values for AS Number Auto assignment			*/
#define IPC_ROUTER_ID_INVALID				0
#define IPC_AS_NUMBER_AUTOASSIGNED			-1

/*	IP address classes.		*/
#define	IPC_ADDRESS_CLASS_UNSPEC	100
#define	IPC_ADDRESS_CLASS_A			101
#define	IPC_ADDRESS_CLASS_B			102
#define	IPC_ADDRESS_CLASS_C			103
#define	IPC_ADDRESS_CLASS_D			104
#define IPC_ADDRESS_CLASS_E			105	

/* Size of planks of IP addresses allocated via the pooled - memory facility. */
#define IPC_ADDR_ALLOC_PLANK_SIZE	1000

/*	IP address data types.	*/
typedef unsigned int				IpT_Address;
typedef	int							IpT_Address_Class;
typedef IpT_Address					IpT_Router_Id;

typedef struct IpT_Address_Range
	{
	IpT_Address 	address;
	IpT_Address		subnet_mask;
	} IpT_Address_Range;

/* Enumerated type to hold the IP address versions	*/
typedef enum InetT_Addr_Family
	{
	InetC_Addr_Family_v4,
	InetC_Addr_Family_v6,
	InetC_Addr_Family_Invalid,
	InetC_Addr_Family_Unknown
	} InetT_Addr_Family;

typedef enum InetT_Ipv6_Addr_Type
	{
	InetC_Ipv6_Addr_Type_Non_EUI_64,
	InetC_Ipv6_Addr_Type_EUI_64
	} InetT_Ipv6_Addr_Type;

/* Structure that can hold both IPv4 and IPv6 addresses.				*/
typedef struct InetT_Address
	{
	unsigned char				addr_family;		/* InetT_Addr_Family value	*/
	union
		{
		IpT_Address				ipv4_addr;			/* Ipv4 address.			*/
		struct Ipv6T_Address*	ipv6_addr_ptr;		/* Ipv6 address.			*/
		} address;
	} InetT_Address;

/* Data type to hold the subnet mask (prefix length) of a v4 or v6 		*/
/* address.																*/
typedef unsigned char	InetT_Subnet_Mask;

/* Structure to hold a v4 or v6 address range (Network address and		*/
/* subnet mask.															*/
typedef struct InetT_Address_Range
	{
	unsigned char				addr_family;		/* InetT_Addr_Family value	*/
	InetT_Subnet_Mask			subnet_mask;		/* Subnet mask.				*/
	union
		{
		IpT_Address				ipv4_addr;			/* Ipv4 address.			*/
		struct Ipv6T_Address*	ipv6_addr_ptr;		/* Ipv6 address.			*/
		} address;
	} InetT_Address_Range;

/* Structure to hold per-prefix based admin weight & filter information	*/
typedef struct InetT_Prefix_Distance
	{
	int					distance;
	InetT_Address		inet_address;
	InetT_Subnet_Mask	inet_smask;	
	void*				filter_list_ptr;/* A list of type IpT_Acl_List* */
	} InetT_Prefix_Distance;

/* Structure that represents a hash table in which the keys are			*/
/* InetT_Address values.												*/
typedef struct InetT_Address_Hash_Table
	{
	PrgT_Bin_Hash_Table*	ipv4_hash_table;
	PrgT_Bin_Hash_Table*	ipv6_hash_table;
	} InetT_Address_Hash_Table;

/* Structure that represents a hash table in which the keys are			*/
/* InetT_Address values.												*/
typedef struct InetT_Address_Hash_Table*	InetT_Address_Hash_Table_Handle;
typedef void (*InetT_Address_Hash_Elem_Dealloc_Proc)(void * item);

#define INETC_ADDRESS_HASH_TABLE_INVALID	((InetT_Address_Hash_Table_Handle) OPC_NIL)

/* Macros to Get and Set IP address component based on byte index. 		*/
/* IP address occupies 32 bits. If the components are c0, c1, c2, c3	*/
/* where c0 is highest order byte and c3 is lowest order byte, address	*/
/* storage using formula: (c0<<24 || c1<<16 || c2<<8 || c3). An address	*/
/* component is retrieved using the reverse(right shift) of the formula.*/
#define IP_ADDRESS_COMPONENT_GET(addr,index) ((addr >> (24 - (index << 3))) & 255)
#define IP_ADDRESS_COMPONENT_SET(addr,index,value) (addr = (addr & \
	IP_ADDRESS_COMPONENT_MASK(index)) | (value << ( 24 - (index << 3)))) 

/* Get a mask which masks all address components except the index-th one*/
#define IP_ADDRESS_COMPONENT_MASK(index) (~(255 << (24 - (index << 3))))

/* Leaving this macro for backward compatibility. This is deprecated as	*/
/* it can only get a component of IP address whereas earlier this macro	*/
/* could set and get a component.										*/
#define IP_ADDRESS_COMPONENT(addr,index) ((addr >> (24 - (index << 3))) & 255)

#define IpI_No_Ip_Address		IpI_Default_Addr

/* Global defined in ip_addr_v4.ex.c */
extern IpT_Address IpI_Broadcast_Addr;
extern IpT_Address IpI_Default_Addr;

extern InetT_Address 		InetI_Invalid_Addr;
extern InetT_Address 		InetI_Invalid_v4_Addr;
extern InetT_Address 		InetI_Default_v4_Addr;
extern InetT_Address 		InetI_Broadcast_v4_Addr;
extern InetT_Address_Range	InetI_Addr_Range_Invalid;

extern InetT_Address		InetI_Ipv6_All_Nodes_NL_Mcast_Addr;
extern InetT_Address		InetI_Ipv6_All_Nodes_LL_Mcast_Addr;
extern InetT_Address		InetI_Ipv6_All_Routers_NL_Mcast_Addr;
extern InetT_Address		InetI_Ipv6_All_Routers_LL_Mcast_Addr;
extern InetT_Address		InetI_Ipv6_All_Routers_SL_Mcast_Addr;
extern InetT_Address		InetI_Default_v6_Addr;

/* Multicast OSPF addresses. */
extern InetT_Address		OspfI_IPv6_All_Spf_Routers;
extern InetT_Address		OspfI_IPv6_All_Dr_Routers;

#define InetI_No_Ipv4_Address		InetI_Default_v4_Addr
#define INETC_ADDRESS_INVALID		InetI_Invalid_Addr
#define INETC_ADDR_RANGE_INVALID	InetI_Addr_Range_Invalid
#define INETC_SMASK_INVALID			((InetT_Subnet_Mask) 255)
#define INETC_SMASK_IPV6_HOST		((InetT_Subnet_Mask) 128)
#define INETC_SMASK_IPV4_HOST		((InetT_Subnet_Mask) 32)

#define IPv6C_ALL_NODES_NL_MCAST_ADDR	InetI_Ipv6_All_Nodes_NL_Mcast_Addr
#define IPv6C_ALL_NODES_LL_MCAST_ADDR	InetI_Ipv6_All_Nodes_LL_Mcast_Addr
#define IPv6C_ALL_RTRS_NL_MCAST_ADDR	InetI_Ipv6_All_Routers_NL_Mcast_Addr
#define IPv6C_ALL_RTRS_LL_MCAST_ADDR	InetI_Ipv6_All_Routers_LL_Mcast_Addr
#define IPv6C_ALL_RTRS_SL_MCAST_ADDR	InetI_Ipv6_All_Routers_SL_Mcast_Addr
#define IPv6C_UNSPECIFIED_ADDRESS		InetI_Default_v6_Addr

#define InetI_Ipv6_Ipv4_Compabible_Addr_Preifx	InetI_Default_v6_Addr
extern IpT_Address	ip_subnet_masks[];

extern unsigned int	IpI_Interface_ID;

extern const char*	addr_family_strings [];

extern PrgT_Boolean ip_addr_initialization_complete;

#define	ip_address_pkg_initialize()			if (PRGC_FALSE == ip_addr_initialization_complete) ip_address_pkg_init ();

/*	IPv4 address function-based interfaces. */
DLLEXPORT void					ip_address_pkg_init (void);
DLLEXPORT IpT_Address				ip_address_create (const char *addr_str);
DLLEXPORT IpT_Address*			ip_address_create_dynamic (IpT_Address addr);
DLLEXPORT void					ip_address_destroy_dynamic (IpT_Address* addr_ptr);
DLLEXPORT int						ip_address_compare (IpT_Address addr1, IpT_Address addr2);

#define ip_address_string_test(_addr_str) ((PrgT_Boolean)(inet_addr_str_is_valid_v4 (_addr_str)))

DLLEXPORT void					ip_address_print (char *str, IpT_Address addr);
DLLEXPORT void					ip_intf_address_print (char *str, IpT_Address addr);
DLLEXPORT void					ip_address_print_debug (IpT_Address addr);

DLLEXPORT IpT_Address_Range *		ip_address_range_create (IpT_Address addr, IpT_Address subnet_mask);
DLLEXPORT void					ip_address_range_destroy (IpT_Address_Range *addr_range_ptr);
DLLEXPORT IpT_Address_Range *		ip_address_range_copy (IpT_Address_Range *addr_range_ptr);
DLLEXPORT void					ip_address_range_print (char *addr_str, char *subnet_mask_str, IpT_Address_Range *addr_range_ptr);
DLLEXPORT PrgT_Boolean					ip_address_range_check (IpT_Address addr, IpT_Address_Range *addr_range_ptr);
DLLEXPORT IpT_Address_Class		ip_address_class (IpT_Address addr);
DLLEXPORT IpT_Address				ip_default_smask_create (IpT_Address ip_addr);
DLLEXPORT IpT_Address				ip_default_smask_from_class (IpT_Address_Class addr_class);
DLLEXPORT void					ip_address_fields_obtain (IpT_Address addr, int *field1, int *field2, int *field3, int *field4);
DLLEXPORT PrgT_Boolean					ip_smask_address_verify (IpT_Address address, IpT_Address mask);

DLLEXPORT PrgT_Boolean					ip_address_is_broadcast (IpT_Address addr, IpT_Address subnet_mask);
DLLEXPORT PrgT_Boolean					ip_address_is_network (IpT_Address addr, IpT_Address subnet_mask);
DLLEXPORT PrgT_Compcode				ip_net_address_increment (IpT_Address* addr_ptr, IpT_Address_Class address_class);
DLLEXPORT PrgT_Compcode 				ip_address_increment (IpT_Address* addr_ptr, IpT_Address subnet_mask);
DLLEXPORT int						ip_smask_length_count (IpT_Address smask);

DLLEXPORT IpT_Address				Ip_Next_Available_Mcast_Addr (void);
DLLEXPORT int						ip_address_ptr_compare (IpT_Address* addr_ptr1, IpT_Address* addr_ptr2);
DLLEXPORT IpT_Address				ip_smask_create (const char *smask_str);
DLLEXPORT IpT_Address				ip_address_node_broadcast_create (IpT_Address address, IpT_Address subnet_mask);


#define	IPC_INITIAL_MCAST_ADDR 		(ip_address_create ("224.0.0.13"))


/* IPv4 Address Macro-based interfaces. */
#define ip_address_equal(addr1,addr2)  						(addr1 == addr2)
#define ip_address_mask_fast(addr,mask) 					(addr & mask)
#define ip_address_mask(addr,mask) 							(addr & mask)
#define ip_address_from_int(int_addr) 						(int_addr)
#define ip_address_to_int(addr) 							(addr)
#define ip_address_copy(addr) 								(addr)
#define ip_address_compare_fast(addr1, addr2) 				(int) ((addr1) - (addr2))
#define ip_address_complement(addr)							(~addr)
#define ip_address_is_multicast(addr)						(IP_ADDRESS_COMPONENT_GET(addr,0) > 223 &&		\
															 IP_ADDRESS_COMPONENT_GET(addr,0) < 240)
#define ip_smask_from_length_create(length)					(ip_subnet_masks[length])
#define ip_network_address_is_classful(_addr)				(ip_address_equal (_addr, ip_address_mask (_addr, ip_default_smask_create (_addr))))

/* Phase out: other interfaces have replaced these. */
#define ip_address_complement_create(addr)	(~addr)
#define ip_address_destroy(addr) 			

/* InetT_Address related functions			*/

#define		inet_addr_family_string(_addr_family)		(addr_family_strings[_addr_family])
DLLEXPORT InetT_Address			inet_address_create (const char* ip_addr_str, InetT_Addr_Family addr_family);
DLLEXPORT InetT_Address			inet_address_create_from_intf_id (const char* ip_addr_str, int intf_id);
DLLEXPORT InetT_Address			inet_ipv6_link_local_addr_create (int intf_id);
DLLEXPORT InetT_Address*			inet_address_mem_alloc (void);
DLLEXPORT char*					inet_address_str_mem_alloc (void);
DLLEXPORT InetT_Address*			inet_address_create_dynamic (InetT_Address addr);
DLLEXPORT InetT_Address*			inet_address_copy_dynamic (InetT_Address* addr_ptr);
DLLEXPORT InetT_Address			inet_address_from_ipv4_address_create (IpT_Address ipv4_address);
DLLEXPORT InetT_Address			inet_address_from_ipv4_address_create_invalid_check (IpT_Address ipv4_address);
DLLEXPORT void					inet_address_destroy_dynamic (InetT_Address* addr_ptr);
DLLEXPORT PrgT_Boolean					inet_address_ptr_equal (const InetT_Address* addr1_ptr, const InetT_Address* addr2_ptr);
DLLEXPORT int						inet_address_ptr_compare_proc (const void* a_ptr, const void* b_ptr);
DLLEXPORT int						inet_address_ptr_compare (const InetT_Address* addr1_ptr, const InetT_Address* addr2_ptr);
DLLEXPORT PrgT_Boolean					inet_address_ptr_less_than (const InetT_Address* addr1_ptr, const InetT_Address* addr2_ptr);
DLLEXPORT PrgT_Boolean					inet_address_ptr_greater_than (const InetT_Address* addr1_ptr, const InetT_Address* addr2_ptr);
#define		inet_address_ptr_ge(_addr1_ptr, _addr2_ptr) (!inet_address_ptr_less_than (_addr1_ptr, _addr2_ptr))
#define		inet_address_ptr_le(_addr1_ptr, _addr2_ptr) (!inet_address_ptr_greater_than (_addr1_ptr, _addr2_ptr))
DLLEXPORT InetT_Address			inet_ipv6_address_ptr_copy (const InetT_Address* addr_ptr);
DLLEXPORT PrgT_Boolean					inet_address_string_test (const char* ip_addr_str, InetT_Addr_Family addr_family);
DLLEXPORT InetT_Address			inet_address_ptr_complement (const InetT_Address* addr_ptr);
DLLEXPORT InetT_Address			inet_address_ptr_mask (const InetT_Address* addr_ptr, InetT_Subnet_Mask subnet_mask);
DLLEXPORT char*					inet_address_ptr_print (char* addr_str, const InetT_Address* addr_ptr);
DLLEXPORT void					inet_address_ici_field_print (void* addr_ptr, PrgT_List* field_value_str_lptr);
DLLEXPORT PrgT_Boolean					inet_address_ptr_is_multicast (const InetT_Address* addr_ptr);
DLLEXPORT InetT_Address			inet_address_ptr_and (const InetT_Address* addr1_ptr, const InetT_Address* addr2_ptr);
DLLEXPORT InetT_Address			inet_address_ptr_or (const InetT_Address* addr1_ptr, const InetT_Address* addr2_ptr);
DLLEXPORT InetT_Address			inet_ipv4_compat_addr_from_ipv4_addr_create (const InetT_Address ipv4_addr);
DLLEXPORT InetT_Address			inet_ipv4_mapped_addr_from_ipv4_addr_create (const InetT_Address ipv4_addr);
DLLEXPORT InetT_Address			inet_ipv4_addr_from_ipv4_compat_addr_get (const InetT_Address* ipv4_compat_addr_ptr);
DLLEXPORT InetT_Address			inet_ipv4_addr_from_ipv4_mapped_addr_get (const InetT_Address* ipv4_mapped_addr_ptr);
DLLEXPORT InetT_Address			inet_ipv4_addr_from_6to4_addr_get (const InetT_Address* ipv6_6to4_addr_ptr);
DLLEXPORT PrgT_Boolean					inet_address_is_6to4 (const InetT_Address* addr_ptr);
DLLEXPORT PrgT_Boolean					inet_address_is_ipv4_compat (const InetT_Address* addr_ptr);
DLLEXPORT PrgT_Boolean					inet_address_is_ipv4_mapped (const InetT_Address* addr_ptr);
DLLEXPORT void*					inet_address_addr_ptr_get (InetT_Address* addr_ptr);
DLLEXPORT const void*				inet_address_addr_ptr_get_const (const InetT_Address* addr_ptr);
DLLEXPORT PrgT_Compcode				inet_address_increment (InetT_Address* addr_ptr, InetT_Subnet_Mask smask);
DLLEXPORT InetT_Address			inet_solicited_node_mcast_addr_create (const InetT_Address ipv6_address);

PrgT_Compcode		inet_addr_str_parse_v4 (const char* addr_str, unsigned int* v4_addr_ptr);
PrgT_Compcode		inet_addr_str_parse_v6 (const char* addr_str, int length, unsigned int** v6_addr_ptr);


#define		inet_address_destroy(_addr)					if ((_addr).addr_family == InetC_Addr_Family_v6) prg_mem_free ((_addr).address.ipv6_addr_ptr);
#define		inet_address_equal(_addr1, _addr2)			(inet_address_ptr_equal (&(_addr1), &(_addr2)))
#define		inet_address_copy(_addr)					(((_addr).addr_family != InetC_Addr_Family_v6) ? (_addr) : inet_ipv6_address_ptr_copy (&(_addr)))
/* C compilers generate an error if the above macro is used with	*/
/* a variable to type const InetT_Address. In such cases, use the	*/
/* macro below.														*/
#define		inet_address_copy_const(_dst, _src)			if ((_src).addr_family != InetC_Addr_Family_v6) \
															_dst = (_src); \
														else \
															_dst = inet_ipv6_address_ptr_copy (&(_src));
#define		inet_address_complement(_addr)				(inet_address_ptr_complement (&(_addr)))
#define		inet_address_mask(_addr, _mask)				(inet_address_ptr_mask (&(_addr), (_mask)))
#define		inet_address_print(_str, _addr)				(inet_address_ptr_print ((_str), &(_addr)))
#define		inet_address_is_multicast(_addr)			(inet_address_ptr_is_multicast (&(_addr)))
#define		inet_address_family_get(addr_ptr)			((InetT_Addr_Family) (addr_ptr)->addr_family)
#define		inet_ipv4_address_get(addr)					((addr).address.ipv4_addr)
#define		inet_address_valid(addr)					((addr).addr_family != InetC_Addr_Family_Invalid) 
#define		inet_address_class(_addr)					(ip_address_class ((_addr).address.ipv4_addr))
#define		inet_default_smask_create(_addr)			(inet_default_smask_for_addr_ptr_create (&(_addr)))
#define		inet_address_is_broadcast(_addr, _mask)		(ip_address_is_broadcast \
				(inet_ipv4_address_get (_addr), ip_smask_from_inet_smask_create (_mask)))
#define		inet_address_is_network(_addr, _mask)		(ip_address_is_network \
				(inet_ipv4_address_get (_addr), ip_smask_from_inet_smask_create (_mask)))
#define		inet_address_and(_addr1, _addr2)			(inet_address_ptr_and (&(_addr1), &(_addr2)))
#define		inet_address_or(_addr1, _addr2)				(inet_address_ptr_or (&(_addr1), &(_addr2)))
#define		inet_rte_ipv6_addr_is_link_local(_addr)		(ipv6_addr_is_link_local (_addr.address.ipv6_addr_ptr))
#define		inet_network_address_is_classful(_addr)		(ip_network_address_is_classful (inet_ipv4_address_get (_addr)))
#define		inet_addr_is_ipv6_link_local(_addr)			(((_addr).addr_family == InetC_Addr_Family_v6) && inet_rte_ipv6_addr_is_link_local(_addr))
#define     inet_address_is_default(addr)               (inet_address_equal (addr, InetI_Default_v4_Addr) ||\
				inet_address_equal (addr, InetI_Default_v6_Addr) || ip_address_equal (IpI_Default_Addr, inet_ipv4_address_get (addr)))

#define		inet_address_range_is_invalid(_addr_range)	((_addr_range).addr_family == InetC_Addr_Family_Invalid) 

/* InetT_Subnet_Mask related functions		*/
DLLEXPORT InetT_Subnet_Mask		inet_default_smask_for_addr_ptr_create (const InetT_Address* addr_ptr);
DLLEXPORT InetT_Subnet_Mask		inet_default_smask_from_class (IpT_Address_Class addr_class);
#define		inet_smask_from_length_create(prefix_len)		((InetT_Subnet_Mask) (prefix_len))
#define		inet_smask_from_ipv4_smask_create(_mask)		((InetT_Subnet_Mask) ip_smask_length_count (_mask))
#define		inet_smask_length_count(subnet_mask)			((int) (subnet_mask))
#define		inet_smask_valid(smask)							((smask) != INETC_SMASK_INVALID)
#define		inet_smask_equal(mask1, mask2)					((mask1) == (mask2))
#define		ip_smask_from_inet_smask_create(_smask)			(ip_smask_from_length_create (_smask))

DLLEXPORT InetT_Subnet_Mask			inet_smask_create (const char* mask_str);

/* InetT_Address_Range related functions	*/
DLLEXPORT InetT_Address_Range		inet_address_range_create (InetT_Address addr, InetT_Subnet_Mask subnet_mask);
DLLEXPORT InetT_Address_Range		inet_address_range_create_fast (InetT_Address addr, InetT_Subnet_Mask subnet_mask);
DLLEXPORT InetT_Address_Range		inet_address_range_network_create (InetT_Address addr, InetT_Subnet_Mask subnet_mask);
DLLEXPORT InetT_Address_Range		inet_address_range_network_from_addr_range_create (InetT_Address_Range* addr_range_ptr);
DLLEXPORT InetT_Address_Range		inet_ipv6_address_range_from_intf_id_create (InetT_Address addr, unsigned int intf_id, InetT_Subnet_Mask subnet_mask);
DLLEXPORT InetT_Address_Range		inet_address_range_ptr_copy (const InetT_Address_Range* addr_range_ptr);
DLLEXPORT void					inet_address_range_destroy (InetT_Address_Range* addr_range_ptr);
DLLEXPORT PrgT_Boolean					inet_address_range_check (InetT_Address addr, const InetT_Address_Range* addr_range_ptr);
DLLEXPORT PrgT_Boolean					inet_address_range_equal (const InetT_Address_Range* addr_range1_ptr, const InetT_Address_Range* addr_range2_ptr);
DLLEXPORT PrgT_Boolean					inet_address_range_address_equal (const InetT_Address_Range* addr_range1_ptr, const InetT_Address* addr_ptr);
DLLEXPORT PrgT_Boolean					inet_address_range_network_address_equal (const InetT_Address_Range* addr_range1_ptr, const InetT_Address* addr_ptr);
DLLEXPORT InetT_Address			inet_address_range_addr_get (const InetT_Address_Range* addr_range_ptr);
DLLEXPORT void*					inet_address_range_addr_ptr_get (InetT_Address_Range* addr_range_ptr);
DLLEXPORT InetT_Address			inet_address_range_addr_get_fast (const InetT_Address_Range* addr_range_ptr);
DLLEXPORT InetT_Address			inet_address_range_broadcast_addr_get (const InetT_Address_Range* addr_range_ptr);
DLLEXPORT InetT_Address			inet_address_range_network_addr_get (const InetT_Address_Range* addr_range_ptr);
DLLEXPORT PrgT_Boolean					inet_address_range_addr_is_broadcast (const InetT_Address_Range* addr_range_ptr, const InetT_Address* addr_ptr);
DLLEXPORT char*					inet_address_range_print (char* addr_range_str, const InetT_Address_Range* addr_range_ptr);
DLLEXPORT char*					inet_address_range_address_print (char* addr_str, const InetT_Address_Range* addr_range_ptr);
DLLEXPORT InetT_Address_Range		inet_ipv4_address_range_create (IpT_Address addr, IpT_Address subnet_mask);
DLLEXPORT InetT_Address_Range		inet_ipv4_address_range_network_create (IpT_Address addr, IpT_Address subnet_mask);
DLLEXPORT IpT_Address_Range		inet_ipv4_address_range_get (const InetT_Address_Range* addr_range_ptr);
DLLEXPORT InetT_Address_Range		inet_address_range_str_parse (const char* addr_range_str, InetT_Addr_Family addr_family);
DLLEXPORT void					inet_address_range_ici_field_print (void* addr_range_ptr, PrgT_List* field_value_str_lptr);

DLLEXPORT void					inet_address_print_debug (const InetT_Address* addr_ptr);
DLLEXPORT int						inet_address_range_compare (const InetT_Address_Range* addr_range1_ptr, const InetT_Address_Range* addr_range2_ptr);
DLLEXPORT InetT_Address_Range*	inet_address_range_mem_alloc (void);
	
#define		inet_address_range_family_get(_addr_range_ptr)	((InetT_Addr_Family) (_addr_range_ptr)->addr_family)
#define		inet_address_range_mask_get(_addr_range_ptr)	((_addr_range_ptr)->subnet_mask)
#define		inet_address_range_mask_len_get(_addr_rnge_ptr)	inet_smask_length_count((_addr_rnge_ptr)->subnet_mask)
#define		inet_address_range_mask_equal(_addr_range_ptr, _mask) \
															(((_addr_range_ptr)->subnet_mask) == (_mask))
#define		inet_address_range_copy(_addr_range)			(inet_address_range_ptr_copy(&(_addr_range)))
#define		inet_address_range_ipv4_address_equal(addr_range_ptr,_addr)	\
															(((addr_range_ptr)->addr_family == InetC_Addr_Family_v4) && \
															 (ip_address_equal ((addr_range_ptr)->address.ipv4_addr, _addr)))
#define		inet_address_range_ipv4_mask_equal(addr_range_ptr,_mask) \
															(((addr_range_ptr)->addr_family == InetC_Addr_Family_v4) && \
															 (ip_address_equal (ip_smask_from_inet_smask_create ((addr_range_ptr)->subnet_mask), _mask)))
#define		inet_address_range_ipv4_addr_get(_range_ptr)	((_range_ptr)->address.ipv4_addr)
#define		inet_address_range_ipv4_mask_get(_range_ptr)	(ip_smask_from_inet_smask_create ((_range_ptr)->subnet_mask))
#define		inet_ipv4_address_range_check(_addr,_range_ptr) (ip_address_equal ( \
															ip_address_mask ((_range_ptr)->address.ipv4_addr, inet_address_range_ipv4_mask_get(_range_ptr)), \
															ip_address_mask ((_addr).address.ipv4_addr, inet_address_range_ipv4_mask_get(_range_ptr))))
#define		inet_ipv4_ntwk_address_range_check(_addr,_range_ptr) \
															(ip_address_equal ((_range_ptr)->address.ipv4_addr, ip_address_mask \
															((_addr).address.ipv4_addr, inet_address_range_ipv4_mask_get(_range_ptr))))
#define		inet_ipv4_address_range_equal(_range1_ptr, _range2_ptr) \
															((ip_address_equal ((_range1_ptr)->address.ipv4_addr, (_range2_ptr)->address.ipv4_addr)) && \
															 ((_range1_ptr)->subnet_mask == (_range2_ptr)->subnet_mask))
#define		inet_address_range_valid(addr_range)			((addr_range).addr_family != InetC_Addr_Family_Invalid) 

void		inet_ipv6_addr_bits_from_range_get	(InetT_Address_Range* range_ptr, int* int_array);
void		inet_ipv6_addr_bits_from_addr_get (InetT_Address* addr_ptr, unsigned int* int_array);															

/* Inet Address hash table related functions	*/
DLLEXPORT InetT_Address_Hash_Table_Handle	inet_addr_hash_table_create (int ipv4_htable_size, int ipv6_htable_size);
DLLEXPORT void							inet_addr_hash_table_destroy (InetT_Address_Hash_Table_Handle hash_handle,
											InetT_Address_Hash_Elem_Dealloc_Proc dealloc_proc);
DLLEXPORT void*							inet_addr_hash_table_item_get (InetT_Address_Hash_Table_Handle htable_handle,
											const InetT_Address* key);
DLLEXPORT void							inet_addr_hash_table_item_insert (InetT_Address_Hash_Table_Handle htable_handle,
											const InetT_Address* key, void * value_ptr, void ** old_value_ptr);
DLLEXPORT void*							inet_addr_hash_table_item_remove (InetT_Address_Hash_Table_Handle htable_handle,
											const InetT_Address* key);
DLLEXPORT PrgT_List*					inet_addr_hash_table_item_list_get (InetT_Address_Hash_Table_Handle htable_handle, 
											InetT_Addr_Family key_addr_family);

/* IPv6 related functions. These functions MUST not be called directly from outside this file */
DLLEXPORT PrgT_Boolean		ipv6_addr_is_link_local (struct Ipv6T_Address* ipv6_addr_ptr);


/****************************************************/
/* New functionality, used by auto-addressing etc.	*/
/****************************************************/
DLLEXPORT PrgT_IP_Address
inet_ipv4_addr_range_to_prg_ip_address (InetT_Address_Range* addr_range_ptr);

DLLEXPORT InetT_Address_Range
inet_ipv4_addr_range_from_prg_ip_address (PrgT_IP_Address* prg_addr);

DLLEXPORT PrgT_Compcode
inet_address_subnet_next (InetT_Address_Range* addr_ptr);

DLLEXPORT PrgT_Boolean
inet_addr_str_is_valid_v4 (const char* addr_str);

DLLEXPORT PrgT_Boolean
inet_addr_str_is_valid_v6 (const char* addr_str, InetT_Ipv6_Addr_Type addr_type);

DLLEXPORT InetT_Address_Range
inet_ipv6_address_range_parse (const char* addr_str, int prefix_len, InetT_Ipv6_Addr_Type type);

DLLEXPORT InetT_Address_Range
inet_ipv4_address_range_parse (const char* addr_str, const char* mask_str);

DLLEXPORT InetT_Address_Range
inet_address_range_parse (const char* cidr_range_str);	

DLLEXPORT PrgT_Boolean
inet_address_subnets_overlap (const InetT_Address_Range* addr1_ptr, const InetT_Address_Range* addr2_ptr);

DLLEXPORT void
inet_address_range_mask_set (InetT_Address_Range* addr_range_ptr, InetT_Subnet_Mask mask);

DLLEXPORT InetT_Address
inet_ipv6_address_from_bits_create (unsigned int* addr_array);

DLLEXPORT PrgT_Compcode
inet_ipv6_type_specific_addr_str_create (char* orig_addr, PrgT_Boolean eui_64, char* return_addr_str);

DLLEXPORT int
ip_addr_matching_prefix_len_get (IpT_Address addr1, IpT_Address addr2);

#if defined (__cplusplus)
} /* end of 'extern "C" {' */
#endif

DLLEXPORT InetT_Address
inet_ipv6_address_from_bits_create (unsigned int* addr_array);



#endif /* for HEADER_FILE_ip_addr_h */


