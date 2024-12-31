
//#ifndef FUNCTION_KERNELS_H
//#define FUNCTION_KERNELS_H
#ifndef sha256_h
#define sha256_h

#include <adf.h>

//generate  4 wots-sk per group， ie， wots-sk 0,1,2,3 first； than jump to next group wots-sk 4,5,6,7
//input: pub_seed | sk_seed
void wots_sk_gen0(input_stream<uint32> * __restrict bufin, output_stream<uint32>* __restrict bufout, output_stream<uint32>* __restrict addr_out);
void wots_sk_gen1(input_stream<uint32> * __restrict bufin, output_stream<uint32>* __restrict bufout, output_stream<uint32>* __restrict addr_out);
void wots_sk_gen2(input_stream<uint32> * __restrict bufin, output_stream<uint32>* __restrict bufout, output_stream<uint32>* __restrict addr_out);
void wots_sk_gen3(input_stream<uint32> * __restrict bufin, output_stream<uint32>* __restrict bufout, output_stream<uint32>* __restrict addr_out);

//wots chain structure. 
//input: din | pub_seed 
//input: addr
//output: dout |pub_seed
//out: addr
void sha256(input_stream<uint32> *bufin,  output_stream<uint32>* bufout);
void thash_f(input_stream<uint32> *  data, input_stream<uint32> * addr,  output_stream<uint32>*  dout1, output_stream<uint32>*  dout2);
void sha256_mask(input_stream<uint32> * bufin,  output_stream<uint32>*  bufout);
void sha256_f(input_stream<uint32> *  prf_in, input_stream<uint32> * mask_in,  output_stream<uint32>*  dout, output_stream<uint32>*  addr_out);


//ltree structure, each layer process different number of node, and have different behaviour, so we need to specify every layer
//input: din | pubseed
//input: addr
//output: din | pub_seed
//output: addr
// node: Layer 6 do not output addr since stream ports are limited,only output din | pub_seed
void thash_h_0(input_stream<uint32> * __restrict data, input_stream<uint32> * __restrict addr, output_stream<uint32>* __restrict dout1, output_stream<uint32>* __restrict dout2);
void thash_h_0_prf(input_stream<uint32> * __restrict bufin, output_stream<uint32>* __restrict bufout);
void thash_h_0_mask2(input_stream<uint32> * __restrict bufin,   output_stream<uint32>* __restrict bufout);
void thash_h_0_mask1(input_stream<uint32> * __restrict bufin, input_stream<uint32> * __restrict prfin,  output_stream<uint32>* __restrict bufout,output_stream<uint32> * __restrict prfout);
void thash_h_0_final(input_stream<uint32> * __restrict mask1_in, input_stream<uint32> * __restrict mask2_in,  output_stream<uint32>* __restrict dout, output_stream<uint32>* __restrict addr_out);

void thash_h_1(input_stream<uint32> * __restrict data, input_stream<uint32> * __restrict addr,  output_stream<uint32>* __restrict dout1, output_stream<uint32>* __restrict dout2);
void thash_h_1_prf(input_stream<uint32> * __restrict bufin, output_stream<uint32>* __restrict bufout);
void thash_h_1_mask2(input_stream<uint32> * __restrict bufin,   output_stream<uint32>* __restrict bufout);
void thash_h_1_mask1(input_stream<uint32> * __restrict bufin, input_stream<uint32> * __restrict prfin,  output_stream<uint32>* __restrict bufout,output_stream<uint32> * __restrict prfout);
void thash_h_1_final(input_stream<uint32> * __restrict mask1_in, input_stream<uint32> * __restrict mask2_in,  output_stream<uint32>* __restrict dout, output_stream<uint32>* __restrict addr_out);
           
void thash_h_2(input_stream<uint32> * __restrict data, input_stream<uint32> * __restrict addr,  output_stream<uint32>* __restrict dout1, output_stream<uint32>* __restrict dout2);
void thash_h_2_prf(input_stream<uint32> * __restrict bufin, output_stream<uint32>* __restrict bufout);
void thash_h_2_mask2(input_stream<uint32> * __restrict bufin,   output_stream<uint32>* __restrict bufout);
void thash_h_2_mask1(input_stream<uint32> * __restrict bufin, input_stream<uint32> * __restrict prfin,  output_stream<uint32>* __restrict bufout,output_stream<uint32> * __restrict prfout);
void thash_h_2_final(input_stream<uint32> * __restrict mask1_in, input_stream<uint32> * __restrict mask2_in,  output_stream<uint32>* __restrict dout, output_stream<uint32>* __restrict addr_out);

