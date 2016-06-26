#include <stdio.h>
#include <stdlib.h>
#include "client_shared.h"

int main(int argc, char *argv) {
	carRadio_sub st_sub_Interface;
	
		st_sub_Interface.uuid = "123";
	st_sub_Interface.server_ip = "10.1.6.56";
	st_sub_Interface.server_port = 1883;
	st_sub_Interface.keepalive = 10;
	st_sub_Interface.clean_seesion = true;
	st_sub_Interface.quiet = true;
	st_sub_Interface.topic = "1/1";
	st_sub_Interface.file_path = "/home";

	st_sub_Interface.product_id = "123";
	st_sub_Interface.product_type = 1;

	sub_init_connect(NULL, NULL, st_sub_Interface);
}
