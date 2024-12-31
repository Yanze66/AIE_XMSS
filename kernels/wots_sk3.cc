#include "../kernels.h"
#include <cstdint>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <adf.h>
#include <aie_api/aie.hpp>
#include "aie_api/aie_types.hpp"

#define len 96 //len of hash function

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

// this function insert int into char
inline void uint_to_char(unsigned char * __restrict output, uint32_t  input){
    
    for(int i=0;i<4;i++){
        output[3-i]= input & 0xff;
        input = input >>8;        
    }
    
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


//GMIO is suppoused to transfer the data in multiple of 32bit, or it renders deadlock when reading.
// input: pk_seed || sk_seed 
// output: wots sk seed (67*256-bit) 
// bufout:wots_sk | pub_seed ; addr_out
// add a parameter run_times, by which you can continually calculate the leaf nodes
void wots_sk_gen3(input_stream<uint32> * __restrict bufin, output_stream<uint32>* __restrict bufout, output_stream<uint32>* __restrict addr_out)
{
    


   
    //package the sha256. we fixed the length of input to be 768-bit (96-B) according to XMSS thash_f

    int r = (int)((len * 8) & 0x1ff); //rest number of data  // original is mod 512, here change to bit operate
    int append = ((r < 448) ? (448 - r) : (448 + 512 - r)) / 8;    
    size_t new_len = len + append + 8;//  original+padding+64 bit-length 
    
    unsigned char buf[new_len] ;//; 
    
    memset(buf ,0,new_len); //reset to zero
    

    uint32_t temp[24]={0}; //pub_seed | sk_seed | addr
   
    for(int j=0;j<16;j++)//
    
        chess_prepare_for_pipelining
        chess_loop_range(2, )
        {  
        
           temp[j]=readincr(bufin);  // readincr converses here: ABCD -> DCBA  //wyz add in 2024.2.23   	          
           
    }
    
    buf[31]= 0x03; // prf padding (0x00_00_00_00....00_03)

    if (len > 0) {
        memcpy(buf+32, temp+8, 64); //   insert sk_seed  | addr .readincr is conversed,but corrected here : DCBA -> ABCD                                                                 
    }
   
    buf[len] = (unsigned char)0x80;

    

    uint64_t bits_len = len * 8; //wyz change  
    for (int i = 0; i < 8; i++) 
    chess_prepare_for_pipelining
    chess_loop_range(8, 8)
    {
        buf[len + append + i] = (bits_len >> ((7 - i) * 8)) & 0xff;
    }
    
    

    /******************************************************************/
    //above process input data, read and package 
    /******************************************************************/
    //following process hash function, compress and write back
    /******************************************************************/
    uint32_t iteration =0;
    while(iteration<256){ //repeat 256 times, obtain 256* 67 wots_sk nodes ; 256*67*32B
        
        temp[20] = swap32(iteration*4 +3 ); //WOTS ADDRESS （0,4,8,...,1020）+3
        temp[21] = 0; // reset the chain address
        if (len > 0) {
        memcpy(buf+32, temp+8, 64); //   insert sk_seed  | addr .                                                                 
        }
        int run_num =0;
            
        while(run_num <68){ //one more hash function to obtain seed from secret seed 
             
      
            uint32_t h0 = 0x6a09e667;
            uint32_t h1 = 0xbb67ae85;
            uint32_t h2 = 0x3c6ef372;
            uint32_t h3 = 0xa54ff53a;
            uint32_t h4 = 0x510e527f;
            uint32_t h5 = 0x9b05688c;
            uint32_t h6 = 0x1f83d9ab;
            uint32_t h7 = 0x5be0cd19;

            uint32_t w[64];
            uint32_t temp_w[2];
            
   
            size_t chunk_len = new_len / 64; //divided by 512-bit
       
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


            if(run_num ==0){ //generate  a seed for according to WOTS address
                uint_to_char(buf+32,h0);
                uint_to_char(buf+36,h1);
                uint_to_char(buf+40,h2);
                uint_to_char(buf+44,h3);
                uint_to_char(buf+48,h4);
                uint_to_char(buf+52,h5);
                uint_to_char(buf+56,h6);
                uint_to_char(buf+60,h7);
                buf[83]=0; //change wots addr to 0 and expand wots_sk
                buf[82]=0;
                buf[81]=0;
                buf[80]=0;
            }else{//run number 1-67
    

                //Reverse happens when transfer data from AIE to PS through DDR, thats,  ABCD -> DCBA， so I reverse it before sending
                writeincr(bufout, swap32(h0));
                writeincr(bufout, swap32(h1));
                writeincr(bufout, swap32(h2));
                writeincr(bufout, swap32(h3));
                writeincr(bufout, swap32(h4));
                writeincr(bufout, swap32(h5));
                writeincr(bufout, swap32(h6));
                writeincr(bufout, swap32(h7));
            
                //output pub_seed
                for(int i=0;i<8;i++){
                    writeincr(bufout, temp[i]);
                }
                //output addr
        
                for(int i=16;i<24;i++){
                    writeincr(addr_out, temp[i]);
                }

                //update chain address and output，the information of WOTS address is in temp[20]
                temp[21]=(temp[21] & 0xff000000 )+ ((temp[21] +1) <<24);
                buf[95]+= 1; // ctr for seed expand
            }
        run_num += 1;
        
        } // while run_num. every 68 time generate a group of wots sk
        iteration += 1;
    }

}
