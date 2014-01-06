//
//  CLayer.cpp
//  Stacksmith
//
//  Created by Uli Kusterer on 2013-12-29.
//  Copyright (c) 2013 Uli Kusterer. All rights reserved.
//

#include "CLayer.h"
#include "CURLConnection.h"
#include "CTinyXMLUtils.h"
#include "CPart.h"
#include "CPartContents.h"
#include "CStack.h"
#include "CButtonPart.h"
#include "CFieldPart.h"
#include "CRectanglePart.h"
#include "CPicturePart.h"


using namespace Carlson;


CLayer::~CLayer()
{
	
}


void	CLayer::SetStack( CStack* inStack )
{
	mStack = inStack;
	if( mStack )
		mDocument = inStack->GetDocument();
}

void	CLayer::Load( std::function<void(CLayer*)> completionBlock )
{
	if( mLoaded )
	{
		completionBlock( this );
		return;
	}
	
	Retain();
	
	mLoadCompletionBlocks.push_back(completionBlock);
	
	if( !mLoading )	// If we're already loading, we've queued up our completion block which gets called when the async load has finished.
	{
		mLoading = true;
		CURLRequest		request( mURL );
		CURLConnection::SendRequestWithCompletionHandler( request, [this] (CURLResponse inResponse, const char* inData, size_t inDataLength) -> void
		{
			tinyxml2::XMLDocument		document;

			if( tinyxml2::XML_SUCCESS == document.Parse( inData, inDataLength ) )
			{
				//document.Print();

				tinyxml2::XMLElement	*	root = document.RootElement();
				
				LoadPropertiesFromElement( root );
				
				LoadUserPropertiesFromElement( root );

				// Load parts:
				tinyxml2::XMLElement	*	currPartElem = root->FirstChildElement( "part" );
				while( currPartElem )
				{
					CPart	*	thePart = CPart::NewPartWithElement( currPartElem, this );
					thePart->Autorelease();
					mParts.push_back( thePart );
					thePart->Retain();	// Retain for the button families array.
					mButtonFamilies.insert( std::make_pair(thePart->GetFamily(), thePart) );

					currPartElem = currPartElem->NextSiblingElement( "part" );
				}

				// Load part contents:
				tinyxml2::XMLElement	*	currPartContentsElem = root->FirstChildElement( "content" );
				while( currPartContentsElem )
				{
					CPartContents	*	theContents = new CPartContents( currPartContentsElem );
					theContents->Autorelease();
					mContents.push_back( theContents );
					
					currPartContentsElem = currPartContentsElem->NextSiblingElement( "content" );
				}
				
				// Load AddColor info:
				LoadAddColorPartsFromElement( root );
			}
			
			CallAllCompletionBlocks();
		} );
	}
}


void	CLayer::CallAllCompletionBlocks()	// Can override this in cards to also load the background if needed and only *then* call completion blocks.
{
	mLoaded = true;
	mLoading = false;
	
	// Call all completion blocks:
	for( auto itty = mLoadCompletionBlocks.begin(); itty != mLoadCompletionBlocks.end(); itty++ )
		(*itty)(this);
	mLoadCompletionBlocks.clear();
			
	Release();
}


void	CLayer::LoadPropertiesFromElement( tinyxml2::XMLElement* root )
{
	// We get id and name from the TOC.xml via the constructor
	mShowPict = CTinyXMLUtils::GetBoolNamed( root, "showPict", true );
	mCantDelete = CTinyXMLUtils::GetBoolNamed( root, "cantDelete", false );
	mDontSearch = CTinyXMLUtils::GetBoolNamed( root, "dontSearch", false );
	mPictureName = "";
	CTinyXMLUtils::GetStringNamed( root, "bitmap", mPictureName );

	mScript.erase();
	CTinyXMLUtils::GetStringNamed( root, "script", mScript );
}


CPart*	CLayer::GetPartWithID( WILDObjectID inID )
{
	for( auto currPart = mParts.begin(); currPart != mParts.end(); currPart++ )
	{
		if( (*currPart)->GetID() == inID )
			return *currPart;
	}
	
	return NULL;
}


size_t	CLayer::GetPartCountOfType( CPartCreatorBase* inType )
{
	if( inType == NULL )
		return mParts.size();
	
	size_t	numParts = 0;
	for( auto currPart = mParts.begin(); currPart != mParts.end(); currPart++ )
	{
		if( (*currPart)->GetPartType() == inType )
			numParts++;
	}
	
	return numParts;
}


CPart*	CLayer::GetPartOfType( size_t inIndex, CPartCreatorBase* inType )
{
	if( inType == NULL )
	{
		if( inIndex >= mParts.size() )
			return NULL;
		return mParts[inIndex];
	}
	
	size_t	currIndex = 0;
	for( auto currPart = mParts.begin(); currPart != mParts.end(); currPart++ )
	{
		if( (*currPart)->GetPartType() == inType )
		{
			if( currIndex == inIndex )
				return *currPart;
			currIndex++;
		}
	}
	
	return NULL;
}


