#include "../kernels.h"
#include <cstdint>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <adf.h>
#include <aie_api/aie.hpp>
#include "aie_api/aie_types.hpp"
#define len 96

#define rightrotate(w,n) ((w>>n) | (w)<< (32-(n)))
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define copy_uint32(p, val) *((uint32_t *)p) = __builtin_bswap32((val))//gcc 内建函数__builtin_bswap32，
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define copy_uint32(p, val) *((uint32_t *)p) = (val)
#else
#error "Unsupported target architecture endianess!"
#endif

static const uint32_t k[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};


//shift data byte order

inline uint32_t swap32(uint32_t datain){
    uint32_t dataout = 0;
    dataout = ((datain&0x000000FF) <<24) + ((datain&0x0000FF00) <<8) + ((datain&0x00FF0000)>>8)+ ((datain&0xff000000)>>24) ;
    return dataout;
}

//this function package uint_8 into uint32
inline uint32_t fill(unsigned char * __restrict input){
    uint32_t output=0;
    output = (*input<<24) | (*(input+1)<<16) | (*(input+2)<<8) | *(input+3);    
   return output;
}

//this function generate w[0] to w[15] in pipeline
inline uint32_t parafill(unsigned char * __restrict input, uint32_t *__restrict output){
      
        for(int i=0;i<16;i++)chess_prepare_for_pipelining{
            *(output+i)=fill(input+i*4);
        }
    return 0;
}

/*****************these four function highly pipelined and reduce generation cycle of w[16] ... w[63] from 1500 to 1239****************************************************/
inline uint32_t gen_s0(uint32_t * __restrict input){     
        uint32_t s0= rightrotate(*(input+1), 7) ^ rightrotate(*(input+1), 18) ^ (*(input+1) >> 3);
        return s0;
}
inline uint32_t gen_s1(uint32_t * __restrict input){      
       uint32_t s1= rightrotate(*(input+14), 17) ^ rightrotate(*(input+14), 19) ^ (*(input+14) >> 10);
    return s1;
}

inline uint32_t gen_w(uint32_t * __restrict input){           
        uint32_t w= *input + *(input+9) + gen_s0(input) +gen_s1(input);      
    return w;
}

inline uint32_t gen_2w(uint32_t * __restrict input,uint32_t * __restrict output){
        
        
        *(output) = gen_w(input);  //this paragraph takes 1239 cycles.
        *(output+1) = gen_w(input+1);
        *(input+16) = *output;
        *(input+17) = *(output+1);

        //*(input+16) = gen_w(input);  //this paragraph takes 1489 cycles
        //*(input+17) = gen_w(input+1);

        //according to the ug-1079,it seems previous paragraph the because in this way, we operate data in two buffer
    return 0;
}


/******************************************************************************************************************/




/*************************************************/

