//
//  ForgeHostFunctionsStacksmith.h
//  Stacksmith
//
//  Created by Uli Kusterer on 16.04.11.
//  Copyright 2011 Uli Kusterer. All rights reserved.
//

#include "LEOInstructions.h"
#include "ForgeTypes.h"


enum
{
	WILD_STACK_INSTRUCTION = 0,
	WILD_BACKGROUND_INSTRUCTION,
	WILD_CARD_INSTRUCTION,
	WILD_CARD_FIELD_INSTRUCTION,
	WILD_CARD_BUTTON_INSTRUCTION,
	WILD_CARD_MOVIEPLAYER_INSTRUCTION,
	WILD_CARD_PART_INSTRUCTION,
	WILD_BACKGROUND_FIELD_INSTRUCTION,
	WILD_BACKGROUND_BUTTON_INSTRUCTION,
	WILD_BACKGROUND_MOVIEPLAYER_INSTRUCTION,
	WILD_BACKGROUND_PART_INSTRUCTION,
	WILD_NEXT_CARD_INSTRUCTION,
	WILD_PREVIOUS_CARD_INSTRUCTION,
	WILD_FIRST_CARD_INSTRUCTION,
	WILD_LAST_CARD_INSTRUCTION,
	WILD_NEXT_BACKGROUND_INSTRUCTION,
	WILD_PREVIOUS_BACKGROUND_INSTRUCTION,
	WILD_PUSH_ORDINAL_BACKGROUND_INSTRUCTION,
	WILD_PUSH_ORDINAL_PART_INSTRUCTION,
	WILD_THIS_STACK_INSTRUCTION,
	WILD_THIS_BACKGROUND_INSTRUCTION,
	WILD_THIS_CARD_INSTRUCTION,
	
	WILD_NUMBER_OF_HOST_FUNCTION_INSTRUCTIONS
};


LEOINSTR_DECL(StacksmithHostFunction,WILD_NUMBER_OF_HOST_FUNCTION_INSTRUCTIONS)

extern size_t						kFirstStacksmithHostFunctionInstruction;


extern struct THostCommandEntry	gStacksmithHostFunctions[WILD_NUMBER_OF_HOST_FUNCTION_INSTRUCTIONS +1];
