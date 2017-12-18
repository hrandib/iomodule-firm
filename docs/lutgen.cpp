#include "stdafx.h"
#include <array>
#include <vector>
#include <iostream>

using namespace std;

using array_t = array<uint32_t, 101>;

int32_t FillArray(array_t& arr, int32_t step, int32_t startValue, int32_t stepFrom = 0)
{
  int32_t val{startValue};
  int32_t increment{1};
  arr[0] = 0;
  for(size_t index{1}; index < arr.size(); ++index) {
    arr[index] = val;
    val += increment;
    if(index > stepFrom && !(index % step)) {
      ++increment;
    }
  }
  return arr.back();
}

int main()
{
  array_t arr;
  int32_t max, startVal;
  cout << "Enter desired max: ";
  cin >> max;
  cout << "Enter start value: ";
  cin >> startVal;
  int32_t step = 10;
  for(; step; --step) {
    if(FillArray(arr, step, startVal) > max) {
      break;
    }
  }
  int32_t stepFrom{0};
  for(; stepFrom < 30; ++stepFrom) {
    if(FillArray(arr, step, startVal, stepFrom) < max) {
      break;
    }
  }
  cout << "Step from: " << stepFrom << "  Step: " << step << "\r\n";
  for(const auto& val : arr) {
    cout << val;
    if(&val != &arr.back()) {
      cout << ", ";
    }
  }
  --stepFrom;
  FillArray(arr, step, startVal, stepFrom);
  cout << "\r\nStep from: " << stepFrom << "  Step: " << step << "\r\n";
  for(const auto& val : arr) {
    cout << val;
    if(&val != &arr.back()) {
      cout << ", ";
    }
  }
  cout << endl;

  return 0;
}

