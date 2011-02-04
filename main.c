#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pcap.h>
#include <stdint.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <net/ethernet.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/tcp.h>

#include <libhashish.h>

#include <ipq_api.h>
#include "ipq_api.h"
#include "config.h"
#include "srpc.h"

enum capture_type {
  FILE_CAPTURE,
  DEVICE_CAPTURE,
  MAX_CAPTURE_TYPE,
};

#define			MAX_OSDPI_IDS			50000
#define			MAX_OSDPI_FLOWS			200000

// id tracking
struct osdpi_id {
	u8 ip[4];
	struct ipoque_id_struct *ipoque_id;
};

// flow tracking
struct osdpi_flow {
  u32 lower_ip;
  u32 upper_ip;
  u16 lower_port;
  u16 upper_port;
  u8 protocol;
  uint32_t byte_count;
  uint32_t pkt_count;
  uint32_t last_pkt;
  struct ipoque_flow_struct *ipoque_flow;
  
  // result only, not used for flow identification
  u32 detected_protocol;
};

static const char *protocol_long_str[] = { IPOQUE_PROTOCOL_LONG_STRING };

struct str_cfg {
  int verbose;
  int type;
  char filename[1000];
  char dev_name[1000];
  char pcap_filter[1000];
  pcap_t * pcap_dev;

  char target[100];
  unsigned short port;
  
  RpcConnection rpc;

  struct ipoque_detection_module_struct *ipoque_struct;

  // results
/*   uint64_t raw_packet_count; */
/*   uint64_t ip_packet_count; */
/*   uint64_t total_bytes; */
/*   uint64_t protocol_counter[IPOQUE_MAX_SUPPORTED_PROTOCOLS + 1]; */
/*   uint64_t protocol_counter_bytes[IPOQUE_MAX_SUPPORTED_PROTOCOLS + 1]; */
  
/*   struct osdpi_id *osdpi_ids; */
/*   uint32_t osdpi_id_count; */
/*   uint32_t size_id_struct; */

/*   struct osdpi_flow *osdpi_flows; */
/*   uint32_t size_flow_struct; */
/*   uint32_t osdpi_flow_count; */

  ///a hast structure to store state
  hi_handle_t *hi_handle_ip;
  hi_handle_t *hi_handle_flows;

};

struct str_cfg obj_cfg;

#define ETHER_ADDR_SIZE 6
#define SIZE_ETHERNET 14

struct packet_header {
  struct ether_header *ether;
  struct iphdr *ip;
  struct udphdr *udp;
  struct tcphdr *tcp;
};

#define USAGE "./linklogger [-i device -f file -v]"


/*
 * A function to fill in a packet_header structure from a captured file
 * return 0 in case of problem during decompression.
 */
int 
extract_headers(struct packet_header *hdr, uint8_t *data, int data_len) {
  int ptr = 0;
  
  //extract ethernet header
  if(data_len < ETHER_HDR_LEN) return 0;
  hdr->ether = (struct ether_header *)data;
  ptr += ETHER_HDR_LEN;

  if(ntohs(hdr->ether->ether_type) != ETHERTYPE_IP) return 0;

  //extract ip headers
  if(data_len < ptr + sizeof(struct iphdr)) return 0;
  hdr->ip = (struct iphdr *)(data + ptr);
  if(data_len < ptr + (hdr->ip->ihl)*4) return 0;  
  ptr += hdr->ip->ihl*4;

  //extract tcp/udp header
  if(hdr->ip->protocol == 6) { //TCP packet
    if(data_len < ptr + sizeof(struct tcphdr)) return 0;
    hdr->tcp = (struct tcphdr *)(data + ptr);
    
  } else if(hdr->ip->protocol == 17) { //UDP packet
    if(data_len < ptr + sizeof(struct udphdr)) return 0;
    hdr->udp = (struct udphdr *)(data + ptr);
  } else 
    return 0;
  return 1;
}

