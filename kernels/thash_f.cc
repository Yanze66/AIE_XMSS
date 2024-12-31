#include "../kernels.h"
#include <cstdint>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <adf.h>
#include <aie_api/aie.hpp>
#include "aie_api/aie_types.hpp"

const uint32_t prf[8] = {0,0,0,0,0,0,0,0x03000000};


//this function package uint_8 into uint32
inline uint32_t fill(unsigned char * __restrict input){
    uint32_t output=0;
    output = (*input<<24) | (*(input+1)<<16) | (*(input+2)<<8) | *(input+3);    
   return output;
}

//this function generate pubseed[0] to pubseed[7] in pipeline
inline uint32_t parafill(unsigned char * __restrict input, uint32_t *__restrict output){
      
        for(int i=0;i<8;i++)chess_prepare_for_pipelining{
            *(output+i)=fill(input+i*4);
        }
    return 0;
}
/*************************************************/


//GMIO is suppoused to transfer the data in multiple of 32bit, or it renders deadlock when reading.
void thash_f(input_stream<uint32> * __restrict data, input_stream<uint32> * __restrict addr, 
                /*unsigned char (&Pubseed)[32],*/ output_stream<uint32>* __restrict dout1, output_stream<uint32>* __restrict dout2)
{
    int iteration =0;
    while(iteration<256){
    
        int pk_num =0;
    while(pk_num <67){ // totally 67 cahin in a leaf
        uint32_t datain[8]; //256bit in
        uint32_t temp_addr[8]; // 256bit addr
        uint32_t temp_pub[8];
      
    
        for(int i=0;i<8;i++){
            datain[i] = readincr(data);
            temp_addr[i] = readincr(addr) ;       
        }

        for(int i=0;i<8;i++){
            temp_pub[i] = readincr(data);          
        }
        
    
        //parafill(Pubseed,temp_pub);
        //printf("\nthash_f_pubseedin\n");
        //for(int i=0;i<8;i++){
        //    printf("%02x",temp_pub[i]);
        //}
        //printf("\n");


        //printf("\nthash_f_datain\n");
        //for(int i=0;i<8;i++){
        //    printf("%02x",datain[i]);
        //}
        //printf("\n");

        //printf("\nthash_f_addr\n");
        //for(int i=0;i<8;i++){
        //   printf("%02x",temp_addr[i]);
        //}
        //printf("\n");

 
        for(int i=0;i<8;i++){
            writeincr(dout1, prf[i]); 
            writeincr(dout2, prf[i]);              
        }
    
        for(int i=0;i<8;i++){
            writeincr(dout1, temp_pub[i]);       
            writeincr(dout2, temp_pub[i]);
        }

    

        temp_addr[7]=0;

        for(int i=0;i<8;i++){
            writeincr(dout1, temp_addr[i]);       
        }
        temp_addr[7]=0x01000000;
        for(int i=0;i<8;i++){
            writeincr(dout2, temp_addr[i]);       
        }

        for(int i=0;i<8;i++){
            writeincr(dout2, datain[i]);       
        }

        pk_num += 1;
    }
    iteration += 1;
    } //iteration
}
