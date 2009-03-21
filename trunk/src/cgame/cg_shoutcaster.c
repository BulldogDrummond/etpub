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
CG_AddNameToESP

FIXME: In some cases the name won't fade out - it suddenly disappears
       (depends on the viewing angle).
================
*/
void CG_AddNameToESP( centity_t *cent )
{
	vec3_t			origin;
	qboolean		visible;
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
	origin[2] += 64;

	// even lower if needed
	if( cent->currentState.eFlags & EF_PRONE ||
		cent->currentState.eFlags & EF_DEAD ||
		cent->currentState.eFlags & EF_PLAYDEAD ) {
		origin[2] -= 45;
	} else if( cent->currentState.eFlags & EF_CROUCHING ) {
		origin[2] -= 18;
	}

	visible = CG_PointIsVisible( origin );
	
	if( !visible &&
		cg.time - cg.espNameTimes[cent->currentState.clientNum] > 1500 ) {
		return;
	}

	if( !CG_WorldToScreen( origin, &x, &y ) ) {
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

	if( visible ) {
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
