//
//  CPart.cpp
//  Stacksmith
//
//  Created by Uli Kusterer on 30.12.13.
//  Copyright (c) 2013 Uli Kusterer. All rights reserved.
//

#include "CPart.h"
#include "CTinyXMLUtils.h"
#include "CLayer.h"
#include "CStack.h"


using namespace Carlson;


static std::map<std::string,CPartCreatorBase*>	sPartCreators;


/*static*/ CPart*	CPart::NewPartWithElement( tinyxml2::XMLElement * inElement, CLayer *inOwner )
{
	std::string		partType;
	CTinyXMLUtils::GetStringNamed( inElement, "type", partType );
	CPart	*		thePart = NULL;

	auto	foundItem = sPartCreators.find( partType );
	if( foundItem != sPartCreators.end() )
	{
		thePart = foundItem->second->NewPartInOwner( inOwner );
		thePart->SetPartType( (*foundItem).second );	// Remember who created this part so we can compare its type.
	}
	else
	{
		thePart = new CPart( inOwner );
		fprintf( stderr, "error: Unknown part type %s, falling back on plain part.\n", partType.c_str() );
	}
	thePart->LoadFromElement( inElement );
	return thePart;
}


/*static*/ void		CPart::RegisterPartCreator( CPartCreatorBase* inCreator )
{
	sPartCreators[inCreator->GetPartTypeName()] = inCreator;
}


CPartCreatorBase*	CPart::GetPartCreatorForType( const char* inType )
{
	if( !inType )
		return NULL;
	auto	foundPartCreator = sPartCreators.find(inType);
	if( foundPartCreator == sPartCreators.end() )
		return NULL;
	return (*foundPartCreator).second;
}


CPart::CPart( CLayer *inOwner )
	: mFamily(0), mOwner(inOwner)
{
	mDocument = inOwner->GetDocument();
}


void	CPart::LoadFromElement( tinyxml2::XMLElement * inElement )
{
	LoadPropertiesFromElement( inElement );
	LoadUserPropertiesFromElement( inElement );
}


void	CPart::LoadPropertiesFromElement( tinyxml2::XMLElement * inElement )
{
	mID = CTinyXMLUtils::GetLongLongNamed( inElement, "id" );
	mName.erase();
	CTinyXMLUtils::GetStringNamed( inElement, "name", mName );
	mScript.erase();
	CTinyXMLUtils::GetStringNamed( inElement, "script", mScript );
	tinyxml2::XMLElement * rectElement = inElement->FirstChildElement( "rect" );
	mLeft = CTinyXMLUtils::GetLongLongNamed( rectElement, "left", 10LL );
	mTop = CTinyXMLUtils::GetLongLongNamed( rectElement, "top", 10LL );
	mRight = CTinyXMLUtils::GetLongLongNamed( rectElement, "right", mLeft + 100LL );
	mBottom = CTinyXMLUtils::GetLongLongNamed( rectElement, "bottom", mLeft + 100LL );
}


CScriptableObject*	CPart::GetParentObject()
{
	return mOwner;
}


CStack*		CPart::GetStack()
{
	return mOwner->GetStack();
}


void	CPart::DumpProperties( size_t inIndent )
{
	const char	*	indentStr = IndentString(inIndent);
	printf( "%srect = %lld,%lld,%lld,%lld\n", indentStr, mLeft, mTop, mRight, mBottom );
	
}


void	CPart::Dump( size_t inIndent )
{
	const char	*	indentStr = IndentString(inIndent);
	printf( "%s%s ID %lld \"%s\"\n%s{\n", indentStr, GetIdentityForDump(), mID, mName.c_str(), indentStr );
	DumpProperties( inIndent +1 );
	DumpUserProperties( inIndent +1 );
	CPartContents*	theContents = GetContentsOnCurrentCard();
	std::string		contents = theContents? theContents->GetText() : std::string();
	printf( "%s\tcontents = <<%s>>\n", indentStr, contents.c_str() );
	printf( "%s\tscript = <<%s>>\n", indentStr, mScript.c_str() );
	printf( "%s}\n", indentStr );
}


CPartContents*	CPart::GetContentsOnCurrentCard()
{
	CCard	*	currCard = GetStack()->GetCurrentCard();
	if( mOwner != currCard && !GetSharedText() )	// We're on the background layer, not on the card?
		return currCard->GetPartContentsByID( GetID(), (mOwner != currCard) );
	else
		return mOwner->GetPartContentsByID( GetID(), (mOwner != currCard) );
}


bool	CPart::GetTextContents( std::string& outString )
{
	CPartContents*	contents = GetContentsOnCurrentCard();
	if( contents )
		outString = contents->GetText();
	return true;
}


bool	CPart::SetTextContents( std::string inString )
{
	CPartContents*	contents = GetContentsOnCurrentCard();
	if( contents )
		contents->SetText( inString );
	else
	{
		CCard	*	currCard = GetStack()->GetCurrentCard();
		bool bgPartWithNonSharedText = (mOwner != currCard && !GetSharedText());
		contents = new CPartContents( bgPartWithNonSharedText ? currCard : mOwner );
		contents->SetID( mID );
		contents->SetText( inString );
		contents->SetIsOnBackground( (mOwner != currCard) );
		if( bgPartWithNonSharedText )	// We're on the background layer, not on the card? But we don't have shared text? Add the contents to the current *card*!
			currCard->AddPartContents( contents );
		else	// Otherwise, we're on the card, or on the background with shared text, add the contents to that.
			mOwner->AddPartContents( contents );
	}
	return true;
}



