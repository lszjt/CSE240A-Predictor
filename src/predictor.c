//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include <stdlib.h>
#include "predictor.h"
#include <math.h>

//
// TODO:Student Information
//
const char *studentName = "Juntao Zhu";
const char *studentID   = "A53209209";
const char *email       = "juz088@ucsd.edu";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = { "Static", "Gshare",
                          "Tournament", "Custom" };

int ghistoryBits; // Number of bits used for Global History
int lhistoryBits; // Number of bits used for Local History
int pcIndexBits;  // Number of bits used for PC index
int bpType;       // Branch Prediction Type
int verbose;

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
//TODO: Add your own Branch Predictor data structures here
//

int *BHT;
int History;

//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//
void
init_predictor()
{
  //
  //TODO: Initialize Branch Predictor Data Structures
  //
  History = 0;
  size_t size = pow(2, ghistoryBits);
  BHT = (int *)malloc(size * sizeof(uint32_t));
  for(int i = 0; i < size; i++) {
    *(BHT + i) = WN;
  }
  
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t
GSHARE_make(uint32_t pc) {
  int mask = 0;
  for(int i = 0; i < ghistoryBits; i++) {
    mask =  (mask << 1) + 1;
  }
  uint32_t index = (pc & mask) ^ (History & mask);
  if(*(BHT + index) >= WT) {
    return TAKEN;
  }
  else {
    return NOTTAKEN;
  }
}

uint8_t
make_prediction(uint32_t pc)
{
  //
  //TODO: Implement prediction scheme
  //
  // Make a prediction based on the bpType
  switch (bpType) {
    case STATIC:
      return TAKEN;
    case GSHARE:
      return GSHARE_make(pc);
    case TOURNAMENT:
    case CUSTOM:
    default:
      break;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}



// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//
void
GSHARE_train(uint32_t pc, uint8_t outcome) {
  uint32_t mask = 0;
  for(int i = 0; i < ghistoryBits; i++) {
    mask =  (mask << 1) + 1;
  }
  uint32_t index = (pc & mask) ^ (History & mask);
  if(outcome == TAKEN) {
    if(*(BHT + index) < ST) {
      *(BHT + index) += 1;
    }
  }
  else {
    if(*(BHT + index) > SN) {
      *(BHT + index) -= 1;
    }
  }
  return;
}

void
train_predictor(uint32_t pc, uint8_t outcome)
{
  //
  //TODO: Implement Predictor training
  //
  switch (bpType) {
    case STATIC:
      break;
    case GSHARE:
      GSHARE_train(pc, outcome);
      break;
    case TOURNAMENT:
    case CUSTOM:
    default:
      break;
  }
  return;
}




