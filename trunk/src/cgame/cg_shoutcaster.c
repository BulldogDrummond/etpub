// cg_shoutcaster.c

#include "cg_local.h"

/*
================
CG_WorldToScreen

Take any world coord and convert it to a 2D virtual 640x480 screen coord
================
*/
qboolean CG_WorldToScreen( vec3_t point, float *x, float *y )
{
	vec3_t	trans;
	float	z, xc, yc, px, py;

	VectorSubtract( point, cg.refdef_current->vieworg, trans );
	z = DotProduct( trans, cg.refdef_current->viewaxis[0] );

	if( z <= .001f ) {
		return qfalse;
	}

	xc = 640.f / 2.f;
	yc = 480.f / 2.f;

	px = tan( cg.refdef_current->fov_x * M_PI / 360.f );
	py = tan( cg.refdef_current->fov_y * M_PI / 360.f );

	*x = xc - DotProduct( trans, cg.refdef_current->viewaxis[1] ) *
		xc / ( z * px );
	*y = yc - DotProduct( trans, cg.refdef_current->viewaxis[2] ) *
		yc / ( z * py );

	return qtrue;
}

/*
================
CG_PointIsVisible

Is point visible from camera viewpoint?
================
*/
qboolean CG_PointIsVisible( vec3_t point )
{
	trace_t	trace;

	CG_Trace( &trace, cg.refdef_current->vieworg, NULL, NULL,
		point, -1, CONTENTS_SOLID );

	if( trace.fraction < 1.f ) {
		return qfalse;
	}

	return qtrue;
}

/*
================
CG_AddFloatingString

FIXME: In some cases the name won't fade out - it suddenly disappears
       (depends on the viewing angle).
================
*/
void CG_AddFloatingString( centity_t *cent, qboolean isCounter )
{
	vec3_t				origin;
	qboolean			visible;
	float				x, y, dist, scale;
	floatingString_t	*string;
	char				*s;

	// don't add following player name to string list
	if( !isCounter &&
		cent->currentState.clientNum == cg.snap->ps.clientNum ) {
		return;
	}

	if( cg.floatingStringsCount >= MAX_FLOATING_STRINGS ) {
		return;
	}

	VectorCopy( cent->lerpOrigin, origin );

	if( !isCounter ) {
		origin[2] += 64;

		// even lower if needed
		if( cent->currentState.eFlags & EF_PRONE ||
			cent->currentState.eFlags & EF_DEAD ||
			cent->currentState.eFlags & EF_PLAYDEAD ) {
			origin[2] -= 45;
		}
	} else {
		origin[2] += 24;
	}

	visible = CG_PointIsVisible( origin );

	if( !visible &&
		cg.time - cent->floatingStringFadeTime > 1500 ) {
		return;
	}

	if( !CG_WorldToScreen( origin, &x, &y ) ) {
		return;
	}

	dist = VectorDistance( cent->lerpOrigin, cg.refdef_current->vieworg );
	scale = 2000.f / ( dist > 1500.f ? 1500.f : dist ) * .05f;

	if( !isCounter ) {
		s = cgs.clientinfo[cent->currentState.clientNum].name;
	} else {
		s = va( "%i",
			30 - ( cg.time - cent->currentState.effect1Time ) / 1000 );
	}

	// add the string to the list
	string = &cg.floatingStrings[cg.floatingStringsCount];
	string->string = s;
	string->x = x - CG_Text_Width_Ext( s, scale, 0, &cgs.media.font1 ) / 2.f;
	string->y = y;
	string->scale = scale;
	string->alpha = 1.f;

	if( visible ) {
		cent->floatingStringFadeTime = cg.time;
	} else {
		float diff = cg.time - cent->floatingStringFadeTime;

		if( diff > 500.f ) {
			string->alpha -= ( diff - 500.f ) / 1000.f;
		}
	}

	cg.floatingStringsCount++;
}

/*
================
CG_DrawFloatingStrings
================
*/
void CG_DrawFloatingStrings( void )
{
	int					i;
	floatingString_t	*string;
	vec4_t				color = { 1.f, 1.f, 1.f, 1.f };

	for( i = 0; i < cg.floatingStringsCount; i++ ) {
		string = &cg.floatingStrings[i];

		if( !string ) {
			break;
		}

		color[3] = string->alpha;

		CG_Text_Paint_Ext( string->x, string->y, string->scale, string->scale,
			color, string->string, 0, 0, 0, &cgs.media.font1 );

		memset( string, 0, sizeof( string ) );
	}

	cg.floatingStringsCount = 0;
}