bool	CPart::GetPropertyNamed( const char* inPropertyName, size_t byteRangeStart, size_t byteRangeEnd, LEOContext* inContext, LEOValuePtr outValue )
{
	if( strcasecmp("name", inPropertyName) == 0 || strcasecmp("short name", inPropertyName) == 0 )
	{
		LEOInitStringValue( outValue, mName.c_str(), mName.size(), kLEOInvalidateReferences, inContext );
	}
	else if( strcasecmp("script", inPropertyName) == 0 )
	{
		LEOInitStringValue( outValue, mScript.c_str(), mScript.size(), kLEOInvalidateReferences, inContext );
	}
	else if( strcasecmp("id", inPropertyName) == 0 )
	{
		LEOInitIntegerValue( outValue, GetID(), kLEOUnitNone, kLEOInvalidateReferences, inContext );
	}
	else if( strcasecmp("rectangle", inPropertyName) == 0 || strcasecmp("rect", inPropertyName) == 0 )
	{
		LEOArrayEntry	*	theArray = NULL;
		LEOAddIntegerArrayEntryToRoot( &theArray, "left", mLeft, kLEOUnitNone, inContext );
		LEOAddIntegerArrayEntryToRoot( &theArray, "top", mTop, kLEOUnitNone, inContext );
		LEOAddIntegerArrayEntryToRoot( &theArray, "right", mRight, kLEOUnitNone, inContext );
		LEOAddIntegerArrayEntryToRoot( &theArray, "bottom", mBottom, kLEOUnitNone, inContext );
		LEOInitArrayValue( &outValue->array, theArray, kLEOInvalidateReferences, inContext );
	}
	else
		return CConcreteObject::GetPropertyNamed( inPropertyName, byteRangeStart, byteRangeEnd, inContext, outValue );
	return true;
}


bool	CPart::SetValueForPropertyNamed( LEOValuePtr inValue, LEOContext* inContext, const char* inPropertyName, size_t byteRangeStart, size_t byteRangeEnd )
{
	if( strcasecmp("name", inPropertyName) == 0 || strcasecmp("short name", inPropertyName) == 0 )
	{
		char		nameBuf[1024];
		const char*	nameStr = LEOGetValueAsString( inValue, nameBuf, sizeof(nameBuf), inContext );
		SetName( nameStr );
	}
	else if( strcasecmp("script", inPropertyName) == 0 )
	{
		char		scriptBuf[1024];
		const char*	scriptStr = LEOGetValueAsString( inValue, scriptBuf, sizeof(scriptBuf), inContext );
		SetScript( scriptStr );
	}
	else if( strcasecmp("script", inPropertyName) == 0 )
	{
		char		scriptBuf[1024];
		const char*	scriptStr = LEOGetValueAsString( inValue, scriptBuf, sizeof(scriptBuf), inContext );
		SetScript( scriptStr );
	}
	else if( strcasecmp("rectangle", inPropertyName) == 0 || strcasecmp("rect", inPropertyName) == 0 )
	{
		LEOValue		tempStorage = {};
		LEOUnit			theUnit = kLEOUnitNone;
		LEOInteger		l = 0, t = 0, r = 0, b = 0;
		LEOValuePtr		leftValue = LEOGetValueForKey( inValue, "left", &tempStorage, kLEOInvalidateReferences, inContext );
		if( leftValue )
			l = LEOGetValueAsInteger( leftValue, &theUnit, inContext );
		if( leftValue == &tempStorage )
			LEOCleanUpValue( &tempStorage, kLEOInvalidateReferences, inContext );
		LEOValuePtr		rightValue = LEOGetValueForKey( inValue, "right", &tempStorage, kLEOInvalidateReferences, inContext );
		if( rightValue )
			r = LEOGetValueAsInteger( rightValue, &theUnit, inContext );
		if( rightValue == &tempStorage )
			LEOCleanUpValue( &tempStorage, kLEOInvalidateReferences, inContext );
		LEOValuePtr		topValue = LEOGetValueForKey( inValue, "top", &tempStorage, kLEOInvalidateReferences, inContext );
		if( topValue )
			t = LEOGetValueAsInteger( topValue, &theUnit, inContext );
		if( topValue == &tempStorage )
			LEOCleanUpValue( &tempStorage, kLEOInvalidateReferences, inContext );
		LEOValuePtr		bottomValue = LEOGetValueForKey( inValue, "bottom", &tempStorage, kLEOInvalidateReferences, inContext );
		if( bottomValue )
			b = LEOGetValueAsInteger( bottomValue, &theUnit, inContext );
		if( bottomValue == &tempStorage )
			LEOCleanUpValue( &tempStorage, kLEOInvalidateReferences, inContext );
		SetRect( l, t, r, b );
	}
	else
		return CConcreteObject::SetValueForPropertyNamed( inValue, inContext, inPropertyName, byteRangeStart, byteRangeEnd );
	return true;
}
