//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include "hl1_hud_numbers.h"


// This is a bad way to implement HL1 style sprite fonts, but it will work for now

CHL1HudNumbers::CHL1HudNumbers( vgui::Panel *parent, const char *name ) : BaseClass( parent, name )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );
}


void CHL1HudNumbers::VidInit( void )
{
	for ( int i = 0; i < 10; i++ )
	{
		// default
		char szNumString[ 14 ];

		sprintf( szNumString, "number_%d", i );
		icon_digits[ i ] = gHUD.GetIcon( szNumString );

		// ammo
		sprintf( szNumString, "ammo_number_%d", i );
		icon_ammo_digits[ i ] = gHUD.GetIcon( szNumString );
	}
}


int CHL1HudNumbers::GetNumberFontHeight( bool bAmmo )
{
	if ( bAmmo )
	{
		if ( icon_ammo_digits[ 0 ] )
		{
			return icon_ammo_digits[ 0 ]->Height();
		}
		else
		{
			return 0;
		}
	}
	else
	{
		if ( icon_digits[ 0 ] )
		{
			return icon_digits[ 0 ]->Height();
		}
		else
		{
			return 0;
		}
	}
}


int CHL1HudNumbers::GetNumberFontWidth( bool bAmmo )
{
	if ( bAmmo )
	{
		if ( icon_ammo_digits[ 0 ] )
		{
			return icon_ammo_digits[ 0 ]->Width();
		}
		else
		{
			return 0;
		}
	}
	else
	{
		if ( icon_digits[ 0 ] )
		{
			return icon_digits[ 0 ]->Width();
		}
		else
		{
			return 0;
		}
	}
}


int CHL1HudNumbers::DrawHudNumber( int x, int y, int iNumber, Color &clrDraw, bool bAmmo )
{
	//int iWidth = bAmmo ? GetNumberFontWidth( true ) : GetNumberFontWidth();
	int iWidth = bAmmo ? GetNumberFontWidth( true ) : GetNumberFontWidth();
	int k;

	CHudTexture *digit[ 10 ];
	//*digit = bAmmo ? *icon_ammo_digits : *icon_digits;
	memcpy( digit, bAmmo ? icon_ammo_digits : icon_digits, sizeof digit );

	if ( iNumber > 0 )
	{
		// SPR_Draw 100's
		if ( iNumber >= 100 )
		{
			k = iNumber / 100;
			digit[ k ]->DrawSelf( x, y, clrDraw );
			x += iWidth;
		}
		else
		{
			digit[ 0 ]->DrawSelf( x, y, clrDraw );
			x += iWidth;
		}

		// SPR_Draw 10's
		if ( iNumber >= 10 )
		{
			k = ( iNumber % 100 ) / 10;
			digit[ k ]->DrawSelf( x, y, clrDraw );
			x += iWidth;
		}
		else
		{
			digit[ 0 ]->DrawSelf( x, y, clrDraw );
			x += iWidth;
		}

		// SPR_Draw ones
		k = iNumber % 10;
		digit[ k ]->DrawSelf( x, y, clrDraw );
		x += iWidth;
	}
	else
	{
		//// SPR_Draw 100's
		//x += iWidth;

		//// SPR_Draw 10's
		//x += iWidth;

		//// SPR_Draw ones
		//k = 0;

		// Draw zeroes
		k = 0;

		digit[ k ]->DrawSelf( x, y, clrDraw );
		x += iWidth;

		digit[ k ]->DrawSelf( x, y, clrDraw );
		x += iWidth;

		digit[ k ]->DrawSelf( x, y, clrDraw );
		x += iWidth;
	}

	return x;
}
