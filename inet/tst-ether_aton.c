#include <netinet/ether.h>

int
main (int argc, char *argv[])
{
  struct ether_addr *val;
  int result;

  val = ether_aton ("12:34:56:78:9a:bc");

  printf ("ether_aton (\"12:34:56:78:9a:bc\") = %hhx:%hhx:%hhx:%hhx:%hhx:%hhx\n",
	  val->ether_addr_octet[0],
	  val->ether_addr_octet[1],
	  val->ether_addr_octet[2],
	  val->ether_addr_octet[3],
	  val->ether_addr_octet[4],
	  val->ether_addr_octet[5]);


  result = (val->ether_addr_octet[0] != 0x12
	    || val->ether_addr_octet[1] != 0x34
	    || val->ether_addr_octet[2] != 0x56
	    || val->ether_addr_octet[3] != 0x78
	    || val->ether_addr_octet[4] != 0x9a
	    || val->ether_addr_octet[5] != 0xbc);

  return result;
}
