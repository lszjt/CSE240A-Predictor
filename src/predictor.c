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

int *BHT_global;
int History;//global history
int *PHT_map;//local history
int *PHT;
int *Choice;//choice predictor

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

  //initialize global history table
  History = 0;
  BHT_global = (int *)malloc((1 << ghistoryBits) * sizeof(int));
  for(int i = 0; i < (1 << ghistoryBits); i++) {
    *(BHT_global + i) = WN;
  }
  
  //initialize local history table
  PHT_map = (int *)malloc((1 << pcIndexBits) * sizeof(int));
  for(int i = 0; i < (1 << pcIndexBits); i++) {
    *(PHT_map + i) = 0;
  }
  PHT = (int *)malloc((1 << pcIndexBits) * sizeof(int));
  for(int i = 0; i < (1 << pcIndexBits); i++) {
    *(PHT + i) = WN;
  }

  Choice = (int *)malloc((1 << ghistoryBits) * sizeof(int));
  for(int i = 0; i < (1 << ghistoryBits); i++) {
    *(Choice + i) = WN;
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
  if(*(BHT_global + index) >= WT) {
    return TAKEN;
  }
  else {
    return NOTTAKEN;
  }
}

uint8_t
Local_make(uint32_t pc) {
  uint32_t mask = 0;
  for(int i = 0; i < pcIndexBits; i++) {
    mask =  (mask << 1) + 1;
  }
  uint32_t index = pc & mask;
  if(*(PHT + *(PHT_map + index)) >= WT) {
    return TAKEN;
  }
  else {
    return NOTTAKEN;
  }
}

uint8_t
Global_make() {
  uint32_t mask = 0;
  for(int i = 0; i < ghistoryBits; i++) {
    mask =  (mask << 1) + 1;
  }
  uint32_t index = History & mask;
  if(*(BHT_global + index) >= WT) {
    return TAKEN;
  }
  else {
    return NOTTAKEN;
  }
}

uint8_t
Choice_make() {
  uint32_t mask = 0;
  for(int i = 0; i < ghistoryBits; i++) {
    mask =  (mask << 1) + 1;
  }
  uint32_t index = History & mask;
  if(*(Choice + index) >= WT) {
    return 1;
  }
  else {
    return 0;
  }
}

uint8_t
TOURNAMENT_make(uint32_t pc) {
  uint8_t Local_result = Local_make(pc);
  uint8_t Global_result = Global_make();
  if(Choice_make()) {
    return Local_result;
  }
  else {
    return Global_result;
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
      return TOURNAMENT_make(pc);
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
    if(*(BHT_global + index) < ST) {
      *(BHT_global + index) += 1;
    }
  }
  else {
    if(*(BHT_global + index) > SN) {
      *(BHT_global + index) -= 1;
    }
  }
  History = (History << 1) + outcome;
  return;
}

void
Local_train(uint32_t pc, uint8_t outcome) {
  //train local BHT
  uint32_t mask = 0;
  for(int i = 0; i < pcIndexBits; i++) {
    mask =  (mask << 1) + 1;
  }
  uint32_t index = pc & mask;
  if(outcome == TAKEN) {
    if(*(PHT + *(PHT_map + index)) < ST) {
      *(PHT + *(PHT_map + index)) += 1;
    } 
  }
  else {
    if(*(PHT + *(PHT_map + index)) > SN) {
      *(PHT + *(PHT_map + index)) -= 1;
    }
  }
  //train PHT_map
  uint32_t mask2 = 0;
  for(int i = 0; i < lhistoryBits; i++) {
    mask2 =  (mask2 << 1) + 1;
  }
  *(PHT_map + index) = ((*(PHT_map + index) << 1) + outcome) & mask2;
  return;
}

void
Global_train(uint8_t outcome) {
  uint32_t mask = 0;
  for(int i = 0; i < ghistoryBits; i++) {
    mask =  (mask << 1) + 1;
  }
  uint32_t index = History & mask;
  if(outcome == TAKEN) {
    if(*(BHT_global + index) < ST) {
      *(BHT_global + index) += 1;
    } 
  }
  else {
    if(*(BHT_global + index) > SN) {
      *(BHT_global + index) -= 1;
    }
  }
  return;
}

void
Choice_train(uint32_t pc, uint8_t outcome) {
  uint8_t Local_result = Local_make(pc);
  uint8_t Global_result = Global_make();
  uint32_t mask = 0;
  for(int i = 0; i < ghistoryBits; i++) {
    mask =  (mask << 1) + 1;
  }
  uint32_t index = History & mask;
  if(Local_result != Global_result) {
    if(Local_result == outcome) {
      if(*(Choice + index) < ST) {
        *(Choice + index) += 1;
      }
    }
    else {
      if(*(Choice + index) > SN) {
        *(Choice + index) -= 1;
      }
    }
  }
  return;
}

void
TOURNAMENT_train(uint32_t pc, uint8_t outcome) {
  Choice_train(pc, outcome);
  Local_train(pc, outcome);
  Global_train(outcome);
  //at last train Global History register
  History = (History << 1) + outcome;
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
      TOURNAMENT_train(pc, outcome);
      break;
    case CUSTOM:
    default:
      break;
  }
  return;
}