void thash_h_3(input_stream<uint32> * __restrict data, input_stream<uint32> * __restrict addr,  output_stream<uint32>* __restrict dout1, output_stream<uint32>* __restrict dout2);
void thash_h_3_prf(input_stream<uint32> * __restrict bufin, output_stream<uint32>* __restrict bufout);
void thash_h_3_mask2(input_stream<uint32> * __restrict bufin,   output_stream<uint32>* __restrict bufout);
void thash_h_3_mask1(input_stream<uint32> * __restrict bufin, input_stream<uint32> * __restrict prfin,  output_stream<uint32>* __restrict bufout,output_stream<uint32> * __restrict prfout);
void thash_h_3_final(input_stream<uint32> * __restrict mask1_in, input_stream<uint32> * __restrict mask2_in,  output_stream<uint32>* __restrict dout, output_stream<uint32>* __restrict addr_out);

void thash_h_4(input_stream<uint32> * __restrict data, input_stream<uint32> * __restrict addr,  output_stream<uint32>* __restrict dout1, output_stream<uint32>* __restrict dout2);
void thash_h_4_prf(input_stream<uint32> * __restrict bufin, output_stream<uint32>* __restrict bufout);
void thash_h_4_mask2(input_stream<uint32> * __restrict bufin,   output_stream<uint32>* __restrict bufout);
void thash_h_4_mask1(input_stream<uint32> * __restrict bufin, input_stream<uint32> * __restrict prfin,  output_stream<uint32>* __restrict bufout,output_stream<uint32> * __restrict prfout);
void thash_h_4_final(input_stream<uint32> * __restrict mask1_in, input_stream<uint32> * __restrict mask2_in,  output_stream<uint32>* __restrict dout, output_stream<uint32>* __restrict addr_out);

void thash_h_5(input_stream<uint32> * __restrict data, input_stream<uint32> * __restrict addr,  output_stream<uint32>* __restrict dout1, output_stream<uint32>* __restrict dout2);
void thash_h_5_prf(input_stream<uint32> * __restrict bufin, output_stream<uint32>* __restrict bufout);
void thash_h_5_mask2(input_stream<uint32> * __restrict bufin,   output_stream<uint32>* __restrict bufout);
void thash_h_5_mask1(input_stream<uint32> * __restrict bufin, input_stream<uint32> * __restrict prfin,  output_stream<uint32>* __restrict bufout,output_stream<uint32> * __restrict prfout);
void thash_h_5_final(input_stream<uint32> * __restrict mask1_in, input_stream<uint32> * __restrict mask2_in,  output_stream<uint32>* __restrict dout, output_stream<uint32>* __restrict addr_out);

void thash_h_6(input_stream<uint32> * __restrict data, input_stream<uint32> * __restrict addr,  output_stream<uint32>* __restrict dout1, output_stream<uint32>* __restrict dout2);
void thash_h_6_prf(input_stream<uint32> * __restrict bufin, output_stream<uint32>* __restrict bufout);
void thash_h_6_mask2(input_stream<uint32> * __restrict bufin,   output_stream<uint32>* __restrict bufout);
void thash_h_6_mask1(input_stream<uint32> * __restrict bufin, input_stream<uint32> * __restrict prfin,  output_stream<uint32>* __restrict bufout,output_stream<uint32> * __restrict prfout);
void thash_h_6_final(input_stream<uint32> * __restrict mask1_in, input_stream<uint32> * __restrict mask2_in,  output_stream<uint32>* __restrict dout);

void merkel_00(input_stream<uint32> * __restrict data_in1,input_stream<uint32> * __restrict data_in2,  output_stream<uint32>* __restrict dout);
void merkel_01(input_stream<uint32> * __restrict data_in1,input_stream<uint32> * __restrict data_in2,  output_stream<uint32>* __restrict dout);
void merkel_1(input_stream<uint32> * __restrict data_in1,input_stream<uint32> * __restrict data_in2,  output_stream<uint32>* __restrict dout);
void merkel_2(input_stream<uint32> * __restrict data_in1, output_stream<uint32>* __restrict dout);
void merkel_3(input_stream<uint32> * __restrict data_in1, output_stream<uint32>* __restrict dout);
void merkel_4(input_stream<uint32> * __restrict data_in1, output_stream<uint32>* __restrict dout);
void merkel_5(input_stream<uint32> * __restrict data_in1, output_stream<uint32>* __restrict dout);
void merkel_6(input_stream<uint32> * __restrict data_in1, output_stream<uint32>* __restrict dout);
void merkel_7(input_stream<uint32> * __restrict data_in1, output_stream<uint32>* __restrict dout);
void merkel_8(input_stream<uint32> * __restrict data_in1, output_stream<uint32>* __restrict dout);
void merkel_9(input_stream<uint32> * __restrict data_in1, output_stream<uint32>* __restrict dout);
#endif
