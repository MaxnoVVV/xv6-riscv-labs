//
// Created by maxim on 20.04.2025.
//
#include "kernel/types.h"
#include "user/user.h"
#include "kernel/procinfo.h"

void bufferPointsToZeroTest() {
  struct procinfo* p = 0;
  int k = 2;
  int result = ps_listinfo(p, k);
  if(result > 0) {
    printf("bufferPointsToZeroTest passed\n");
  } else {
    printf("bufferPointsToZeroTest failed\n");
  }
}

void smallLimTest() {
  struct procinfo p[1];
  int k = 0;
  int result = ps_listinfo(p, k);
  if(result == -1) {
    printf("smallLimTest passed\n");
  } else {
    printf("smallLimTest failed\n");
  }
}

void badAddressTest() {
  struct procinfo* p = (struct procinfo*) -1;
  int k = 2;
  int result = ps_listinfo(p, k);
  if(result == -2) {
    printf("badAddressTest passed\n");
  } else {
    printf("badAddressTest failed\n");
  }
}

void successTest() {
  struct procinfo p[64];
  int k = 64;
  int result = ps_listinfo(p, k);
  if(result > 0 && result < k) {
    printf("successTest passed\n");
  } else {
    printf("successTest failed\n");
  }
}

void main() {
    bufferPointsToZeroTest();
    smallLimTest();
    badAddressTest();
    successTest();
}

