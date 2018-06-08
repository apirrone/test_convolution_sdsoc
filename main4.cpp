#include <iostream>
#include <stdlib.h>
#include <stdint.h>
#include <fstream>
#include <string.h>
#include <unistd.h>
#include "main4.h"

#include "sds_lib.h"

class perf_counter
{
public:
     uint64_t tot, cnt, calls;
     perf_counter() : tot(0), cnt(0), calls(0) {};
     inline void reset() { tot = cnt = calls = 0; }
     inline void start() { cnt = sds_clock_counter(); calls++; };
     inline void stop() { tot += (sds_clock_counter() - cnt); };
     inline uint64_t avg_cpu_cycles() { return ((tot+(calls>>1)) / calls); };
};

int* readFile(char* file){

  if(access(file, F_OK))// File does not exist
     return 0;
  
  std::string line;
  std::ifstream myFile(file);
  
  int width = IM_W;
  int height = IM_H;
  
  if(myFile.is_open()){

    getline(myFile, line);

    int* input;
    input = (int*)malloc(width*sizeof(int));
    
    for(int i = 0 ; i < width*height ; i++){
      getline(myFile, line);
      input[i] = atoi(line.c_str());
    }
    
    return input;
    
  }
  else
    return 0;
  
}

void writeFile(int* input, int width, int height, char* filename){

  std::ofstream myFile(filename);
  if(myFile.is_open()){
    myFile << width << " " << height << "\n";
    for(int i = 0 ; i < width*height ; i++)
	myFile << input[i] << "\n";

    myFile.close();    
  }
  else
    std::cout << "error" << std::endl;
  
}

//TODO invert loops i/j and k/l
void sw_naiveConvolution(int width, int height, int border_width, int input[IM_W*IM_H], int conv[IM_W*IM_H]){

  for(int i = 0 ; i < width ; i++){
    int startK = (i-border_width) >= 0 ? (i-border_width) : 0;
    int stopK  = (i+border_width) < width ? (i+border_width) : width-1;

    for(int j = 0 ; j < height ; j++){
      int startL = (j-border_width) >= 0 ? (j-border_width) : 0;
      int stopL  = (j+border_width) < height ? (j+border_width) : height-1;

      int index = j*width+i;
      conv[index] = 0;
      
      //Computing convolution
      int cpt = 0;
      for(int k = startK ; k <= stopK ; k++){
  	for(int l = startL ; l <= stopL ; l++){
	  int indexConv = l*width+k;
	  
  	  conv[index] += input[indexConv]; // Memory access error
  	  cpt++;
  	}
      }
      conv[index] /= cpt;
    }
  }  
}

//TODO invert loops i/j and k/l
void hw_naiveConvolution(int width, int height, int border_width, int input[IM_W*IM_H]){

  int conv[IM_W*IM_H];
  
  for(int i = 0 ; i < width ; i++){
    int startK = (i-border_width) >= 0 ? (i-border_width) : 0;
    int stopK  = (i+border_width) < width ? (i+border_width) : width-1;

    for(int j = 0 ; j < height ; j++){
#pragma HLS PIPELINE
      int startL = (j-border_width) >= 0 ? (j-border_width) : 0;
      int stopL  = (j+border_width) < height ? (j+border_width) : height-1;

      int index = j*width+i;
      conv[index] = 0;
      
      //Computing convolution
      int cpt = 0;
      for(int k = startK ; k <= stopK ; k++){
  	for(int l = startL ; l <= stopL ; l++){
	  int indexConv = l*width+k;
  	  conv[index] += input[indexConv];
  	  cpt++;
  	}
      }
      conv[index] /= cpt;
    }
  }  
}

void naiveBenchmark(char* file){

  int* tmp_input = readFile(file);

  int input[IM_W*IM_H];

  memcpy(input, tmp_input, IM_W*IM_H*sizeof(int));
  
  const int conv_size = 3;
  const int border_width = (int)(conv_size/2);

  int conv[IM_W*IM_H];
  
  perf_counter hw_ctr, sw_ctr;

  for(int i = 0 ; i < NB_TESTS ; i++){
    sw_ctr.start();
    sw_naiveConvolution(IM_W, IM_H, border_width, input, conv);
    sw_ctr.stop();

    // writeFile(sw_conv, width, height, "sw_out.txt");
  
    hw_ctr.start();
    hw_naiveConvolution(IM_W, IM_H, border_width, input);
    hw_ctr.stop();

    // writeFile(hw_conv, width, height, "hw_out.txt");

    std::cout << i <<"/" << NB_TESTS << std::endl;
  }
  
  uint64_t sw_cycles = sw_ctr.avg_cpu_cycles();
  uint64_t hw_cycles = hw_ctr.avg_cpu_cycles();

  double speedup = (double) sw_cycles / (double) hw_cycles;

  std::cout << "Average number of CPU cycles running in software: "
  	    << sw_cycles << std::endl;
  std::cout << "Average number of CPU cycles running in hardware: "
  	    << hw_cycles << std::endl;
  std::cout << "Speed up: " << speedup << std::endl;
  
}

int main(int argc, char* argv[]){
  
  //Usage
  if(argc < 2){
    std::cout << "Usage : " << argv[0] << " <image>" << std::endl;
    return EXIT_FAILURE;
  }

  naiveBenchmark(argv[1]);

  // TODO other benchmarks
  
  return EXIT_SUCCESS;
}
