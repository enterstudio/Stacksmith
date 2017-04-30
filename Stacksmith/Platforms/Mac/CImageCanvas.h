//
//  CImageCanvas.h
//  Stacksmith
//
//  Created by Uli Kusterer on 02/05/16.
//  Copyright © 2016 Uli Kusterer. All rights reserved.
//

#ifndef CImageCanvas_h
#define CImageCanvas_h

#include "CCanvas.h"
#include <string>


#if __OBJC__
@class NSImage;
typedef NSImage*		WILDNSImagePtr;
#else
typedef struct NSImage*	WILDNSImagePtr;
#endif

namespace Carlson {


class CImageCanvas : public CCanvas
{
public:
	CImageCanvas() : mImage(NULL) {}	//!< IsValid() returns false for a default-constructed image.
	CImageCanvas( const CSize& inSize, TCoordinate scaleFactor = 1.0 );			//!< Create an empty image of the given size.
	CImageCanvas( const std::string& inImageURL );	//!< Load the given image file into this canvas, making it the same size.
	CImageCanvas( CImageCanvas&& inOriginal );		//!< Move constructor for more efficient returning of images.
	explicit CImageCanvas(WILDNSImagePtr inImage);
	~CImageCanvas();
	
	virtual void	InitWithSize( const CSize& inSize, TCoordinate scaleFactor = 1.0 );	//!< Like CImageCanvas(CSize) for when you inited with the default constructor because you didn't know the size yet, or for resizing. Pass CSize(0,0) to free the actual image and get back to the way the default constructor leaves the image.
	virtual void	InitWithImageFileURL( const std::string& inImageURL );	//!< Like CImageCanvas(string) for when you inited with the default constructor because you didn't know the file path yet.
	
	virtual bool	IsValid() const	{ return mImage != NULL; }
	
	virtual CRect	GetRect() const;
	virtual CSize	GetSize() const;
	
	virtual void	BeginDrawing();
	virtual void	EndDrawing();
	
	CImageCanvas	Copy() const;

	virtual CImageCanvas GetImageForRect( const CRect& box );
	
	WILDNSImagePtr	GetMacImage() const	{ return mImage; }	// Only for use by platform-specific code to e.g. hand icons to OS controls.
	
	virtual CImageCanvas& operator =( const CImageCanvas& inOriginal );
	
protected:
	WILDNSImagePtr	mImage;

friend class CCanvas;
};


} /* namespace Carlson */

#endif /* CImageCanvas_h */