// GMIO is suppoused to transfer the data in multiple of 32bit, or it renders deadlock when reading.
void thash_h_6_prf(input_stream<uint32> * __restrict bufin, output_stream<uint32>* __restrict bufout)
{
    int iteration =0;
    while(iteration<256){
    int run_num =0;
    while(run_num <1){ // calculate 1 time prf 
        uint32_t h0 = 0x6a09e667;
        uint32_t h1 = 0xbb67ae85;
        uint32_t h2 = 0x3c6ef372;
        uint32_t h3 = 0xa54ff53a;
        uint32_t h4 = 0x510e527f;
        uint32_t h5 = 0x9b05688c;
        uint32_t h6 = 0x1f83d9ab;
        uint32_t h7 = 0x5be0cd19;

        //int len=96;
        

    //aie::tile tile=aie::tile::current();
    //unsigned long long time1=tile.cycles();
        

        int r = (int)((len * 8) & 0x1ff); //rest number of data  // original is mod 512, here change to bit operate
        int append = ((r < 448) ? (448 - r) : (448 + 512 - r)) / 8;    
        size_t new_len = len + append + 8;// original+padding+length 
        
        unsigned char buf[new_len];//; 
        
        memset(buf + len,0,append); //zero
        

        uint32_t temp[len/4]; //we read 32-bit in one cycle， while len infer the length of char
    
        for(int j=0;j<((len-1)/4)+1;j++)//I don't know why function ceil() doesn't work in AIE, so just manually extract the int. //wyz add in 2024.2.23
        
            chess_prepare_for_pipelining
            chess_loop_range(2, )
            {  
            
            temp[j]=readincr(bufin);  // important !! every instruction reads 32bit data, so DDR must instore the data in mltiple of 32-bit, or it causes deadlock!! //wyz add in 2024.2.23   	
            //printf("\ntemp=%02x\n",temp[j]);
            
        }
        

        if (len > 0) {
            memcpy(buf, temp, len); //     readincr is conversed,but corrected here : DCBA -> ABCD                                                            
        }
    
        buf[len] = (unsigned char)0x80;
        buf[95]= 0;//the mask should be 0

        uint64_t bits_len = len * 8; //wyz change  
        for (int i = 0; i < 8; i++) 
        chess_prepare_for_pipelining
        chess_loop_range(8, 8)
        {
            buf[len + append + i] = (bits_len >> ((7 - i) * 8)) & 0xff;
        }
        
        //printf("\nsha256 buf[]=");
        //for (int j=0;j<new_len;j++){
        //    printf("%02x",buf[j]);
        //}
        //printf("\n");

        /******************************************************************/
        //above process input data, read and package 
        /******************************************************************/
        //following process hash function, compress and write back
        /******************************************************************/
        uint32_t w[64];
        uint32_t temp_w[2];
        //memset(w ,0,64); //change bzero to memset
    
        size_t chunk_len = new_len / 64; //分512bit区块
        
        for (int idx = 0; idx < chunk_len; idx++) {
            
                
            parafill(buf+idx*64,w);  //  generate W[0]...W[15] with pipeline, pipeling insert declines cycles from 900 to 170,wyz add in 2024.2.26

            for (int i = 0; i < 48; i=i+2)chess_prepare_for_pipelining{ //this paragraph generate w[16]...w[63] with pile line, decline cycles from 1500 to 1239
                gen_2w(w+i,temp_w);           
            }
            
            
            
            // start mapping
        
            uint32_t a = h0;
            uint32_t b = h1;
            uint32_t c = h2;
            uint32_t d = h3;
            uint32_t e = h4;
            uint32_t f = h5;
            uint32_t g = h6;
            uint32_t h = h7;
                
            for (int i = 0; i < 64; i++) chess_prepare_for_pipelining
            
            {
                uint32_t s_1 = rightrotate(e, 6) ^ rightrotate(e, 11) ^ rightrotate(e, 25);
                uint32_t ch = (e & f) ^ (~e & g);
                uint32_t temp1 = h + s_1 + ch + k[i] + w[i];
                uint32_t s_0 = rightrotate(a, 2) ^ rightrotate(a, 13) ^ rightrotate(a, 22);
                uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
                uint32_t temp2 = s_0 + maj;
                
                h = g;
                g = f;
                f = e;
                e = d + temp1;
                d = c;
                c = b;
                b = a;
                a = temp1 + temp2;
                        
            }
            //printf("\na=%02x",a);
        
            h0 += a;
            h1 += b;
            h2 += c;
            h3 += d;
            h4 += e;
            h5 += f;
            h6 += g;
            h7 += h;
            
        }

        //printf("\nh0=%02x",h0);
        

        //unsigned long long time2=tile.cycles();
        //printf("\n&cycles=%lld",time2-time1);

        //printf("\nhash without mask=%02x\n",h0);
        
        //Reverse happens when transfer data from AIE to PS through DDR, thats,  ABCD -> DCBA， so I reverse it before sending
        writeincr(bufout, swap32(h0));
        writeincr(bufout, swap32(h1));
        writeincr(bufout, swap32(h2));
        writeincr(bufout, swap32(h3));
        writeincr(bufout, swap32(h4));
        writeincr(bufout, swap32(h5));
        writeincr(bufout, swap32(h6));
        writeincr(bufout, swap32(h7));

        run_num += 1;  
    }//while
    iteration +=1;
    }


}
