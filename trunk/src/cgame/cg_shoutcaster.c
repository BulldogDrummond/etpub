// cg_shoutcaster.c

#include "cg_local.h"

/*
================
CG_WorldCoordToScreenCoordFloat

Take any world coord and convert it to a 2D virtual 640x480 screen coord
================
*/
qboolean CG_WorldCoordToScreenCoordFloat( vec3_t worldCoord, float *x, float *y )
{
	float	xcenter, ycenter;
	vec3_t	local, transformed;
	vec3_t	vfwd;
	vec3_t	vright;
	vec3_t	vup;
	float	xzi;
	float	yzi;

	//xcenter = cg.refdef.width / 2; // gives screen coords adjusted for resolution
	//ycenter = cg.refdef.height / 2; // gives screen coords adjusted for resolution
	
	// NOTE: did it this way because most draw functions expect virtual 640x480 coords
	//       and adjust them for current resolution
	xcenter = 640.f * .5f; // gives screen coords in virtual 640x480, to be adjusted when drawn
	ycenter = 480.f * .5f; // gives screen coords in virtual 640x480, to be adjusted when drawn
	
	AngleVectors( cg.refdefViewAngles, vfwd, vright, vup );
	VectorSubtract( worldCoord, cg.refdef.vieworg, local );
	
	transformed[0] = DotProduct( local, vright );
	transformed[1] = DotProduct( local, vup );
	transformed[2] = DotProduct( local, vfwd );

	// Make sure Z is not negative.
	if( transformed[2] < .01f ) {
		return qfalse;
	}

	xzi = xcenter / transformed[2] * ( 90.f / cg.refdef.fov_x );
	yzi = ycenter / transformed[2] * ( 90.f / cg.refdef.fov_y );

	*x = xcenter + xzi * transformed[0];
	*y = ycenter - yzi * transformed[1];

	return qtrue;
}

/*
================
CG_PointIsVisible

Is point visible from camera viewpoint?
================
*/
qboolean CG_PointIsVisible( vec3_t origin )
{
	trace_t	trace;

	CG_Trace( &trace, cg.refdef_current->vieworg, NULL, NULL,
		origin, -1, CONTENTS_SOLID );

	if( trace.fraction < 1.f ) {
		return qfalse;
	}

	return qtrue;
}

/*
================
CG_AddNameToESP

FIXME: In some cases the name won't fade out - it suddenly disappears
       (depends on the viewing angle).
================
*/
void CG_AddNameToESP( centity_t *cent )
{
	vec3_t			origin;
	qboolean		isVisible;
	float			x, y, dist, scale;
	clientInfo_t	*ci;
	espName_t		*name;

	// don't add spectating player name
	if( cent->currentState.clientNum == cg.snap->ps.clientNum ) {
		return;
	}

	if( cg.espNamesCount >= MAX_CLIENTS ) {
		return;
	}

	VectorCopy( cent->lerpOrigin, origin );
	origin[2] += 56;

	// even lower if needed
	if( cent->currentState.eFlags & EF_PRONE ||
		cent->currentState.eFlags & EF_DEAD ||
		cent->currentState.eFlags & EF_PLAYDEAD ) {
		origin[2] -= 45;
	} else if( cent->currentState.eFlags & EF_CROUCHING ) {
		origin[2] -= 18;
	}

	isVisible = CG_PointIsVisible( origin );
	
	if( !isVisible &&
		cg.time - cg.espNameTimes[cent->currentState.clientNum] > 1500 ) {
		return;
	}

	if( !CG_WorldCoordToScreenCoordFloat( origin, &x, &y ) ) {
		return;
	}

	dist = VectorDistance( cent->lerpOrigin, cg.refdef_current->vieworg );
	scale = 2000.f / ( dist > 1500.f ? 1500.f : dist ) * .075f;

	ci = &cgs.clientinfo[cent->currentState.clientNum];
	name = &cg.espNames[cg.espNamesCount];

	// add player name to esp list
	name->string = ci->name;
	name->x = x - CG_Text_Width_Ext( ci->name, scale, 0,
		&cgs.media.font1 ) * .5f;
	name->y = y;
	name->scale = scale;
	name->alpha = 1.f;

	if( isVisible ) {
		cg.espNameTimes[cent->currentState.clientNum] = cg.time;
	} else {
		float diff = cg.time - cg.espNameTimes[cent->currentState.clientNum];

		if( diff > 500.f ) {
			name->alpha -= ( diff - 500.f ) / 1000.f;
		}
	}

	cg.espNamesCount++;
}

/*
================
CG_DrawESP
================
*/
void CG_DrawESP( void )
{
	int			i;
	espName_t	*name;
	vec4_t		color = { 1.f, 1.f, 1.f, 1.f };

	// draw player names
	for( i = 0; i < cg.espNamesCount; i++ ) {
		name = &cg.espNames[i];

		if( !name ) {
			break;
		}

		color[3] = name->alpha;

		CG_Text_Paint_Ext( name->x, name->y, name->scale, name->scale,
			color, name->string, 0, 0, 0, &cgs.media.font1 );

		memset( name, 0, sizeof( name ) );
	}

	cg.espNamesCount = 0;
}
