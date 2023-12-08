#include <stdio.h>
#include <ostream>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <cstdint>
#include "mbr.h"

using namespace std;

void getPartAdd(FILE* openFile, int partitionNumber){
  DISK_mbr mbr;
  fread(&mbr,sizeof(DISK_mbr),1,openFile);
  printf("What's before Fslba: 0x%08x%x%08x", mbr.pt[0].end_chs[0], mbr.pt[0].end_chs[1], mbr.pt[0].end_chs[2]);
  printf("whats in mbr first_sector_lba 0x%08x\n", mbr.pt[0].first_sector_lba);
  printf("The partition starts at address 0x%08x\n", mbr.pt[0].first_sector_lba * 512);

}

int main (int argc, char *argv[]) {
  char* toolName;
  char* fileName;
  FILE *ptr_file;
  int partitionNumber;
  int openFile, sz;
  byte* bufferOutput = (byte*)calloc(100, sizeof(byte));

  for(int i = 0; i < argc; i++){
    cout << argv[i] << endl;
  }
  ptr_file = fopen(argv[1], "r");
  if(!ptr_file){
    printf("Unable to open file");
    return 1;
  }
  getPartAdd(ptr_file, atoi(argv[2]));

  fclose(ptr_file);

  return 0;
}
