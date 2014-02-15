//
//  WILDLayerInfoViewController
//  Stacksmith
//
//  Created by Uli Kusterer on 18.03.11.
//  Copyright 2011 Uli Kusterer. All rights reserved.
//

#import <Cocoa/Cocoa.h>

using namespace Carlson
{
	class CLayer;
}

@interface WILDLayerInfoViewController : NSViewController
{
	Carlson::CLayer	*	mLayer;
	
	NSTextField		*	mNameField;
	NSTextField		*	mNumberField;
	NSTextField		*	mIDField;
	NSTextField		*	mFieldCountField;
	NSTextField		*	mButtonCountField;
	NSButton		*	mEditScriptButton;
	NSButton		*	mDontSearchSwitch;
	NSButton		*	mCantDeleteSwitch;
	NSButton		*	mUserPropertyEditButton;
}

@property (assign) Carlson::CLayer			*	layer;

@property (retain) IBOutlet	NSButton		*	editScriptButton;
@property (retain) IBOutlet	NSButton		*	dontSearchSwitch;
@property (retain) IBOutlet	NSButton		*	cantDeleteSwitch;
@property (retain) IBOutlet	NSTextField		*	nameField;
@property (retain) IBOutlet	NSTextField		*	numberField;
@property (retain) IBOutlet	NSTextField		*	IDField;
@property (retain) IBOutlet	NSTextField		*	fieldCountField;
@property (retain) IBOutlet	NSTextField		*	buttonCountField;
@property (retain) IBOutlet	NSButton		*	userPropertyEditButton;

-(id)		initWithLayer: (Carlson::CLayer*)inCard;

-(IBAction)	doEditScriptButton: (id)sender;
-(IBAction)	doUserPropertyEditButton: (id)sender;

-(IBAction)	doCantDeleteSwitchChanged: (id)sender;
-(IBAction)	doDontSearchSwitchChanged: (id)sender;

@end
