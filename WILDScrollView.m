//
//  WILDScrollView.m
//  Stacksmith
//
//  Created by Uli Kusterer on 09.05.10.
//  Copyright 2010 Uli Kusterer. All rights reserved.
//

#import "WILDScrollView.h"
#import "WILDPartView.h"


@implementation WILDScrollView

-(void)	resetCursorRects
{
	NSCursor	*	currentCursor = [WILDTools cursorForTool: [[WILDTools sharedTools] currentTool]];
	[self addCursorRect: [self bounds] cursor: currentCursor];
}

@end
