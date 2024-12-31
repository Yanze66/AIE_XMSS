#include <adf.h>
#include <algorithm>
#include <cstdio>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <fstream>
#include <cstdlib>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/time.h>
#include "adf/adf_api/XRTConfig.h"
#include "experimental/xrt_kernel.h"
#include "experimental/xrt_graph.h"
#include <chrono>
#include <ctime>
#include "data.h"
#include "adf/adf_api/XRTConfig.h"

using namespace adf; //add for profile


    //parameter n=32,w=16,h=10;
    // unsigned char secseed[32]={0x20,0x72,0xa1,0xa2,0x66,0xf2,0x36,0xc9,0x3b,0x46,0xdf,0xa9,0xce,0x86,0x8e,0x79,0x29,0x81,0xd0,0xd0,0xa0,0x47,0x81,0x74,0x46,0xcb,0x7c,0x58,0x69,0x8f,0xd2,0x33};
    // unsigned char pubseed[32]={0x8e,0x91,0x10,0x0e,0x6f,0x44,0x66,0x6f,0x95,0x0b,0x93,0xf6,0x8c,0x2d,0xb4,0x57,0x15,0x8f,0x3c,0xde,0x39,0x33,0x3e,0x27,0x68,0xc5,0x37,0x70,0x23,0x0e,0xa2,0xb3};

    // unsigned char root[32] = {0xf0,0x5e,0xd8,0x53,0x33,0x4b,0xef,0x10,0x2a,0xe7,0x0a,0x48,0x10,0xe6,0x32,0x9a,0x49,0x02,0xec,0x60,0x5f,0x2e,0xf1,0xd8,0xf3,0x7b,0x02,0x01,0x1b,0xcc,0xf8,0x25};
    // const int ITERATION=1;
    // const int BLOCK_SIZE_in_Bytes=ITERATION*32*2; //pubseed || sec_seed
    // const int BLOCK_SIZE_out_Bytes=ITERATION*2*32; //root
int main(int argc, char * argv[]) {
    printf("start host\n");
    // Create XRT device handle for ADF API
    char* xclbinFilename = argv[1];
	// Open xclbin
	auto device = xrt::device(0); //device index=0
    if(device == nullptr)
        
		throw std::runtime_error("No valid device handle found. Make sure using right xclOpen index.");
	auto xclbin_uuid = device.load_xclbin(xclbinFilename);

    /////for profile/////
    auto dhdl = xrtDeviceOpenFromXcl(device);
    adf::registerXRT(dhdl, xclbin_uuid.get());
    
    //////////////////////////////////////////
	// input memory
	// Allocating the input size of sizeIn to MM2S
	// MM2S module transfers input data from PL to the AI Engine
	//////////////////////////////////////////
		
	auto in_bohdl = xrt::bo(device, 8 * sizeof(uint32_t) * 2, 0, 0);
	auto in_bomapped = in_bohdl.map<uint32_t*>();
	memcpy(in_bomapped, uint32Input, 8 * sizeof(uint32_t) * 2);
	printf("Input memory virtual addr 0x%px\n", in_bomapped);

	in_bohdl.sync(XCL_BO_SYNC_BO_TO_DEVICE);

    //////////////////////////////////////////
	// output memory
	// Allocating the output size of sizeOut to S2MM
	// S2MM module receives the output data from AI Engine 
	//////////////////////////////////////////
	
	auto out_bohdl = xrt::bo(device, 16 * sizeof(uint32_t), 0, 0);
	auto out_bomapped = out_bohdl.map<uint32_t*>();
	memset(out_bomapped, 0xABCDEF00, 16 * sizeof(uint32_t));
	printf("Output memory virtual addr 0x%px\n", out_bomapped);

    ////////////////////////////////////////////////////////
	// mm2s ip - Creating kernel handle using xrt::kernel API
	///////////////////////////////////////////////////////	
	
	auto mm2s_khdl = xrt::kernel(device, xclbin_uuid, "mm2s");
	auto mm2s_rhdl = mm2s_khdl(in_bohdl, nullptr, 16);
	printf("run mm2s\n");

	////////////////////////////////////////////////////////
	// s2mm ip - Creating kernel handle using xrt::kernel API
	///////////////////////////////////////////////////////		
	
	auto s2mm_khdl = xrt::kernel(device, xclbin_uuid, "s2mm");
	auto s2mm_rhdl = s2mm_khdl(out_bohdl, nullptr, 16);
	printf("run s2mm\n");

    //////////////////////////////////////////
	// graph execution for AIE 
	//////////////////////////////////////////	
	
	//Obtains the graph handle from the XCLBIN that is loaded into the device
    auto cghdl = xrt::graph(device,xclbin_uuid,"mygraph");
	
    //start profile. its better to execute after graph run, to avoid extra consumption resulted by graph.run()
	//event::handle handle = event::start_profiling(cghdl.in, cghdl.Output, event::event::io_stream_start_difference_cycles);
    struct timeval start; 
    struct timeval end; 
	gettimeofday(&start, NULL );
	//Run th graph for 1 iteration	    
    cghdl.run(1);
	cghdl.wait();
	//Graph end
	cghdl.end();
	
    gettimeofday(&end, NULL );  
	long timeuse =1000000 * ( end.tv_sec - start.tv_sec ) + end.tv_usec - start.tv_usec;  
	printf("time=%f\n",timeuse /1000000.0); //us  
    //long long cycle_count = event::read_profiling(handle);
    //printf("Latency cycles=: %lld\n", cycle_count);
    //event::stop_profiling(handle);//Performance counter is released and cleared
    

    //////////////////////////////////////////
	// wait for mm2s done
	//////////////////////////////////////////	
	
	mm2s_rhdl.wait();

	//////////////////////////////////////////
	// wait for s2mm done
	//////////////////////////////////////////	
	
	s2mm_rhdl.wait();

	out_bohdl.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
	
	//////////////////////////////////////////
	// Comparing the execution data to the golden data
	//////////////////////////////////////////	

    int errorCount = 0;
    {
		for (int i = 0; i < 16; i++)
		{
				if (out_bomapped[i] != golden[i])
				{
					printf("Error found @ %d, %d != %d\n", i, out_bomapped[i], golden[i]);
					errorCount++;
				}
		}

		if (errorCount)
			printf("Test failed with %d errors\n", errorCount);
		else
			printf("TEST PASSED\n");
	}

    std::cout << "Releasing remaining XRT objects...\n";
	
	return errorCount;




   

};