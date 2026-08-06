#include "move.h"
#include <stdint.h>
#include <stdio.h>

uint64_t get_a_file_mask(int f) {
  uint64_t file = 0;
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      if (j == f - 1) {
        file |= 1ULL << (i * 8 + j);
      }
    }
  }
  return file;
}
uint64_t get_row_mask(int row) {
  uint64_t file = 0;
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      if (i == row - 1) {
        file |= 1ULL << (i * 8 + j);
        printf("placed\n");
      }
    }
  }
  return file;
}
int get_mask_with_length(int length) {
  int result = 0;
  for (int i = 0; i < length; i++) {
    result |= 1 << i;
  };
  return result;
}

int value_from(int value, int start, int length) {
  int shifted = value >> start;
  int mask = get_mask_with_length(length);
  return shifted & (mask);
}

void print_binary_grid(uint64_t value) {
  for (int i = 7; i > -1; i--) {
    for (int j = 0; j < 8; j++) {
      if (value & (1ULL << ((i * 8) + j))) {
        printf("1");
      } else {
        printf("0");
      }
    }
    printf("\n");
  }
}

int main(int argc, char *argv[]) {
  for (int i = 1; i < 9; i++) {
    uint64_t row = get_row_mask(i);
    uint64_t file = get_a_file_mask(i);
    printf("row: %d \n %llu\n", i, (unsigned long long)row);
    print_binary_grid(row);
    printf("file: %d \n %llu\n", i, (unsigned long long)file);
    print_binary_grid(file);
  }
  return 0;
}