CPart*	CLayer::GetPartWithNameOfType( const std::string& inName, CPartCreatorBase* inType )
{
	size_t	currIndex = 0;
	for( auto currPart = mParts.begin(); currPart != mParts.end(); currPart++ )
	{
		if( inType == NULL || (*currPart)->GetPartType() == inType )
		{
			if( strcasecmp(inName.c_str(), (*currPart)->GetName().c_str()) == 0 )
				return *currPart;
			currIndex++;
		}
	}
	
	return NULL;
}


void	CLayer::LoadAddColorPartsFromElement( tinyxml2::XMLElement* root )
{
	tinyxml2::XMLElement	* theObject = root->FirstChildElement( "addcolorobject" );
	
	for( ; theObject != NULL; theObject = theObject->NextSiblingElement("addcolorobject") )
	{
		WILDObjectID	objectID = CTinyXMLUtils::GetLongLongNamed( theObject, "id" );
		int				objectBevel = CTinyXMLUtils::GetIntNamed( theObject, "bevel" );
		std::string		objectType;
		CTinyXMLUtils::GetStringNamed( theObject, "type", objectType );
		std::string		objectName;
		CTinyXMLUtils::GetStringNamed( theObject, "name", objectName );
		bool			objectTransparent = CTinyXMLUtils::GetBoolNamed( theObject, "transparent", false);
		bool			objectVisible = CTinyXMLUtils::GetBoolNamed( theObject, "visible", false);
		
		int				left = 0, top = 0, right = 100, bottom = 100;
		CTinyXMLUtils::GetRectNamed( theObject, "rect", &left, &top, &right, &bottom );
		int				red = 0, green = 0, blue = 0, alpha = 0;
		CTinyXMLUtils::GetColorNamed( theObject, "color", &red, &green, &blue, &alpha );
		
		if( objectType.compare("button") == 0 )
		{
			CButtonPart*	thePart = dynamic_cast<CButtonPart*>(GetPartWithID( objectID ));
			if( thePart )
			{
				thePart->SetFillColor( red, green, blue, alpha );
				thePart->SetBevelWidth( objectBevel );
				mAddColorParts.push_back( thePart );
			}
		}
		else if( objectType.compare("field") == 0 )
		{
			CFieldPart*	thePart = dynamic_cast<CFieldPart*>( GetPartWithID( objectID ) );
			if( thePart )
			{
				thePart->SetFillColor( red, green, blue, alpha );
				thePart->SetBevelWidth( objectBevel );
				mAddColorParts.push_back( thePart );
			}
		}
		else if( objectType.compare("rectangle") == 0 )
		{
			CRectanglePart*	thePart = new CRectanglePart( this );
			thePart->SetRect( left, top, right, bottom );
			thePart->SetFillColor( red, green, blue, alpha );
			thePart->SetBevelWidth( objectBevel );
			thePart->SetVisible( objectVisible );
			mAddColorParts.push_back( thePart );
		}
		else if( objectType.compare("picture") == 0 )
		{
			CPicturePart*	thePart = new CPicturePart( this );
			thePart->SetRect( left, top, right, bottom );
			thePart->SetMediaPath( objectName );
			thePart->SetTransparent( objectTransparent );
			thePart->SetVisible( objectVisible );
			mAddColorParts.push_back( thePart );
		}
	}
}


void	CLayer::WakeUp()
{
	for( auto currPart = mParts.begin(); currPart != mParts.end(); currPart++ )
	{
		(*currPart)->WakeUp();
	}
}


void	CLayer::GoToSleep()
{
	for( auto currPart = mParts.begin(); currPart != mParts.end(); currPart++ )
	{
		(*currPart)->GoToSleep();
	}
}


void    CLayer::AddPart( CPart* inPart )
{
    mParts.push_back( inPart );
}


const char*	CLayer::GetIdentityForDump()
{
	return "Layer";
}


void	CLayer::DumpProperties( size_t inIndent )
{
	const char	*	indentStr = IndentString(inIndent);
	printf( "%sloaded = %s\n", indentStr, mLoaded ? "true" : "false" );
}


void	CLayer::Dump( size_t inIndent )
{
	const char	*	indentStr = IndentString(inIndent);
	printf( "%s%s ID %lld \"%s\"\n%s{\n", indentStr, GetIdentityForDump(), mID, mName.c_str(), indentStr );
	DumpProperties( inIndent +1 );
	DumpUserProperties( inIndent +1 );
	printf( "%s\tscript = <<%s>>\n", indentStr, mScript.c_str() );
	printf("%s\tparts\n%s\t{\n",indentStr,indentStr);
	for( auto itty = mParts.begin(); itty != mParts.end(); itty++ )
		(*itty)->Dump( inIndent +2 );
	printf( "%s\t}\n%s\tcontents\n%s\t{\n", indentStr, indentStr, indentStr );
	for( auto itty = mContents.begin(); itty != mContents.end(); itty++ )
		(*itty)->Dump( inIndent +2 );
	printf( "%s\t}\n%s\taddcolor parts\n%s\t{\n", indentStr, indentStr, indentStr );
	for( auto itty = mAddColorParts.begin(); itty != mAddColorParts.end(); itty++ )
		(*itty)->Dump( inIndent +2 );
	printf( "%s\t}\n%s}\n", indentStr, indentStr );
}

