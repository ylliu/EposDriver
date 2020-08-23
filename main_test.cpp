#include "EposDriver.h"
#include <unistd.h>
int main(){
	//config demo
	std::pair<string, unsigned int> port_map_node_id[3];
	port_map_node_id[0].first = "USB0";
	port_map_node_id[0].second = 1;

	port_map_node_id[1].first = "USB1";
	port_map_node_id[1].second = 2;

	port_map_node_id[2].first = "USB2";
	port_map_node_id[2].second = 3;
	//init epos demo
	EposDriver* epos[3];
	for (int i = 0; i < 3; i++){
		epos[i] = new EposDriver(port_map_node_id[i].first, port_map_node_id[i].second);
		if (MMC_SUCCESS != epos[i]->InitDevice())
		{
			std::cout << "Init epos device failed,usb port name:"
				<< port_map_node_id[i].first << " node_id:" << port_map_node_id[i].second << std::endl;
		}
		else
			std::cout << "Init successed,,usb port name:"
			    << port_map_node_id[i].first << " node_id:" << port_map_node_id[i].second << std::endl;
	}
	//current mode demo
	for (int i = 0; i < 3; i++){
		epos[i]->DemoCurrentMode(100,200);
	}
	sleep(2);
	for (int i = 0; i < 3; i++){
		epos[i]->DemoCurrentMode(100,300);
	}
	sleep(2);
	for (int i = 0; i < 3; i++){
		epos[i]->DemoStopCurrentMode();
	}

	return 0;
}