static void 
*get_id(const u8 * ip) {
  uint32_t i, res;
  char str_ip[20];
  struct in_addr addr;
  struct osdpi_id *data;

  addr.s_addr = ip;

  sprintf(str_ip, "%s", inet_ntoa(addr));
  res = hi_get_str(obj_cfg.hi_handle_ip, str_ip, (void *)&data);

  //if state found retrurn object
  if(res == HI_ERR_SUCCESS) {
    return data->ipoque_id;
  } else {
    //if file not found create new state
    data = malloc(sizeof(struct osdpi_id));
    if(data == NULL) {
      perror("malloc osdpi_id");
      exit(1);
    }
    memcpy(data->ip, ip, 4);
    data->ipoque_id = calloc(1, ipoque_detection_get_sizeof_ipoque_id_struct());
    if(data->ipoque_id == NULL) {
      perror("malloc osdpi_id->ipoque_id");
      exit(1);
    }
    hi_insert_str(obj_cfg.hi_handle_ip, str_ip, data);
    return  data->ipoque_id;
  }
}

struct osdpi_flow *
get_osdpi_flow(const struct packet_header *hdr, 
					 u16 ipsize, uint32_t time)
{
  int res;
  char lower_ip[20], upper_ip[20];
  struct in_addr addr;
  u16 lower_port, upper_port;
  struct osdpi_flow *data;

  //XXX.XXX.XXX.XXX:XXXXX-XXX.XXX.XXX.XXX:XXXXX-XXX
  char flow_key[50];

  if (hdr->ip->saddr < hdr->ip->daddr) {
    addr.s_addr = hdr->ip->saddr;
    sprintf(lower_ip, "%s", inet_ntoa(addr));
    addr.s_addr = hdr->ip->daddr;
    sprintf(upper_ip, "%s", inet_ntoa(addr));
  } else {
    addr.s_addr = hdr->ip->saddr;
    sprintf(upper_ip, "%s", inet_ntoa(addr));
    addr.s_addr = hdr->ip->daddr;
    sprintf(lower_ip, "%s", inet_ntoa(addr));
  }
  
  if (hdr->ip->protocol == 6) { // tcp
    if (hdr->ip->saddr < hdr->ip->daddr) 
      snprintf(flow_key, 50, "%x:%05d-%x:%05d-TCP", lower_ip, ntohs(hdr->tcp->source),
	       upper_ip, ntohs(hdr->tcp->dest));
    else
      snprintf(flow_key, 50, "%x:%05d-%x:%05d-TCP", lower_ip, ntohs(hdr->tcp->dest),
	       upper_ip, ntohs(hdr->tcp->source));
  } else if (hdr->ip->protocol == 17) {// udp
    if (hdr->ip->saddr < hdr->ip->daddr) 
      snprintf(flow_key, 50, "%x:%05d-%x:%05d-UDP", lower_ip, ntohs(hdr->tcp->source),
	       upper_ip, ntohs(hdr->tcp->dest));
    else
      snprintf(flow_key, 50, "%x:%05d-%x:%05d-UDP", lower_ip, ntohs(hdr->tcp->dest),
	       upper_ip, ntohs(hdr->tcp->source));
    
  } 
  
  res = hi_get_str(obj_cfg.hi_handle_ip, flow_key, (void **)&data);  
  //if state found retrurn object
  if(res == HI_ERR_SUCCESS) {
    data->byte_count += ipsize;
    data->pkt_count++;
    data->last_pkt += time;
    return data;
  } else {
    //if file not found create new state
    data = malloc(sizeof(struct osdpi_flow));
    if(data == NULL) {
      perror("malloc osdpi_flow");
      exit(1);
    }
    
    data->byte_count = ipsize;
    data->pkt_count = 1;
    data->last_pkt = time;
    data->ipoque_flow = calloc(1, ipoque_detection_get_sizeof_ipoque_flow_struct());
    if(data->ipoque_flow == NULL) {
      perror("malloc osdpi_id->ipoque_id");
      exit(1);
    }
    hi_insert_str(obj_cfg.hi_handle_ip, flow_key, data);
    return  data;
  }
}

