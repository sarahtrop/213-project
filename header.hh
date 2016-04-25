#if !defined(CREATURE_HH)
#define CREATURE_HH

#include <cmath>

#define MAX_RADIUS 50
#define MIN_RADIUS 10

int binaryToInt(int arr[8]) {
  int result = 0;
  
  for(int i = 7; i >= 0; --i){
    if(arr[i] == 1) {
      result += pow(2, i);
    }
  }
  
  return result;
}

#endif