void 
process_packet(u_char *args, const struct pcap_pkthdr* pkthdr, const u_char* packet) {
  struct packet_header hdr;
  char src_ip[20], dst_ip[20];
  struct in_addr addr;

  struct ipoque_id_struct *src = NULL;
  struct ipoque_id_struct *dst = NULL;
  struct osdpi_flow *flow = NULL;
  struct ipoque_flow_struct *ipq_flow = NULL;
  u32 protocol = 0;
  
  if(extract_headers(&hdr,(uint8_t *)packet, pkthdr->caplen) == 0) {
    //printf("Failed to parse header information\n");
    return;
  }

  addr.s_addr=hdr.ip->saddr;
  strcpy(src_ip, (char *)inet_ntoa(addr));
  addr.s_addr=hdr.ip->daddr;
  strcpy(dst_ip, (char *)inet_ntoa(addr));

  src = get_id((u8 *) & hdr.ip->saddr);
  dst = get_id((u8 *) & hdr.ip->daddr);

  flow = get_osdpi_flow(&hdr, pkthdr->caplen - ETHER_HDR_LEN, pkthdr->ts.tv_sec);

  if (flow != NULL) {
    ipq_flow = flow->ipoque_flow;
  }

  if ((hdr.ip->frag_off & htons(0x1FFF)) == 0) {
    uint64_t time =((((uint64_t) pkthdr->ts.tv_sec)*1000) + pkthdr->ts.tv_usec/1000);
    // here the actual detection is performed
    protocol = ipoque_detection_process_packet(obj_cfg.ipoque_struct, ipq_flow, (uint8_t *) hdr.ip, 
					       pkthdr->caplen - ETHER_HDR_LEN, time, src, dst);
    
    if(hdr.ip->protocol == 6) //TCP packet
      printf("New packet received: %s:%d-%s:%d-TCP >>> %s\n", src_ip, ntohs(hdr.tcp->source), 
	     dst_ip,ntohs(hdr.tcp->dest),protocol_long_str[protocol]);
    else
      printf("New packet received: %s:%d-%s:%d-UDP >>>> %s\n", src_ip, ntohs(hdr.udp->source), 
	     dst_ip, ntohs(hdr.udp->dest),protocol_long_str[protocol]);
  } else {
    static u8 frag_warning_used = 0;
    if (frag_warning_used == 0) {
      printf("\n\nWARNING: fragmented ip packets are not supported and will be skipped \n\n");
      sleep(2);
      frag_warning_used = 1;
    }
    return;
  }
}


/*
 * Initialize the configuration structure
 */
void
init_cfg() {
  obj_cfg.verbose = 0;
  obj_cfg.type = DEVICE_CAPTURE;
  strcpy(obj_cfg.dev_name, "eth0");
  strcpy(obj_cfg.pcap_filter, "udp or tcp");
  obj_cfg.ipoque_struct= NULL;

  hi_init_str(&obj_cfg.hi_handle_ip, 93563);
  hi_init_str(&obj_cfg.hi_handle_flows, 93563);
};

/*
 * Parse the command line parameters and modify appropriately the config object
 */
int 
parse_options(int argc, char *argv[]) {
  int i, j, c;
  //intiliaze program configuration object
  init_cfg();

  
  strcpy(obj_cfg.target, HWDB_SERVER_ADDR);
  obj_cfg.port = HWDB_SERVER_PORT;

  while ((c = getopt (argc, argv, "f:r:i:v")) != -1) {
    switch (c) {
    case 'f':
      strcpy(obj_cfg.pcap_filter, optarg);
      break;
    case 'r':
      strcpy(obj_cfg.filename, optarg);
      obj_cfg.type = FILE_CAPTURE;
      break;
    case 'i':
      strcpy(obj_cfg.dev_name, optarg);
      obj_cfg.type = DEVICE_CAPTURE;
      break;
    case 'v':
      obj_cfg.verbose = 1;
      break;
    case 'h':
      printf("usage: %s\n", USAGE);
      exit(0);
    default:
      printf("unknown param -%c. \n usage: %s\n", c, USAGE);
      exit(0);
    } 
  }
}


static void 
*malloc_wrapper(unsigned long size) {
  return malloc(size);
}

static void 
debug_printf(u32 protocol, void *id_struct, ipq_log_level_t log_level, 
			 const char *format, ...) {
#ifdef IPOQUE_ENABLE_DEBUG_MESSAGES
  if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(debug_messages_bitmask, protocol) != 0) {
    const char *protocol_string;
    const char *file;
    const char *func;
    u32 line;
    va_list ap;
    va_start(ap, format);
    
    protocol_string = prot_short_str[protocol];
    
    ipoque_debug_get_last_log_function_line(ipoque_struct, &file, &func, &line);
    
    printf("\nDEBUG: %s:%s:%u Prot: %s, level: %u packet: %llu :", file, func, line, 
	   protocol_string, log_level, raw_packet_count);
    vprintf(format, ap);
    va_end(ap);
  }
#endif
}

/*
 * Initialize pcap structures and rpc structures
 */
int
init() {  
  char errbuf[PCAP_ERRBUF_SIZE];
  struct bpf_program fp;      /* hold compiled program     */
  char *host = HWDB_SERVER_ADDR;
  unsigned short port = HWDB_SERVER_PORT;
  IPOQUE_PROTOCOL_BITMASK all;
  uint32_t size_id_struct; 
  uint32_t size_flow_struct;
  uint32_t i;

  /* ask pcap for the network address and mask of the device */
  if(obj_cfg.type == DEVICE_CAPTURE) {
    if (obj_cfg.verbose) 
      printf("Device is %s\n", obj_cfg.dev_name);
    //pcap_lookupnet(dev, &netp, &maskp, errbuf);
    
    /* open device for reading. NOTE: defaulting to
     * promiscuous mode*/
    obj_cfg.pcap_dev = pcap_open_live(obj_cfg.dev_name, BUFSIZ, 1, -1, errbuf);
    if(obj_cfg.pcap_dev == NULL) {
      fprintf(stderr, "pcap_open_live(): %s\n", errbuf);
      exit(1);
    }
  } else if (obj_cfg.type == FILE_CAPTURE) {
    if (obj_cfg.verbose) 
      printf("File is %s\n", obj_cfg.filename);
    obj_cfg.pcap_dev = pcap_open_offline(obj_cfg.filename, errbuf);
    if(obj_cfg.pcap_dev == NULL) {
      fprintf(stderr, "pcap_open_live(): %s\n", errbuf);
      exit(1);
    }
  } else {
      fprintf(stderr, "No device or file was defined for capturing\n");
      exit(1);
  }
/*   // For radiotap, datalink should be DLT_IEEE802_11_RADIO.  */
/*   // Otherwise, exit. */
/*   if( pcap_datalink(descr) != DLT_IEEE802_11_RADIO) { */
/*     if (pcap_set_datalink (descr, DLT_IEEE802_11_RADIO) == -1) { */
/*       pcap_perror(descr, "Error"); */
/*       exit(1); */
/*     } */
/*   } */

  if(strlen(obj_cfg.pcap_filter) > 0) {
    /* Lets try and compile the program.. non-optimized */
    if(pcap_compile(obj_cfg.pcap_dev, &fp, obj_cfg.pcap_filter, 0, 0) == -1) {
      fprintf(stderr, "Error calling pcap_compile\n");
      exit(1);
    }
    
    /* set the compiled program as the filter */
    if(pcap_setfilter(obj_cfg.pcap_dev, &fp) == -1) {
      fprintf(stderr, "Error setting filter\n");
      exit(1);
    }
  }

  //rpc initialization
  if (! rpc_init(0)) {
    fprintf(stderr, "Initialization failure for rpc system\n");
    exit(-1);
  }
  obj_cfg.rpc = rpc_connect(host, port, "HWDB", 1l);
  if (obj_cfg.rpc == NULL) {
    fprintf(stderr, "Error connecting to HWDB at %s:%05u\n", 
	    host, port);
    exit(-1);
  }
  
  // init global opendpi structure with millisecond precision
  obj_cfg.ipoque_struct = ipoque_init_detection_module(1000, 
					       malloc_wrapper, debug_printf);
  if (obj_cfg.ipoque_struct == NULL) {
    printf("ERROR: global structure initialization failed\n");
    exit(-1);
  }
  // enable all protocols
  IPOQUE_BITMASK_SET_ALL(all);
  ipoque_set_protocol_detection_bitmask2(obj_cfg.ipoque_struct, &all);
}

int
main(int argc, char *argv[]) {	
  char *dev;
  char errbuf[PCAP_ERRBUF_SIZE];
  bpf_u_int32 maskp;
  bpf_u_int32 netp;
  u_char* args = NULL;
  pthread_t thr;
  int i;
  char buf[100];

  // Initialize link accumulator.
  //  lt_init();
  parse_options(argc, argv);
  init();

  // and the thread that processes (inserts into hwdb) the accumulated results.
/*   if (pthread_create(&thr, NULL, handler, NULL)) { */
/*     fprintf(stderr, "Failure to start database thread\n"); */
/*     exit(1); */
/*   } */
  /* ... and loop */ 
  pcap_loop(obj_cfg.pcap_dev, -1, process_packet, args);
  fprintf(stderr, "\nfinished\n");
  pcap_close(obj_cfg.pcap_dev);


  printf("Starting hwdb-opendpi daemon...\n");
  return 0;
}
