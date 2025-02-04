//  edm - extensible display manager

//  Copyright (C) 1999 John W. Sinclair

//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.

//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

// This is the version of the bar widget from edm-R1-12-56-lcls1
// This previous version has some optimizations that were pulled out from later releases.
// This change seems to affect the LCLS fastBPM screens in terms of a flicker.
// Using double buffering in the fastEDM screen helps with the flickering but consumes way too much horsepower on the Sunrays in MCC
// So, Murali and Mike Z copied over the edm-R1-12-56-lcls1 version of this widget and added it as activeSlacBarClass in SLAClib


#define __bar_cc 1

#include "slacBar.h"
#include "app_pkg.h"
#include "act_win.h"

#include "thread.h"

static void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id )
{

activeSlacBarClass *baro = (activeSlacBarClass *) client;

  if ( !baro->init ) {
    baro->needToDrawUnconnected = 1;
    baro->needRefresh = 1;
    baro->actWin->addDefExeNode( baro->aglPtr );
  }

  baro->unconnectedTimer = 0;

}

static void barc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeSlacBarClass *baro = (activeSlacBarClass *) client;
char fmt[31+1], str[31+1];
int l;

  baro->actWin->setChanged();

  baro->eraseSelectBoxCorners();
  baro->erase();

  baro->fgColorMode = baro->bufFgColorMode;
  if ( baro->fgColorMode == BARC_K_COLORMODE_ALARM )
    baro->fgColor.setAlarmSensitive();
  else
    baro->fgColor.setAlarmInsensitive();
  baro->fgColor.setColorIndex( baro->bufFgColor, baro->actWin->ci );

  baro->barColorMode = baro->bufBarColorMode;
  if ( baro->barColorMode == BARC_K_COLORMODE_ALARM )
    baro->barColor.setAlarmSensitive();
  else
    baro->barColor.setAlarmInsensitive();
  baro->barColor.setColorIndex( baro->bufBarColor, baro->actWin->ci );

  baro->bgColor.setColorIndex( baro->bufBgColor, baro->actWin->ci );

  baro->controlPvExpStr.setRaw( baro->eBuf->bufControlPvName );
  baro->readPvExpStr.setRaw( baro->eBuf->bufReadPvName );
  baro->nullPvExpStr.setRaw( baro->eBuf->bufNullPvName );

  baro->label.setRaw( baro->eBuf->bufLabel );

  baro->labelType = baro->bufLabelType;

  strncpy( baro->fontTag, baro->fm.currentFontTag(), 63 );
  baro->actWin->fi->loadFontTag( baro->fontTag );
  baro->fs = baro->actWin->fi->getXFontStruct( baro->fontTag );
  baro->actWin->drawGc.setFontTag( baro->fontTag, baro->actWin->fi );

  if ( baro->fs ) {
    baro->barStrLen = XTextWidth( baro->fs, "10", 2 );
  }

  baro->border = baro->bufBorder;

  strncpy( baro->scaleFormat, baro->bufScaleFormat, 15 );
  baro->showScale = baro->bufShowScale;

  baro->labelTicksExpStr.setRaw( baro->bufLabelTicks );
  baro->majorTicksExpStr.setRaw( baro->bufMajorTicks );
  baro->minorTicksExpStr.setRaw( baro->bufMinorTicks );

  baro->x = baro->bufX;
  baro->sboxX = baro->bufX;

  baro->y = baro->bufY;
  baro->sboxY = baro->bufY;

  baro->w = baro->bufW;
  baro->sboxW = baro->bufW;

  baro->h = baro->bufH;
  baro->sboxH = baro->bufH;

  baro->horizontal = baro->bufHorizontal;

  baro->limitsFromDb = baro->bufLimitsFromDb;

  baro->precisionExpStr.setRaw( baro->bufPrecision );

  baro->readMinExpStr.setRaw( baro->bufReadMin );
  baro->readMaxExpStr.setRaw( baro->bufReadMax );

  baro->barOriginXExpStr.setRaw( baro->bufBarOriginX );

  // set edit-mode display values
  baro->precision = 0;
  baro->readMin = 0;
  baro->readMax = 10;
  baro->labelTicks = 10;
  baro->majorTicks = 2;
  baro->minorTicks = 2;
  baro->barOriginX = 0;
  strcpy( fmt, "%-g" );

  sprintf( str, fmt, baro->readMin );
  if ( baro->fs ) {
    baro->barStrLen = XTextWidth( baro->fs, str, strlen(str) );
  }
  sprintf( str, fmt, baro->readMax );
  if ( baro->fs ) {
    l = XTextWidth( baro->fs, str, strlen(str) );
    if ( l > baro->barStrLen ) baro->barStrLen = l;
  }

  baro->updateDimensions();

  if ( baro->horizontal ) {

    if ( baro->h < baro->minH ) {
      baro->h = baro->minH;
      baro->sboxH = baro->minH;
    }

  }
  else {

    if ( baro->h < baro->minVertH ) {
      baro->h = baro->minVertH;
      baro->sboxH = baro->minVertH;
    }

  }

}

static void barc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeSlacBarClass *baro = (activeSlacBarClass *) client;

  barc_edit_update ( w, client, call );
  baro->refresh( baro );

}

static void barc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeSlacBarClass *baro = (activeSlacBarClass *) client;

  barc_edit_update ( w, client, call );
  baro->ef.popdown();
  baro->operationComplete();

}

static void barc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeSlacBarClass *baro = (activeSlacBarClass *) client;

  baro->ef.popdown();
  baro->operationCancel();

}

static void barc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeSlacBarClass *baro = (activeSlacBarClass *) client;

  baro->ef.popdown();
  baro->operationCancel();
  baro->erase();
  baro->deleteRequest = 1;
  baro->drawAll();

}

static void bar_monitor_read_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

activeSlacBarClass *baro = (activeSlacBarClass *) userarg;

  baro->actWin->appCtx->proc->lock();

  if ( baro->activeMode ) {

    if ( pv->is_valid() ) {

      baro->pvNotConnectedMask &= ~( (unsigned char) 1 );
      if ( !baro->pvNotConnectedMask ) { // if all are connected
        baro->needConnectInit = 1;
        baro->actWin->addDefExeNode( baro->aglPtr );
      }

    }
    else {

      baro->pvNotConnectedMask |= 1; // read pv not connected
      baro->active = 0;
      baro->barColor.setDisconnected();
      baro->fgColor.setDisconnected();
      baro->bufInvalidate();
      baro->needFullDraw = 1;
      baro->actWin->addDefExeNode( baro->aglPtr );

    }

  }

  baro->actWin->appCtx->proc->unlock();

}

static void bar_monitor_null_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

activeSlacBarClass *baro = (activeSlacBarClass *) userarg;

  baro->actWin->appCtx->proc->lock();

  if ( baro->activeMode ) {

    if ( pv->is_valid() ) {

      baro->pvNotConnectedMask &= ~( (unsigned char) 2 );
      if ( !baro->pvNotConnectedMask ) { // if all are connected
        baro->needConnectInit = 1;
        baro->actWin->addDefExeNode( baro->aglPtr );
      }

    }
    else {

      baro->pvNotConnectedMask |= 2; // null pv not connected
      baro->active = 0;
      baro->barColor.setDisconnected();
      baro->fgColor.setDisconnected();
      baro->bufInvalidate();
      baro->needDraw = 1;
      baro->actWin->addDefExeNode( baro->aglPtr );

    }

  }

  baro->actWin->appCtx->proc->unlock();

}

static void bar_readUpdate (
  ProcessVariable *pv,
  void *userarg )
{

activeSlacBarClass *baro = (activeSlacBarClass *) userarg;
int st, sev;

  baro->actWin->appCtx->proc->lock();

  if ( baro->active ) {

    st = pv->get_status();
    sev = pv->get_severity();
    if ( ( st != baro->oldStat ) || ( sev != baro->oldSev ) ) {
      baro->oldStat = st;
      baro->oldSev = sev;
      baro->fgColor.setStatus( st, sev );
      baro->barColor.setStatus( st, sev );
      baro->needFullDraw = 1;
    }

    baro->curReadV = pv->get_double();
    baro->needDrawCheck = 1;
    baro->actWin->addDefExeNode( baro->aglPtr );

  }

  baro->actWin->appCtx->proc->unlock();

}

static void bar_nullUpdate (
  ProcessVariable *pv,
  void *userarg )
{

activeSlacBarClass *baro = (activeSlacBarClass *) userarg;

  baro->actWin->appCtx->proc->lock();

  if ( baro->active ) {

    baro->curNullV = pv->get_double();

    baro->needDrawCheck = 1;
    baro->actWin->addDefExeNode( baro->aglPtr );

  }

  baro->actWin->appCtx->proc->unlock();

}

activeSlacBarClass::activeSlacBarClass ( void ) {

  name = new char[strlen("activeSlacBarClass")+1];
  strcpy( name, "activeSlacBarClass" );
  checkBaseClassVersion( activeGraphicClass::MAJOR_VERSION, name );
  minW = 50;
  minH = 2;
  minVertW = 2;
  minVertH = 10;
  barStrLen = 10;
  strcpy( fontTag, "" );
  fs = NULL;
  activeMode = 0;

  barColorMode = BARC_K_COLORMODE_STATIC;
  fgColorMode = BARC_K_COLORMODE_STATIC;
  labelType = BARC_K_LITERAL;
  border = 1;
  showScale = 1;
  labelTicksExpStr.setRaw( "" );
  majorTicksExpStr.setRaw( "" );
  minorTicksExpStr.setRaw( "" );
  barOriginXExpStr.setRaw( "" );
  readMinExpStr.setRaw( "" );
  readMaxExpStr.setRaw( "" );

  readMin = 0;
  readMax = 10;
  labelTicks = 10;
  majorTicks = 2;
  minorTicks = 2;
  barOriginX = 0;

  limitsFromDb = 1;
  precisionExpStr.setRaw( "" );
  strcpy( scaleFormat, "FFloat" );
  precision = 0;
  unconnectedTimer = 0;
  eBuf = NULL;

}

// copy constructor
activeSlacBarClass::activeSlacBarClass
 ( const activeSlacBarClass *source ) {

activeGraphicClass *baro = (activeGraphicClass *) this;

  baro->clone( (activeGraphicClass *) source );

  name = new char[strlen("activeSlacBarClass")+1];
  strcpy( name, "activeSlacBarClass" );

  barCb = source->barCb;
  fgCb = source->fgCb;
  bgCb = source->bgCb;

  strncpy( fontTag, source->fontTag, 63 );
  fs = actWin->fi->getXFontStruct( fontTag );

  barColor.copy( source->barColor );
  fgColor.copy( source->fgColor );
  bgColor.copy( source->bgColor );

  controlPvExpStr.copy( source->controlPvExpStr );
  readPvExpStr.copy( source->readPvExpStr );
  nullPvExpStr.copy( source->nullPvExpStr );
  label.copy( source->label );

  barColorMode = source->barColorMode;
  fgColorMode = source->fgColorMode;
  labelType = source->labelType;
  border = source->border;
  showScale = source->showScale;
  labelTicksExpStr.copy( source->labelTicksExpStr );
  majorTicksExpStr.copy( source->majorTicksExpStr );
  minorTicksExpStr.copy( source->minorTicksExpStr );
  barOriginXExpStr.copy( source->barOriginXExpStr );
  barStrLen = source->barStrLen;

  minW = 50;
  minH = 2;
  minVertW = 2;
  minVertH = 10;
  activeMode = 0;

  readMin = source->readMin;
  readMax = source->readMax;
  labelTicks = source->labelTicks;
  majorTicks = source->majorTicks;
  minorTicks = source->minorTicks;
  barOriginX = source->barOriginX;

  limitsFromDb = source->limitsFromDb;
  readMinExpStr.copy( source->readMinExpStr );
  readMaxExpStr.copy( source->readMaxExpStr );

  precisionExpStr.copy( source->precisionExpStr );
  precision = source->precision;

  strncpy( scaleFormat, source->scaleFormat, 15 );

  horizontal = source->horizontal;

  unconnectedTimer = 0;

  eBuf = NULL;

  updateDimensions();

}

activeSlacBarClass::~activeSlacBarClass ( void ) {

/*   fprintf( stderr, "In activeSlacBarClass::~activeSlacBarClass\n" ); */

  if ( name ) delete[] name;

  if ( eBuf ) delete eBuf;

  if ( unconnectedTimer ) {
    XtRemoveTimeOut( unconnectedTimer );
    unconnectedTimer = 0;
  }

}

int activeSlacBarClass::createInteractive (
  activeWindowClass *aw_obj,
  int _x,
  int _y,
  int _w,
  int _h ) {

  actWin = (activeWindowClass *) aw_obj;
  x = _x;
  y = _y;
  w = _w;
  h = _h;

  if ( _w >= _h ) {
    horizontal = 1;
  }
  else {
    horizontal = 0;
  }

  if ( horizontal ) {
    if ( w < minW ) w = minW;
    if ( h < minH ) h = minH;
  }
  else {
    if ( w < minVertW ) w = minVertW;
    if ( h < minVertH ) h = minVertH;
  }

  barColor.setColorIndex( actWin->defaultFg1Color, actWin->ci );
  fgColor.setColorIndex( actWin->defaultTextFgColor, actWin->ci );
  bgColor.setColorIndex( actWin->defaultBgColor, actWin->ci );

  strcpy( fontTag, actWin->defaultCtlFontTag );
  actWin->fi->loadFontTag( fontTag );
  fs = actWin->fi->getXFontStruct( fontTag );

  updateDimensions();

  this->draw();

  this->editCreate();

  return 1;

}

int activeSlacBarClass::save (
  FILE *f )
{

int stat, major, minor, release;

tagClass tag;

static int zero = 0;
static char *emptyStr = "";

int lit = 1;
static char *labelTypeEnumStr[2] = {
  "pvName",
  "literal"
};
static int labelTypeEnum[2] = {
  0,
  1
};

int horz = 1;
static char *orienTypeEnumStr[2] = {
  "vertical",
  "horizontal"
};
static int orienTypeEnum[2] = {
  0,
  1
};

  major = BARC_MAJOR_VERSION;
  minor = BARC_MINOR_VERSION;
  release = BARC_RELEASE;

  tag.init();
  tag.loadW( "beginObjectProperties" );
  tag.loadW( "major", &major );
  tag.loadW( "minor", &minor );
  tag.loadW( "release", &release );
  tag.loadW( "x", &x );
  tag.loadW( "y", &y );
  tag.loadW( "w", &w );
  tag.loadW( "h", &h );
  tag.loadW( "indicatorColor", actWin->ci, &barColor );
  tag.loadBoolW( "indicatorAlarm", &barColorMode, &zero );
  tag.loadW( "fgColor", actWin->ci, &fgColor );
  tag.loadBoolW( "fgAlarm", &fgColorMode, &zero );
  tag.loadW( "bgColor", actWin->ci, &bgColor );
  tag.loadW( "indicatorPv", &readPvExpStr, emptyStr );
  tag.loadW( "nullPv", &nullPvExpStr, emptyStr );
  tag.loadW( "label", &label, emptyStr );
  tag.loadW( "labelType", 2, labelTypeEnumStr, labelTypeEnum,
   &labelType, &lit );
  tag.loadBoolW( "showScale", &showScale, &zero );
  tag.loadW( "origin", &barOriginXExpStr, emptyStr );
  tag.loadW( "font", fontTag );
  tag.loadW( "labelTicks", &labelTicksExpStr, emptyStr );
  tag.loadW( "majorTicks", &majorTicksExpStr, emptyStr );
  tag.loadW( "minorTicks", &minorTicksExpStr, emptyStr );
  tag.loadBoolW( "border", &border, &zero );
  tag.loadBoolW( "limitsFromDb", &limitsFromDb, &zero );
  tag.loadW( "precision", &precisionExpStr, emptyStr );
  tag.loadW( "min", &readMinExpStr, emptyStr );
  tag.loadW( "max", &readMaxExpStr, emptyStr );
  tag.loadW( "scaleFormat", scaleFormat );
  tag.loadW( "orientation", 2, orienTypeEnumStr, orienTypeEnum,
   &horizontal, &horz );
  tag.loadW( unknownTags );
  tag.loadW( "endObjectProperties" );
  tag.loadW( "" );

  stat = tag.writeTags( f );

  return stat;

}

int activeSlacBarClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int major, minor, release, stat;

tagClass tag;

static int zero = 0;
static char *emptyStr = "";

int lit = 1;
static char *labelTypeEnumStr[2] = {
  "pvName",
  "literal"
};
static int labelTypeEnum[2] = {
  0,
  1
};

int horz = 1;
static char *orienTypeEnumStr[2] = {
  "vertical",
  "horizontal"
};
static int orienTypeEnum[2] = {
  0,
  1
};

int l;
char fmt[31+1], str[31+1];

efInt efLabelTicks, efMajorTicks, efMinorTicks, efPrec;
efDouble efMin, efMax;

  this->actWin = _actWin;

  tag.init();
  tag.loadR( "beginObjectProperties" );
  tag.loadR( unknownTags );
  tag.loadR( "major", &major );
  tag.loadR( "minor", &minor );
  tag.loadR( "release", &release );
  tag.loadR( "x", &x );
  tag.loadR( "y", &y );
  tag.loadR( "w", &w );
  tag.loadR( "h", &h );
  tag.loadR( "indicatorColor", actWin->ci, &barColor );
  tag.loadR( "indicatorAlarm", &barColorMode, &zero );
  tag.loadR( "fgColor", actWin->ci, &fgColor );
  tag.loadR( "fgAlarm", &fgColorMode, &zero );
  tag.loadR( "bgColor", actWin->ci, &bgColor );
  tag.loadR( "indicatorPv", &readPvExpStr, emptyStr );
  tag.loadR( "nullPv", &nullPvExpStr, emptyStr );
  tag.loadR( "label", &label, emptyStr );
  tag.loadR( "labelType", 2, labelTypeEnumStr, labelTypeEnum,
   &labelType, &lit );
  tag.loadR( "showScale", &showScale, &zero );
  tag.loadR( "origin", &barOriginXExpStr, emptyStr );
  tag.loadR( "font", 63, fontTag );

  tag.loadR( "labelTicks", &labelTicksExpStr, emptyStr );
  tag.loadR( "majorTicks", &majorTicksExpStr, emptyStr );
  tag.loadR( "minorTicks", &minorTicksExpStr, emptyStr );

  tag.loadR( "border", &border, &zero );
  tag.loadR( "limitsFromDb", &limitsFromDb, &zero );

  tag.loadR( "precision", &precisionExpStr, emptyStr );
  tag.loadR( "min", &readMinExpStr, emptyStr );
  tag.loadR( "max", &readMaxExpStr, emptyStr );

  tag.loadR( "scaleFormat", 15, scaleFormat );
  tag.loadR( "orientation", 2, orienTypeEnumStr, orienTypeEnum,
   &horizontal, &horz );
  tag.loadR( "endObjectProperties" );

  stat = tag.readTags( f, "endObjectProperties" );

  if ( !( stat & 1 ) ) {
    actWin->appCtx->postMessage( tag.errMsg() );
  }

  if ( major > BARC_MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }

  if ( major < 4 ) {
    postIncompatable();
    return 0;
  }

  this->initSelectBox(); // call after getting x,y,w,h

  if ( barColorMode == BARC_K_COLORMODE_ALARM )
    barColor.setAlarmSensitive();
  else
    barColor.setAlarmInsensitive();

  if ( fgColorMode == BARC_K_COLORMODE_ALARM )
    fgColor.setAlarmSensitive();
  else
    fgColor.setAlarmInsensitive();

  bgColor.setAlarmInsensitive();

  actWin->fi->loadFontTag( fontTag );
  fs = actWin->fi->getXFontStruct( fontTag );

  if ( fs ) {
    barStrLen = XTextWidth( fs, "10", 2 );
  }

  // set edit-mode display values
  readMin = 0;
  readMax = 10;
  labelTicks = 10;
  majorTicks = 2;
  minorTicks = 2;
  barOriginX = 0;
  strcpy( fmt, "%-g" );

  sprintf( str, fmt, readMin );
  if ( fs ) {
    barStrLen = XTextWidth( fs, str, strlen(str) );
  }

  sprintf( str, fmt, readMax );
  if ( fs ) {
    l = XTextWidth( fs, str, strlen(str) );
    if ( l > barStrLen ) barStrLen = l;
  }

  readV = barOriginX;
  curReadV = barOriginX;
  curNullV = 0.0;
  updateDimensions();

  return stat;

}

int activeSlacBarClass::old_createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int r, g, b, discard, l, index;
int major, minor, release;
unsigned int pixel;
char oneName[PV_Factory::MAX_PV_NAME+1], fmt[31+1], str[31+1];
float fBarOriginX;
efInt efI;
efDouble efD;

  this->actWin = _actWin;

  fscanf( f, "%d %d %d\n", &major, &minor, &release ); actWin->incLine();

  if ( major > BARC_MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }

  fscanf( f, "%d\n", &x ); actWin->incLine();
  fscanf( f, "%d\n", &y ); actWin->incLine();
  fscanf( f, "%d\n", &w ); actWin->incLine();
  fscanf( f, "%d\n", &h ); actWin->incLine();

  this->initSelectBox();

  if ( ( major > 2 ) || ( ( major == 2 ) && ( minor > 1 ) ) ) {

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    barColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &barColorMode ); actWin->incLine();

    if ( barColorMode == BARC_K_COLORMODE_ALARM )
      barColor.setAlarmSensitive();
    else
      barColor.setAlarmInsensitive();

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    fgColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &fgColorMode ); actWin->incLine();

    if ( fgColorMode == BARC_K_COLORMODE_ALARM )
      fgColor.setAlarmSensitive();
    else
      fgColor.setAlarmInsensitive();

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    bgColor.setColorIndex( index, actWin->ci );

  }
  else if ( major > 1 ) {

    fscanf( f, "%d\n", &index ); actWin->incLine();
    barColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &barColorMode ); actWin->incLine();

    if ( barColorMode == BARC_K_COLORMODE_ALARM )
      barColor.setAlarmSensitive();
    else
      barColor.setAlarmInsensitive();

    fscanf( f, "%d\n", &index ); actWin->incLine();
    fgColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &fgColorMode ); actWin->incLine();

    if ( fgColorMode == BARC_K_COLORMODE_ALARM )
      fgColor.setAlarmSensitive();
    else
      fgColor.setAlarmInsensitive();

    fscanf( f, "%d\n", &index ); actWin->incLine();
    bgColor.setColorIndex( index, actWin->ci );

  }
  else {

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    if ( ( major < 2 ) && ( minor < 2 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    actWin->ci->setRGB( r, g, b, &pixel );
    index = actWin->ci->pixIndex( pixel );
    barColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &barColorMode ); actWin->incLine();

    if ( barColorMode == BARC_K_COLORMODE_ALARM )
      barColor.setAlarmSensitive();
    else
      barColor.setAlarmInsensitive();

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    if ( ( major < 2 ) && ( minor < 2 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    actWin->ci->setRGB( r, g, b, &pixel );
    index = actWin->ci->pixIndex( pixel );
    fgColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &fgColorMode ); actWin->incLine();

    if ( fgColorMode == BARC_K_COLORMODE_ALARM )
      fgColor.setAlarmSensitive();
    else
      fgColor.setAlarmInsensitive();

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    if ( ( major < 2 ) && ( minor < 2 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    actWin->ci->setRGB( r, g, b, &pixel );
    index = actWin->ci->pixIndex( pixel );
    bgColor.setColorIndex( index, actWin->ci );

  }

  bgColor.setAlarmInsensitive();

  readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
   actWin->incLine();
  controlPvExpStr.setRaw( oneName );

  readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
   actWin->incLine();
  readPvExpStr.setRaw( oneName );

  readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
   actWin->incLine();
  label.setRaw( oneName );

  fscanf( f, "%d\n", &labelType ); actWin->incLine();

  fscanf( f, "%d\n", &showScale ); actWin->incLine();

  if ( ( major == 1 ) && ( minor == 0 ) && ( release == 0 ) ) {
    fscanf( f, "%d\n", &discard ); actWin->incLine(); // use to be useDisplayBg
  }

  if ( ( major > 1 ) || ( minor > 4 ) ) {

    efD.read( f ); actWin->incLine();
    if ( !efD.isNull() ) {
      snprintf( oneName, 15, "%-g", efD.value() );
    }
    else {
      strcpy( oneName, "" );
    }
    barOriginXExpStr.setRaw( oneName );

  }
  else {

    fscanf( f, "%g\n", &fBarOriginX ); actWin->incLine();
    snprintf( oneName, 15, "%-g", (double) fBarOriginX );
    barOriginXExpStr.setRaw( oneName );

  }

  readStringFromFile( fontTag, 63+1, f ); actWin->incLine();

  actWin->fi->loadFontTag( fontTag );
  fs = actWin->fi->getXFontStruct( fontTag );

  if ( fs ) {
    barStrLen = XTextWidth( fs, "10", 2 );
  }

  if ( ( major > 1 ) || ( minor > 0 ) ) {

    readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
     actWin->incLine();
    labelTicksExpStr.setRaw( oneName );

    readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
     actWin->incLine();
    majorTicksExpStr.setRaw( oneName );

    readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
     actWin->incLine();
    minorTicksExpStr.setRaw( oneName );

    // majorTicks now means majors per label

    labelTicks = atol( labelTicksExpStr.getRaw() );

    if ( labelTicks ) {
      majorTicks /= labelTicks;
    }
    else {
      majorTicks = 0;
    }

    snprintf( str, 31, "%-d", majorTicks );

    majorTicksExpStr.setRaw( str );

    fscanf( f, "%d\n", &border ); actWin->incLine();

  }

 if ( ( major > 1 ) || ( minor > 3 ) ) {
   fscanf( f, "%d\n", &limitsFromDb ); actWin->incLine();
  }
  else {
    limitsFromDb = 1;
  }

  if ( ( major > 1 ) || ( minor > 2 ) ) {

    efI.read( f ); actWin->incLine();
    if ( !efI.isNull() ) {
      snprintf( oneName, 15, "%-d", efI.value() );
    }
    else {
      strcpy( oneName, "" );
    }
    precisionExpStr.setRaw( oneName );

    efD.read( f ); actWin->incLine();
    if ( !efD.isNull() ) {
      snprintf( oneName, 15, "%-g", efD.value() );
    }
    else {
      strcpy( oneName, "" );
    }
    readMinExpStr.setRaw( oneName );

    efD.read( f ); actWin->incLine();
    if ( !efD.isNull() ) {
      snprintf( oneName, 15, "%-g", efD.value() );
    }
    else {
      strcpy( oneName, "" );
    }
    readMaxExpStr.setRaw( oneName );

    readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f ); actWin->incLine();
    strncpy( scaleFormat, oneName, 15 );

  }
  else {

    
    precisionExpStr.setRaw( "" );
    precision = 0;
    readMinExpStr.setRaw( "" );
    readMin = 0;
    readMaxExpStr.setRaw( "" );
    readMax = 10;

  }

  if ( ( major > 1 ) || ( minor > 5 ) ) {

    readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
     actWin->incLine();
    nullPvExpStr.setRaw( oneName );

  }
  else {

    nullPvExpStr.setRaw( "" );

  }

  if ( ( ( major == 2 ) && ( minor > 0 ) ) || major > 2 ) {

   fscanf( f, "%d\n", &horizontal ); actWin->incLine();

  }
  else {
    
    if ( w >= h ) {
      horizontal = 1;
    }
    else {
      horizontal = 0;
    }

  }

  // set edit-mode display values
  readMin = 0;
  readMax = 10;
  labelTicks = 10;
  majorTicks = 2;
  minorTicks = 2;
  barOriginX = 0;
  strcpy( fmt, "%-g" );

  sprintf( str, fmt, readMin );
  if ( fs ) {
    barStrLen = XTextWidth( fs, str, strlen(str) );
  }
  sprintf( str, fmt, readMax );
  if ( fs ) {
    l = XTextWidth( fs, str, strlen(str) );
    if ( l > barStrLen ) barStrLen = l;
  }

  readV = barOriginX;
  curReadV = barOriginX;
  curNullV = 0.0;
  updateDimensions();

  return 1;

}

int activeSlacBarClass::genericEdit ( void ) {

char title[32], *ptr;

  if ( !eBuf ) {
    eBuf = new editBufType;
  }

  if ( horizontal )
    strcpy( title, activeSlacBarClass_str3 );
  else
    strcpy( title, activeSlacBarClass_str4 );

  ptr = actWin->obj.getNameFromClass( "activeSlacBarClass" );
  if ( ptr )
    Strncat( title, ptr, 31 );
  else
    Strncat( title, activeSlacBarClass_str5, 31 );

  Strncat( title, activeSlacBarClass_str6, 31 );

  bufX = x;
  bufY = y;
  bufW = w;
  bufH = h;

  bufBarColor = barColor.pixelIndex();
  bufBarColorMode = barColorMode;

  bufFgColor = fgColor.pixelIndex();
  bufFgColorMode = fgColorMode;

  bufBgColor = bgColor.pixelIndex();

  strncpy( bufFontTag, fontTag, 63 );

  if ( readPvExpStr.getRaw() )
    strncpy( eBuf->bufReadPvName, readPvExpStr.getRaw(), PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->bufReadPvName, "" );

  if ( controlPvExpStr.getRaw() )
    strncpy( eBuf->bufControlPvName, controlPvExpStr.getRaw(), PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->bufControlPvName, "" );

  if ( nullPvExpStr.getRaw() )
    strncpy( eBuf->bufNullPvName, nullPvExpStr.getRaw(), PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->bufNullPvName, "" );

  if ( label.getRaw() )
    strncpy( eBuf->bufLabel, label.getRaw(), PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->bufLabel, "" );

  bufLabelType = labelType;

  bufBorder = border;

  bufShowScale = showScale;

  if ( labelTicksExpStr.getRaw() ) {
    strncpy( bufLabelTicks, labelTicksExpStr.getRaw(), 15 );
    bufLabelTicks[15] = 0;
  }
  else {
    strcpy( bufLabelTicks, "" );
  }

  if ( majorTicksExpStr.getRaw() ) {
    strncpy( bufMajorTicks, majorTicksExpStr.getRaw(), 15 );
    bufMajorTicks[15] = 0;
  }
  else {
    strcpy( bufMajorTicks, "" );
  }

  if ( minorTicksExpStr.getRaw() ) {
    strncpy( bufMinorTicks, minorTicksExpStr.getRaw(), 15 );
    bufMinorTicks[15] = 0;
  }
  else {
    strcpy( bufMinorTicks, "" );
  }

  if ( barOriginXExpStr.getRaw() ) {
    strncpy( bufBarOriginX, barOriginXExpStr.getRaw(), 15 );
    bufBarOriginX[15] = 0;
  }
  else {
    strcpy( bufBarOriginX, "" );
  }

  bufLimitsFromDb = limitsFromDb;

  if ( precisionExpStr.getRaw() ) {
    strncpy( bufPrecision, precisionExpStr.getRaw(), 15 );
    bufPrecision[15] = 0;
  }
  else {
    strcpy( bufPrecision, "" );
  }

  if ( readMinExpStr.getRaw() ) {
    strncpy( bufReadMin, readMinExpStr.getRaw(), 15 );
    bufReadMin[15] = 0;
  }
  else {
    strcpy( bufReadMin, "" );
  }

  if ( readMaxExpStr.getRaw() ) {
    strncpy( bufReadMax, readMaxExpStr.getRaw(), 15 );
    bufReadMax[15] = 0;
  }
  else {
    strcpy( bufReadMax, "" );
  }

  strncpy( bufScaleFormat, scaleFormat, 15 );
  bufHorizontal = horizontal;

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  ef.addTextField( activeSlacBarClass_str7, 35, &bufX );
  ef.addTextField( activeSlacBarClass_str8, 35, &bufY );
  ef.addTextField( activeSlacBarClass_str9, 35, &bufW );
  ef.addTextField( activeSlacBarClass_str10, 35, &bufH );

  ef.addTextField( activeSlacBarClass_str12, 35, eBuf->bufReadPvName, PV_Factory::MAX_PV_NAME );
  ef.addTextField( activeSlacBarClass_str13, 35, eBuf->bufNullPvName, PV_Factory::MAX_PV_NAME );

  ef.addOption( activeSlacBarClass_str14, activeSlacBarClass_str15, &bufLabelType );
  labelTypeEntry = ef.getCurItem();
  labelTypeEntry->setNumValues( 2 );
  ef.addTextField( activeSlacBarClass_str16, 35, eBuf->bufLabel, PV_Factory::MAX_PV_NAME );
  labelEntry = ef.getCurItem();
  labelTypeEntry->addInvDependency( 0, labelEntry );
  labelTypeEntry->addDependencyCallbacks();

  ef.addToggle( activeSlacBarClass_str18, &bufBorder );

  ef.addToggle( activeSlacBarClass_str19, &bufShowScale );
  showScaleEntry = ef.getCurItem();
  ef.addTextField( activeSlacBarClass_str20, 35, bufLabelTicks, 15 );
  labelTicksEntry = ef.getCurItem();
  showScaleEntry->addDependency( labelTicksEntry );
  ef.addTextField( activeSlacBarClass_str21, 35, bufMajorTicks, 15 );
  majorTicksEntry = ef.getCurItem();
  showScaleEntry->addDependency( majorTicksEntry );
  ef.addTextField( activeSlacBarClass_str22, 35, bufMinorTicks, 15 );
  minorTicksEntry = ef.getCurItem();
  showScaleEntry->addDependency( minorTicksEntry );

  ef.addToggle( activeSlacBarClass_str23, &bufLimitsFromDb );
  limitsFromDbEntry = ef.getCurItem();
  ef.addOption( activeSlacBarClass_str24, activeSlacBarClass_str25, bufScaleFormat, 15 );
  scaleFormatEntry = ef.getCurItem();
  showScaleEntry->addDependency( scaleFormatEntry );
  ef.addTextField( activeSlacBarClass_str26, 35, bufPrecision, 15 );
  scalePrecEntry = ef.getCurItem();
  limitsFromDbEntry->addInvDependency( scalePrecEntry );
  ef.addTextField( activeSlacBarClass_str27, 35, bufReadMin, 15 );
  scaleMinEntry = ef.getCurItem();
  limitsFromDbEntry->addInvDependency( scaleMinEntry );
  ef.addTextField( activeSlacBarClass_str28, 35, bufReadMax, 15 );
  scaleMaxEntry = ef.getCurItem();
  limitsFromDbEntry->addInvDependency( scaleMaxEntry );
  limitsFromDbEntry->addDependencyCallbacks();

  ef.addTextField( activeSlacBarClass_str29, 35, bufBarOriginX, 15 );
  scaleOriginEntry = ef.getCurItem();
  showScaleEntry->addDependency( scaleOriginEntry );
  showScaleEntry->addDependencyCallbacks();

  ef.addOption( activeSlacBarClass_str44, activeSlacBarClass_str45,
   &bufHorizontal );

  ef.addColorButton( activeSlacBarClass_str30, actWin->ci, &barCb, &bufBarColor );
  ef.addToggle( activeSlacBarClass_str31, &bufBarColorMode );
  ef.addColorButton( activeSlacBarClass_str32, actWin->ci, &fgCb, &bufFgColor );
  ef.addToggle( activeSlacBarClass_str33, &bufFgColorMode );
  ef.addColorButton( activeSlacBarClass_str34, actWin->ci, &bgCb, &bufBgColor );

  ef.addFontMenu( activeSlacBarClass_str17, actWin->fi, &fm, fontTag );

  XtUnmanageChild( fm.alignWidget() ); // no alignment info

  return 1;

}

int activeSlacBarClass::editCreate ( void ) {

  this->genericEdit();
  ef.finished( barc_edit_ok, barc_edit_apply, barc_edit_cancel_delete, this );
  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

int activeSlacBarClass::edit ( void ) {

  this->genericEdit();
  ef.finished( barc_edit_ok, barc_edit_apply, barc_edit_cancel, this );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

int activeSlacBarClass::erase ( void ) {

  if ( deleteRequest ) return 1;

  actWin->drawGc.setLineWidth( 1 );
  actWin->drawGc.setLineStyle( LineSolid );

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  return 1;

}

int activeSlacBarClass::eraseActive ( void ) {

  if ( !enabled || !activeMode || !init ) return 1;

  actWin->executeGc.setFG( bgColor.getColor() );

  if ( bufInvalid ) {

    actWin->executeGc.setLineWidth( 1 );
    actWin->executeGc.setLineStyle( LineSolid );

    XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
     actWin->executeGc.eraseGC(), x, y, w, h );

    XFillRectangle( actWin->d, drawable(actWin->executeWidget),
     actWin->executeGc.eraseGC(), x, y, w, h );

  }
  else {

    XFillRectangle( actWin->d, drawable(actWin->executeWidget),
     actWin->executeGc.normGC(), oldBarX, barY, oldBarW, barH );

  }

  return 1;

}

void activeSlacBarClass::drawHorzScale (
  Widget widget,
  Drawable dr,
  gcClass *gc )
{

  drawXLinearScale ( actWin->d, dr, gc, 1, barAreaX,
   barY + barH + 3, barAreaW, readMin, readMax, labelTicks,
   majorTicks, minorTicks, fgColor.pixelColor(),
   bgColor.pixelColor(), 0, 0, 0, 0, 0, actWin->fi, fontTag, fs, 1, 0, 0, 0 );

}

void activeSlacBarClass::drawVertScale (
  Widget widget,
  Drawable dr,
  gcClass *gc )
{

  drawYLinearScale ( actWin->d, dr, gc, 1, barAreaX - 4,
   barAreaY, barAreaH, readMin, readMax, labelTicks,
   majorTicks, minorTicks, fgColor.pixelColor(),
   bgColor.pixelColor(), 0, 0, 0, 0, 0, actWin->fi, fontTag, fs, 1, 0, 0, 0 );

}

void activeSlacBarClass::drawScale (
  Widget widget,
  Drawable dr,
  gcClass *gc )
{

  if ( horizontal )
    drawHorzScale( widget, dr, gc );
  else
    drawVertScale( widget, dr, gc );

}

int activeSlacBarClass::draw ( void ) {

int tX, tY;

  if ( deleteRequest ) return 1;

  actWin->drawGc.setLineWidth( 1 );
  actWin->drawGc.setLineStyle( LineSolid );

  actWin->drawGc.saveFg();

  if ( horizontal ) {

    actWin->drawGc.setFG( bgColor.pixelColor() );

    XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.normGC(), x, y, w, h );

    actWin->drawGc.setFG( barColor.pixelColor() );

    XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.normGC(), barAreaX, barY, barAreaW, barH );

    actWin->drawGc.setFG( fgColor.getColor() );

    if ( showScale ) drawScale( actWin->drawWidget,
     XtWindow(actWin->drawWidget), &actWin->drawGc );

    XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.normGC(), x, y, w, h );

    if ( strcmp( label.getRaw(), "" ) != 0 ) {
      if ( fs ) {
        actWin->drawGc.setFontTag( fontTag, actWin->fi );
        tX = barAreaX;
        tY = y + 2;
        if ( border ) tY += 2;
        drawText( actWin->drawWidget, &actWin->drawGc, fs, tX, tY,
         XmALIGNMENT_BEGINNING, label.getRaw() );
      }
    }

  }
  else { // vertical

    actWin->drawGc.setFG( bgColor.pixelColor() );

    XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.normGC(), x, y, w, h );

    actWin->drawGc.setFG( barColor.pixelColor() );

    XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.normGC(), barAreaX, barAreaY-barAreaH,
     barAreaW, barAreaH );

    actWin->drawGc.setFG( fgColor.getColor() );

    if ( showScale ) drawScale( actWin->drawWidget,
     XtWindow(actWin->drawWidget), &actWin->drawGc );

    XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.normGC(), x, y, w, h );

    if ( strcmp( label.getRaw(), "" ) != 0 ) {
      if ( fs ) {
        actWin->drawGc.setFontTag( fontTag, actWin->fi );
        tX = barAreaX + barAreaW;
        tY = y + (int) ( .25 * (double) fontHeight );
        if ( border ) tY += 2;
        drawText( actWin->drawWidget, &actWin->drawGc, fs, tX, tY,
         XmALIGNMENT_END, label.getRaw() );
      }
    }

  }

  actWin->drawGc.restoreFg();

  return 1;

}

int activeSlacBarClass::drawActive ( void ) {

int tX, tY, x0, y0, x1, y1;
char str[PV_Factory::MAX_PV_NAME+1];

  if ( !init ) {
    if ( needToDrawUnconnected ) {
      actWin->executeGc.saveFg();
      actWin->executeGc.setFG( bgColor.getDisconnected() );
      actWin->executeGc.setLineWidth( 1 );
      actWin->executeGc.setLineStyle( LineSolid );
      XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
       actWin->executeGc.normGC(), x, y, w, h );
      actWin->executeGc.restoreFg();
      needToEraseUnconnected = 1;
    }
  }
  else if ( needToEraseUnconnected ) {
    actWin->executeGc.setLineWidth( 1 );
    actWin->executeGc.setLineStyle( LineSolid );
    XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
     actWin->executeGc.eraseGC(), x, y, w, h );
    needToEraseUnconnected = 0;
  }

  if ( !enabled || !activeMode || !init ) return 1;

  actWin->executeGc.saveFg();

  if ( horizontal ) {

    if ( bufInvalid ) {

      actWin->executeGc.setFG( bgColor.getColor() );

      XFillRectangle( actWin->d, drawable(actWin->executeWidget),
       actWin->executeGc.normGC(), x, y, w, h );

      actWin->executeGc.setFG( barColor.getColor() );

      XFillRectangle( actWin->d, drawable(actWin->executeWidget),
       actWin->executeGc.normGC(), barX, barY, barW, barH );

    }
    else {

      if ( zeroCrossover ) {

        actWin->executeGc.setFG( bgColor.getColor() );

        XFillRectangle( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.normGC(), oldBarX, barY, oldBarW, barH );

        actWin->executeGc.setFG( barColor.getColor() );

        XFillRectangle( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.normGC(), barX, barY, barW, barH );

      }
      else {

        if ( aboveBarOrigin ) {

          if ( barW > oldBarW ) {

            actWin->executeGc.setFG( barColor.getColor() );

            XFillRectangle( actWin->d, drawable(actWin->executeWidget),
             actWin->executeGc.normGC(), oldBarX+oldBarW, barY,
             barW-oldBarW, barH );

          }
          else {

            actWin->executeGc.setFG( bgColor.getColor() );

            XFillRectangle( actWin->d, drawable(actWin->executeWidget),
             actWin->executeGc.normGC(), barX+barW, barY,
             oldBarW-barW, barH );

          }

        }
        else {

          if ( barX < oldBarX ) {

            actWin->executeGc.setFG( barColor.getColor() );

            XFillRectangle( actWin->d, drawable(actWin->executeWidget),
             actWin->executeGc.normGC(), barX, barY,
             oldBarX-barX, barH );

          }
          else {

            actWin->executeGc.setFG( bgColor.getColor() );

            XFillRectangle( actWin->d, drawable(actWin->executeWidget),
             actWin->executeGc.normGC(), oldBarX, barY,
             barX-oldBarX, barH );

          }

        }

      }

    }

    oldBarX = barX;
    oldBarW = barW;

  }
  else { // vertical

    if ( bufInvalid ) {

      actWin->executeGc.setFG( bgColor.getColor() );

      XFillRectangle( actWin->d, drawable(actWin->executeWidget),
       actWin->executeGc.normGC(), x, y, w, h );

      actWin->executeGc.setFG( barColor.getColor() );

      XFillRectangle( actWin->d, drawable(actWin->executeWidget),
       actWin->executeGc.normGC(), barX, barY-barH, barW, barH );

      // draw line along origin
      if ( border || showScale )
        x0 = barAreaX - 4;
      else
        x0 = x;
      x1 = x + w;
      y1 = y0 = (int) rint( barAreaY -
       ( barOriginX - readMin ) * barAreaH / ( readMax - readMin ) );
      XDrawLine( actWin->d, drawable(actWin->executeWidget),
       actWin->executeGc.normGC(), x0, y0, x1, y1 );

    }
    else {

      if ( zeroCrossover ) {

        actWin->executeGc.setFG( bgColor.getColor() );

        XFillRectangle( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.normGC(), barX, oldBarY-oldBarH, barW, oldBarH );

        actWin->executeGc.setFG( barColor.getColor() );

        XFillRectangle( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.normGC(), barX, barY-barH, barW, barH );

        // draw line along origin
        if ( border || showScale )
          x0 = barAreaX - 4;
        else
          x0 = x;
        x1 = x + w;
        y1 = y0 = (int) rint( barAreaY -
         ( barOriginX - readMin ) * barAreaH / ( readMax - readMin ) );
        XDrawLine( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.normGC(), x0, y0, x1, y1 );

      }
      else {

        if ( aboveBarOrigin ) {

          if ( barH > oldBarH ) {

            actWin->executeGc.setFG( barColor.getColor() );

            XFillRectangle( actWin->d, drawable(actWin->executeWidget),
             actWin->executeGc.normGC(), barX, barY-barH,
             barW, barH-oldBarH );

          }
          else {

            actWin->executeGc.setFG( bgColor.getColor() );

            XFillRectangle( actWin->d, drawable(actWin->executeWidget),
             actWin->executeGc.normGC(), barX, barY-oldBarH,
             barW, oldBarH-barH );

          }

        }
        else {

          if ( barY > oldBarY ) {

            actWin->executeGc.setFG( barColor.getColor() );

            XFillRectangle( actWin->d, drawable(actWin->executeWidget),
             actWin->executeGc.normGC(), barX, oldBarY,
             barW, barY-oldBarY );

          }
          else {

            actWin->executeGc.setFG( bgColor.getColor() );

            XFillRectangle( actWin->d, drawable(actWin->executeWidget),
             actWin->executeGc.normGC(), barX, barY,
             barW, oldBarY-barY );

          }

        }

      }

    }

    oldBarY = barY;
    oldBarH = barH;

  }

  if ( bufInvalid ) { // draw scale, label, etc ...

    actWin->executeGc.setFG( fgColor.getColor() );

    if ( showScale ) {
      drawScale( actWin->executeWidget,
       drawable(actWin->drawWidget), &actWin->executeGc );
    }

    if ( labelType == BARC_K_PV_NAME )
      strncpy( str, readPvId->get_name(), PV_Factory::MAX_PV_NAME );
    else
      strncpy( str, label.getExpanded(), PV_Factory::MAX_PV_NAME );

    if ( horizontal ) {

      if ( strcmp( str, "" ) != 0 ) {
        if ( fs ) {
          actWin->executeGc.setFontTag( fontTag, actWin->fi );
          tX = barAreaX;
          tY = y + 2;
          if ( border ) tY += 2;
          drawText( actWin->executeWidget, drawable(actWin->drawWidget),
           &actWin->executeGc, fs, tX, tY, XmALIGNMENT_BEGINNING, str );
        }
      }

    }
    else {

      if ( strcmp( str, "" ) != 0 ) {
        if ( fs ) {
          actWin->executeGc.setFontTag( fontTag, actWin->fi );
          tX = barAreaX + barAreaW;
          tY = y + (int) ( .25 * (double) fontHeight );
          if ( border ) tY += 2;
          drawText( actWin->executeWidget, drawable(actWin->drawWidget),
           &actWin->executeGc, fs, tX, tY, XmALIGNMENT_END, str );
        }
      }

    }

    if ( border ) {
      actWin->executeGc.setLineWidth( 1 );
      actWin->executeGc.setLineStyle( LineSolid );
      XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
       actWin->executeGc.normGC(), x, y, w, h );
    }

    bufInvalid = 0;

  }

  actWin->executeGc.restoreFg();

  return 1;

}

int activeSlacBarClass::activate (
  int pass,
  void *ptr )
{

int opStat;
char fmt[31+1];

  switch ( pass ) {

  case 1:

    zeroCrossover = 0;
    oldAboveBarOrigin = 0;
    needConnectInit = needInfoInit = needRefresh = needErase = needDrawCheck =
     needDraw = needFullDraw = 0;
    needToEraseUnconnected = 0;
    needToDrawUnconnected = 0;
    unconnectedTimer = 0;

    readPvId = nullPvId = NULL;
    initialReadConnection = initialNullConnection = 1;
    oldStat = oldSev = -1;

    aglPtr = ptr;
    opComplete = 0;
    curNullV = 0.0;

    if ( horizontal ) {
      barW = 0;
      oldBarW = 0;
      barX = 0;
      oldBarX = 0;
    }
    else {
      barH = 0;
      oldBarH = 0;
      barY = 0;
      oldBarY = 0;
    }

    pvNotConnectedMask = active = init = 0;
    activeMode = 1;

    if ( !readPvExpStr.getExpanded() ||
       // ( strcmp( readPvExpStr.getExpanded(), "" ) == 0 ) ) {
       blankOrComment( readPvExpStr.getExpanded() ) ) {
      readExists = 0;
    }
    else {
      readExists = 1;
      pvNotConnectedMask |= 1;
      barColor.setConnectSensitive();
      fgColor.setConnectSensitive();
    }

    if ( !nullPvExpStr.getExpanded() ||
       // ( strcmp( nullPvExpStr.getExpanded(), "" ) == 0 ) ) {
       blankOrComment( nullPvExpStr.getExpanded() ) ) {
      nullExists = 0;
    }
    else {
      nullExists = 1;
      pvNotConnectedMask |= 2;
    }

    break;

  case 2:

    if ( !opComplete ) {

      if ( blank( labelTicksExpStr.getExpanded() ) ) {
        labelTicks = 0;
      }
      else {
        labelTicks = atol( labelTicksExpStr.getExpanded() );
      }

      if ( blank( majorTicksExpStr.getExpanded() ) ) {
        majorTicks = 0;
      }
      else {
        majorTicks = atol( majorTicksExpStr.getExpanded() );
      }

      if ( blank( minorTicksExpStr.getExpanded() ) ) {
        minorTicks = 0;
      }
      else {
        minorTicks = atol( minorTicksExpStr.getExpanded() );
      }

      if ( blank( barOriginXExpStr.getExpanded() ) ) {
        barOriginX = 0;
      }
      else {
        barOriginX = atol( barOriginXExpStr.getExpanded() );
      }

      if ( blank( precisionExpStr.getExpanded() ) ) {
        precision = 0;
      }
      else {
        precision = atol( precisionExpStr.getExpanded() );
      }

      if ( blank( readMinExpStr.getExpanded() ) ) {
        readMin = 0;
      }
      else {
        readMin = atof( readMinExpStr.getExpanded() );
      }

      if ( blank( readMaxExpStr.getExpanded() ) ) {
        readMax = 0;
      }
      else {
        readMax = atof( readMaxExpStr.getExpanded() );
      }

      if ( readMax == readMin ) {
        readMax = readMin + 1;
      }

      if ( blank( barOriginXExpStr.getExpanded() ) ) {
        barOriginX = 0;
      }
      else {
        barOriginX = atof( barOriginXExpStr.getExpanded() );
      }

      if ( strcmp( scaleFormat, "GFloat" ) == 0 ) {
        sprintf( fmt, "%%.%-dg", precision );
      }
      else if ( strcmp( scaleFormat, "Exponential" ) == 0 ) {
        sprintf( fmt, "%%.%-de", precision );
      }
      else {
        sprintf( fmt, "%%.%-df", precision );
      }

      initEnable();

      if ( !unconnectedTimer ) {
        unconnectedTimer = appAddTimeOut( actWin->appCtx->appContext(),
         2000, unconnectedTimeout, this );
      }

      opStat = 1;

      if ( readExists ) {
	readPvId = the_PV_Factory->create( readPvExpStr.getExpanded() );
	if ( readPvId ) {
	  readPvId->add_conn_state_callback( bar_monitor_read_connect_state,
           this );
	}
	else {
          fprintf( stderr, activeSlacBarClass_str36 );
          opStat = 0;
        }
      }

      if ( nullExists ) {
	nullPvId = the_PV_Factory->create( nullPvExpStr.getExpanded() );
	if ( nullPvId ) {
	  nullPvId->add_conn_state_callback( bar_monitor_null_connect_state,
           this );
	}
	else {
          fprintf( stderr, activeSlacBarClass_str36 );
          opStat = 0;
        }
      }

      if ( opStat & 1 ) opComplete = 1;

      return opStat;

    }

    break;

  case 3:
  case 4:
  case 5:
  case 6:

    break;

  }

  return 1;

}

int activeSlacBarClass::deactivate (
  int pass
) {

char fmt[31+1], str[31+1];
int l;

  active = 0;
  activeMode = 0;

  if ( pass == 1 ) {

    if ( unconnectedTimer ) {
      XtRemoveTimeOut( unconnectedTimer );
      unconnectedTimer = 0;
    }

    if ( readExists ) {
      if ( readPvId ) {
        readPvId->remove_conn_state_callback( bar_monitor_read_connect_state,
         this );
        readPvId->remove_value_callback( bar_readUpdate, this );
        readPvId->release();
        readPvId = NULL;
      }
    }

    if ( nullExists ) {
      if ( nullPvId ) {
        nullPvId->remove_conn_state_callback( bar_monitor_null_connect_state,
         this );
        nullPvId->remove_value_callback( bar_nullUpdate, this );
        nullPvId->release();
        nullPvId = NULL;
      }
    }

  }

  // set edit-mode display values
  readMin = 0;
  readMax = 10;
  labelTicks = 10;
  majorTicks = 2;
  minorTicks = 2;
  barOriginX = 0;
  strcpy( fmt, "%-g" );

  sprintf( str, fmt, readMin );
  if ( fs ) {
    barStrLen = XTextWidth( fs, str, strlen(str) );
  }
  sprintf( str, fmt, readMax );
  if ( fs ) {
    l = XTextWidth( fs, str, strlen(str) );
    if ( l > barStrLen ) barStrLen = l;
  }

  updateDimensions();

  return 1;

}

void activeSlacBarClass::updateDimensions ( void )
{

  if ( fs ) {
    fontAscent = fs->ascent;
    fontDescent = fs->descent;
    fontHeight = fontAscent + fontDescent;
  }
  else {
    fontAscent = 10;
    fontDescent = 5;
    fontHeight = fontAscent + fontDescent;
  }

  barAreaX = x;
  barAreaY = y;
  barAreaW = w;
  barAreaH = h;

  if ( horizontal ) {

    minH = 2;
    barY = y;

    barAreaX = x;
    barAreaW = w;

    if ( ( strcmp( label.getRaw(), "" ) != 0 ) ||
         ( labelType == BARC_K_PV_NAME ) ) {
      minH += fontHeight + 5;
      barY += fontHeight + 5;
      if ( border ) {
        minH += 9;
        barY += 5;
        barAreaX = x + 5;
        barAreaW = w - 9;
      }
    }
    else {
      if ( border && showScale ) {
        minH += 9;
        barY += 5;
      }
    }

    if ( showScale ) {
      minH += fontHeight + fontHeight + 5;
      barAreaX = x + barStrLen/2 + 3;
      barAreaW = w - barStrLen - 6;
    }

    if ( border && !showScale && ( ( strcmp( label.getRaw(), "" ) == 0 ) ||
     ( labelType == BARC_K_PV_NAME ) ) ) {
      minH += 9;
      barY += 5;
      barAreaX = x + 5;
      barAreaW = w - 9;
    }

    if ( h < minH ) {

      h = minH;
      sboxH = minH;

    }

    barH = h;

    if ( ( strcmp( label.getRaw(), "" ) != 0 ) ||
         ( labelType == BARC_K_PV_NAME ) ) {
      barH -= ( fontHeight + 5 );
      if ( border ) barH -= 9;
    }

    if ( showScale ) {
      barH -= ( fontHeight + fontHeight + 5 );
    }

    if ( border && !showScale && ( ( strcmp( label.getRaw(), "" ) == 0 ) ||
     ( labelType == BARC_K_PV_NAME ) ) ) {
      barH -= 9;
    }

  }
  else {  // vertical

    minVertW = 2;
    minVertH = 10;

    if ( ( strcmp( label.getRaw(), "" ) != 0 ) ||
         ( labelType == BARC_K_PV_NAME ) ) {
      minVertH += fontHeight + 5;
    }

    if ( showScale ) {
      minVertH += fontHeight;
      //???????????????????????
      minVertW += 4 + barStrLen + 10 + (int) rint( 0.5 * fontHeight );
    }
    else if ( border ) {
      //???????????????????????
      minVertH += 8;
      minVertW += 4;
    }

    if ( w < minVertW ) {

      w = minVertW;
      sboxW = minVertW;

    }

    if ( h < minVertH ) {

      h = minVertH;
      sboxH = minVertH;

    }

    barH = barAreaH = h;
    barY = barAreaY = y + barAreaH;
    barX = barAreaX = x;
    barW = barAreaW = w;

    if ( ( strcmp( label.getRaw(), "" ) != 0 ) ||
         ( labelType == BARC_K_PV_NAME ) ) {
      barAreaH -= (int) ( 1.5 * (double) fontHeight ) - 5;
      barH = barAreaH;
    }

    if ( showScale ) {
      barH -= ( fontHeight );
      barAreaH -= ( fontHeight );
    }
    else if ( border ) {
      barH -= 8;
      barAreaH -= 8;
    }

    if ( showScale ) {
      barY -= (int) rint( 0.5 * fontHeight );
      barAreaY -= (int) rint( 0.5 * fontHeight );
      barAreaW -= ( 4 +  barStrLen + 8 + (int) rint( 0.5 * fontHeight ) );
      barW -= ( 4 + barStrLen + 8 + (int) rint( 0.5 * fontHeight ) );
      barAreaX += 2 + barStrLen + 8 + (int) rint( 0.5 * fontHeight );
      barX += 2 + barStrLen + 8 + (int) rint( 0.5 * fontHeight );
    }
    else if ( border ) {
      barY -= 4;
      barAreaY -= 4;
      barAreaW -= 9;
      barW -= 9;
      barAreaX += 5;
      barX += 5;
    }

  }

  updateScaleInfo();

}

void activeSlacBarClass::btnUp (
  int x,
  int y,
  int barState,
  int barNumber )
{

  if ( !enabled ) return;

}

void activeSlacBarClass::btnDown (
  int x,
  int y,
  int barState,
  int barNumber )
{

  if ( !enabled ) return;

}

void activeSlacBarClass::btnDrag (
  int x,
  int y,
  int barState,
  int barNumber )
{

  if ( !enabled ) return;

}

int activeSlacBarClass::getBarActionRequest (
  int *up,
  int *down,
  int *drag )
{

  if ( !controlExists ) {
    *up = 0;
    *down = 0;
    *drag = 0;
    return 1;
  }

  *up = 1;
  *down = 1;
  *drag = 1;

  return 1;

}

void activeSlacBarClass::bufInvalidate ( void )
{

  bufInvalid = 1;

}

int activeSlacBarClass::expandTemplate (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

expStringClass tmpStr;

  tmpStr.setRaw( label.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  label.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( readPvExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  readPvExpStr.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( nullPvExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  nullPvExpStr.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( labelTicksExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  labelTicksExpStr.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( majorTicksExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  majorTicksExpStr.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( minorTicksExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  minorTicksExpStr.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( readMinExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  readMinExpStr.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( readMaxExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  readMaxExpStr.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( precisionExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  precisionExpStr.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( barOriginXExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  barOriginXExpStr.setRaw( tmpStr.getExpanded() );

  return 1;

}

int activeSlacBarClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int retStat, stat;

  retStat = label.expand1st( numMacros, macros, expansions );
  stat = readPvExpStr.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = nullPvExpStr.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = labelTicksExpStr.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = majorTicksExpStr.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = minorTicksExpStr.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = readMinExpStr.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = readMaxExpStr.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = precisionExpStr.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = barOriginXExpStr.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  return retStat;

}

int activeSlacBarClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int retStat, stat;

  retStat = label.expand2nd( numMacros, macros, expansions );
  stat = readPvExpStr.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = nullPvExpStr.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = labelTicksExpStr.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = majorTicksExpStr.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = minorTicksExpStr.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = readMinExpStr.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = readMaxExpStr.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = precisionExpStr.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = barOriginXExpStr.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  return retStat;

}

int activeSlacBarClass::containsMacros ( void ) {

int result;

  result = label.containsPrimaryMacros();
  if ( result ) return 1;

  result = readPvExpStr.containsPrimaryMacros();
  if ( result ) return 1;

  result = nullPvExpStr.containsPrimaryMacros();
  if ( result ) return 1;

  result = labelTicksExpStr.containsPrimaryMacros();
  if ( result ) return 1;

  result = majorTicksExpStr.containsPrimaryMacros();
  if ( result ) return 1;

  result = minorTicksExpStr.containsPrimaryMacros();
  if ( result ) return 1;

  result = readMinExpStr.containsPrimaryMacros();
  if ( result ) return 1;

  result = readMaxExpStr.containsPrimaryMacros();
  if ( result ) return 1;

  result = precisionExpStr.containsPrimaryMacros();
  if ( result ) return 1;

  result = barOriginXExpStr.containsPrimaryMacros();
  if ( result ) return 1;

  return 0;

}

int activeSlacBarClass::checkResizeSelectBox (
  int _x,
  int _y,
  int _w,
  int _h ) {

int tmpw, tmph, ret_stat;

  ret_stat = 1;

  tmpw = sboxW;
  tmph = sboxH;

  tmpw += _w;
  tmph += _h;

  if ( horizontal ) {

    if ( tmpw < minW ) {
      ret_stat = 0;
    }

    if ( tmph < minH ) {
      ret_stat = 0;
    }

  }
  else {

    if ( tmpw < minVertW ) {
      ret_stat = 0;
    }

    if ( tmph < minVertH ) {
      ret_stat = 0;
    }

  }

  return ret_stat;

}

int activeSlacBarClass::checkResizeSelectBoxAbs (
  int _x,
  int _y,
  int _w,
  int _h ) {

int tmpw, tmph, ret_stat;

  ret_stat = 1;

  tmpw = _w;
  tmph = _h;

  if ( horizontal ) {

    if ( tmpw != -1 ) {
      if ( tmpw < minW ) {
        ret_stat = 0;
      }
    }

    if ( tmph != -1 ) {
      if ( tmph < minH ) {
        ret_stat = 0;
      }
    }

  }
  else {

    if ( tmpw != -1 ) {
      if ( tmpw < minVertW ) {
        ret_stat = 0;
      }
    }

    if ( tmph != -1 ) {
      if ( tmph < minVertH ) {
        ret_stat = 0;
      }
    }

  }

  return ret_stat;

}

void activeSlacBarClass::updateScaleInfo ( void ) {

  if ( horizontal )
    updateHorzScaleInfo();
  else
    updateVertScaleInfo();

}

void activeSlacBarClass::updateHorzScaleInfo ( void ) {

int locW;

  if ( readMax == readMin ) readMax = readMin + 1.0;

  if ( readMax >= readMin ) {

    mode = BARC_K_MAX_GE_MIN;

    if ( barOriginX < readMin )
      barOriginX = readMin;
    else if ( barOriginX > readMax )
     barOriginX = readMax;

    originW = (int) rint( ( barOriginX - readMin ) *
     barAreaW / ( readMax - readMin ) );

    if ( readV > readMax ) readV = readMax;
    if ( readV < readMin ) readV = readMin;

  }
  else {

    mode = BARC_K_MAX_LT_MIN;

    if ( barOriginX > readMin )
      barOriginX = readMin;
    else if ( barOriginX < readMax )
     barOriginX = readMax;

    originW = (int) rint( ( barOriginX - readMin ) *
     barAreaW / ( readMax - readMin ) );

    if ( readV < readMax ) readV = readMax;
    if ( readV > readMin ) readV = readMin;

  }

  switch ( mode ) {

  case BARC_K_MAX_GE_MIN:

    if ( readV >= barOriginX ) {

      barX = originW;

      if ( barOriginX == readMax ) {

        barW = 0;
        factorGe = 0;

      }
      else {

        barW = (int) ( ( barAreaW - originW ) *
         ( readV - barOriginX ) /
         ( readMax - barOriginX ) + 0.5 );

        if ( barW > ( barAreaW - originW ) )
          barW = barAreaW - originW;

        factorGe = ( barAreaW - originW ) / ( readMax - barOriginX );

      }

    }
    else {

      if ( barOriginX == readMin ) {

        locW = 0;
        factorLt = 0;

      }
      else {

        locW = (int) ( ( originW ) *
         ( readV - barOriginX ) /
         ( readMin - barOriginX ) + 0.5 );

        factorLt = originW / ( readMin - barOriginX );

      }

      barX = originW - locW;

      barW = abs( locW );

      if ( barX < 0 ) {
        barX = 0;
        barW = originW;
      }

    }

    break;

  case BARC_K_MAX_LT_MIN:

    if ( readV < barOriginX ) {

      barX = originW;

      if ( barOriginX == readMax ) {

        barW = 0;
        factorLt = 0;

      }
      else {

        barW = (int) ( ( barAreaW - originW ) *
         ( readV - barOriginX ) /
         ( readMax - barOriginX ) + 0.5 );

        if ( barW > ( barAreaW - originW ) )
          barW = barAreaW - originW;

        factorLt = ( barAreaW - originW ) / ( readMax - barOriginX );

      }

    }
    else {

      if ( barOriginX == readMin ) {

        locW = 0;
        factorGe = 0;

      }
      else {

        locW = (int) ( ( originW ) *
         ( readV - barOriginX ) /
         ( readMin - barOriginX ) + 0.5 );

        factorGe = originW / ( readMin - barOriginX );

      }

      barX = originW - locW;

      barW = abs( locW );

      if ( barX < 0 ) {
        barX = 0;
        barW = originW;
      }

    }

    break;

  }

  barMaxW = barAreaW - originW;

  barX += barAreaX;

}

void activeSlacBarClass::updateVertScaleInfo ( void ) {

int locH;

  if ( readMax == readMin ) readMax = readMin + 1.0;

  if ( readMax >= readMin ) {

    mode = BARC_K_MAX_GE_MIN;

    if ( barOriginX < readMin )
      barOriginX = readMin;
    else if ( barOriginX > readMax )
     barOriginX = readMax;

    originH = (int) rint( ( barOriginX - readMin ) *
     barAreaH / ( readMax - readMin ) );

    if ( readV > readMax ) readV = readMax;
    if ( readV < readMin ) readV = readMin;

  }
  else {

    mode = BARC_K_MAX_LT_MIN;

    if ( barOriginX > readMin )
      barOriginX = readMin;
    else if ( barOriginX < readMax )
     barOriginX = readMax;

    originH = (int) rint( ( barOriginX - readMin ) *
     barAreaH / ( readMax - readMin ) );

    if ( readV < readMax ) readV = readMax;
    if ( readV > readMin ) readV = readMin;

  }

  switch ( mode ) {

  case BARC_K_MAX_GE_MIN:

    if ( readV >= barOriginX ) {

      barY = barAreaY - originH;

      if ( barOriginX == readMax ) {

        barH = 0;
        factorGe = 0;

      }
      else {

        barH = (int) ( ( barAreaH - originH ) *
         ( readV - barOriginX ) /
         ( readMax - barOriginX ) + 0.5 );

        if ( barH > ( barAreaH - originH ) )
          barH = barAreaH - originH;

        factorGe = ( barAreaH - originH ) / ( readMax - barOriginX );

      }

    }
    else {

      if ( barOriginX == readMin ) {

        locH = 0;
        factorLt = 0;

      }
      else {

        locH = (int) ( ( originH ) *
         ( readV - barOriginX ) /
         ( readMin - barOriginX ) + 0.5 );

        factorLt = originH / ( readMin - barOriginX );

      }

      barY = barAreaY - ( originH - locH );

      barH = abs( locH );

      if ( barY < 0 ) {
        barY = 0;
        barH = originH;
      }

    }

    break;

  case BARC_K_MAX_LT_MIN:

    if ( readV < barOriginX ) {

      barY = barAreaY - originH;

      if ( barOriginX == readMax ) {

        barH = 0;
        factorLt = 0;

      }
      else {

        barH = (int) ( ( barAreaH - originH ) *
         ( readV - barOriginX ) /
         ( readMax - barOriginX ) + 0.5 );

        if ( barH > ( barAreaH - originH ) )
          barH = barAreaH - originH;

        factorLt = ( barAreaH - originH ) / ( readMax - barOriginX );

      }

    }
    else {

      if ( barOriginX == readMin ) {

        locH = 0;
        factorGe = 0;

      }
      else {

        locH = (int) ( ( originH ) *
         ( readV - barOriginX ) /
         ( readMin - barOriginX ) + 0.5 );

        factorGe = originH / ( readMin - barOriginX );

      }

      barY = barAreaY - ( originH - locH );

      barH = abs( locH );

      if ( barY < 0 ) {
        barY = 0;
        barH = originH;
      }

    }

    break;

  }

  barMaxH = barAreaH - originH;

}

void activeSlacBarClass::updateBar ( void ) {

int locW, locH;
double tempW, tempH;

  if ( horizontal ) {

    switch ( mode ) {

    case BARC_K_MAX_GE_MIN:

      if ( readV >= barOriginX ) {

        aboveBarOrigin = 1;

      }
      else {

        aboveBarOrigin = 0;

      }

      break;

    case BARC_K_MAX_LT_MIN:

      if ( readV < barOriginX ) {

        aboveBarOrigin = 1;

      }
      else {

        aboveBarOrigin = 0;

      }

      break;

    }

    if ( aboveBarOrigin != oldAboveBarOrigin ) {
      oldAboveBarOrigin = aboveBarOrigin;
      zeroCrossover = 1;
      updateScaleInfo();
    }
    else {
      zeroCrossover = 0;
    }

    switch ( mode ) {

    case BARC_K_MAX_GE_MIN:

      if ( readV >= barOriginX ) {

        barX = originW;

        //barW = (int) ( factorGe * ( readV - barOriginX ) + 0.5 );

        //if ( barW > barMaxW ) barW = barMaxW;

        tempW = factorGe * ( readV - barOriginX );
        if ( tempW > barMaxW ) {
          barW = barMaxW;
        }
        else {
          barW = (int) ( tempW + 0.5 );
        }

      }
      else {

        locW = (int) ( ( readV - barOriginX ) * factorLt + 0.5 );

        barX = originW - locW;

        barW = abs( locW );

        if ( barX < 0 ) {
          barX = 0;
          barW = originW;
        }

      }

      break;

    case BARC_K_MAX_LT_MIN:

      if ( readV < barOriginX ) {

        barX = originW;

        //barW = (int) ( ( readV - barOriginX ) * factorLt + 0.5 );

        //if ( barW > barMaxW ) barW = barMaxW;

        tempW = ( readV - barOriginX ) * factorLt;
        if ( tempW > barMaxW ) {
          barW = barMaxW;
        }
        else {
          barW = (int) ( tempW + 0.5 );
        }

      }
      else {

        locW = (int) ( factorGe * ( readV - barOriginX ) + 0.5 );

        barX = originW - locW;

        barW = abs( locW );

        if ( barX < 0 ) {
          barX = 0;
          barW = originW;
        }

      }

      break;

    }

    barX += barAreaX;

  }
  else { // vertical

    switch ( mode ) {

    case BARC_K_MAX_GE_MIN:

      if ( readV >= barOriginX ) {

        aboveBarOrigin = 1;

      }
      else {

        aboveBarOrigin = 0;

      }

      break;

    case BARC_K_MAX_LT_MIN:

      if ( readV < barOriginX ) {

        aboveBarOrigin = 1;

      }
      else {

        aboveBarOrigin = 0;

      }

      break;

    }

    if ( aboveBarOrigin != oldAboveBarOrigin ) {
      oldAboveBarOrigin = aboveBarOrigin;
      zeroCrossover = 1;
      updateScaleInfo();
    }
    else {
      zeroCrossover = 0;
    }

    switch ( mode ) {

    case BARC_K_MAX_GE_MIN:

      if ( readV >= barOriginX ) {

        barY = barAreaY - originH;

        //barH = (int) ( factorGe * ( readV - barOriginX ) + 0.5 );

        //if ( barH > barMaxH ) barH = barMaxH;

        tempH = factorGe * ( readV - barOriginX );
        if ( tempH > barMaxH ) {
          barH = barMaxH;
        }
        else {
          barH = (int) ( tempH + 0.5 );
        }

      }
      else {

        locH = (int) ( ( readV - barOriginX ) * factorLt + 0.5 );

        barY = barAreaY - ( originH - locH );

        barH = abs( locH );

        if ( barY > barAreaY ) {
          barY = barAreaY;
          barH = originH;
        }

      }

      break;

    case BARC_K_MAX_LT_MIN:

      if ( readV < barOriginX ) {

        barY = barAreaY - originH;

        //barH = (int) ( ( readV - barOriginX ) * factorLt + 0.5 );

        //if ( barH > barMaxH ) barH = barMaxH;

        tempH = ( readV - barOriginX ) * factorLt;
        if ( tempH > barMaxH ) {
          barH = barMaxH;
        }
        else {
          barH = (int) ( tempH + 0.5 );
        }

      }
      else {

        locH = (int) ( factorGe * ( readV - barOriginX ) + 0.5 );

        barY = barAreaY - ( originH - locH );

        barH = abs( locH );

        if ( barY > barAreaY ) {
          barY = barAreaY;
          barH = originH;
        }

      }

      break;

    }

  }

}

void activeSlacBarClass::executeDeferred ( void ) {

int l, nc, ni, nr, ne, nd, nfd, ndc;
char fmt[31+1], str[31+1];
double v;

  if ( actWin->isIconified ) return;

  actWin->appCtx->proc->lock();
  nc = needConnectInit; needConnectInit = 0;
  ni = needInfoInit; needInfoInit = 0;
  nr = needRefresh; needRefresh = 0;
  ne = needErase; needErase = 0;
  nd = needDraw; needDraw = 0;
  nfd = needFullDraw; needFullDraw = 0;
  ndc = needDrawCheck; needDrawCheck = 0;
  v = curReadV - curNullV;
  actWin->remDefExeNode( aglPtr );
  actWin->appCtx->proc->unlock();

  if ( !activeMode ) return;

//----------------------------------------------------------------------------

  if ( nc ) {

    v = curReadV = readPvId->get_double();

#if 0
    if ( limitsFromDb || blank( readMinExpStr.getExpanded() ) ) {
      readMin = readPvId->get_lower_disp_limit();
    }

    if ( limitsFromDb || blank( readMaxExpStr.getExpanded() ) ) {
      readMax = readPvId->get_upper_disp_limit();
    }

    if ( limitsFromDb || blank( precisionExpStr.getExpanded() ) ) {
      precision = readPvId->get_precision();
    }
#endif

    if ( limitsFromDb ) {
      readMin = readPvId->get_lower_disp_limit();
    }

    if ( limitsFromDb ) {
      readMax = readPvId->get_upper_disp_limit();
    }

    if ( limitsFromDb ) {
      precision = readPvId->get_precision();
    }

    if ( readMin == readMax ) readMax = readMin + 1;

    ni = 1;

  }

//----------------------------------------------------------------------------

  if ( ni ) {

    if ( blank( barOriginXExpStr.getExpanded() ) ) {
      barOriginX = readMin;
    }

    if ( strcmp( scaleFormat, "GFloat" ) == 0 ) {
      sprintf( fmt, "%%.%-dg", precision );
    }
    else if ( strcmp( scaleFormat, "Exponential" ) == 0 ) {
      sprintf( fmt, "%%.%-de", precision );
    }
    else {
      sprintf( fmt, "%%.%-df", precision );
    }

    sprintf( str, fmt, readMin );
    if ( fs ) {
      barStrLen = XTextWidth( fs, str, strlen(str) );
    }

    sprintf( str, fmt, readMax );
    if ( fs ) {
      l = XTextWidth( fs, str, strlen(str) );
      if ( l > barStrLen ) barStrLen = l;
    }

    updateDimensions();

    active = 1;
    init = 1;
    barColor.setConnected();
    fgColor.setConnected();
    bufInvalidate();
    eraseActive();
    readV = v;
    updateDimensions();
    drawActive();

    if ( initialReadConnection ) {

      initialReadConnection = 0;

      readPvId->add_value_callback( bar_readUpdate, this );

    }

    if ( nullExists ) {

      if ( initialNullConnection ) {

        initialNullConnection = 0;

        nullPvId->add_value_callback( bar_nullUpdate, this );

      }

    }

  }

//----------------------------------------------------------------------------

  if ( nr ) {

    bufInvalidate();

    eraseActive();

    readV = v;

    updateDimensions();

    drawActive();

  }

//----------------------------------------------------------------------------

  if ( ne ) {
    eraseActive();
  }

//----------------------------------------------------------------------------

  if ( nd ) {
    readV = v;
    drawActive();
  }

//----------------------------------------------------------------------------

  if ( nfd ) {
    readV = v;
    bufInvalidate();
    drawActive();
  }

//----------------------------------------------------------------------------

  if ( ndc ) {

      readV = v;
      updateBar();
      drawActive();

  }

//----------------------------------------------------------------------------

}

char *activeSlacBarClass::firstDragName ( void ) {

  if ( !enabled ) return NULL;

  dragIndex = 0;
  return dragName[dragIndex];

}

char *activeSlacBarClass::nextDragName ( void ) {

  if ( !enabled ) return NULL;

  if ( dragIndex < (int) ( sizeof(dragName) / sizeof(char *) ) - 1 ) {
    dragIndex++;
    return dragName[dragIndex];
  }
  else {
    return NULL;
  }

}

char *activeSlacBarClass::dragValue (
  int i ) {

  if ( !enabled ) return NULL;

  if ( actWin->mode == AWC_EXECUTE ) {

    if ( !i ) {
      return readPvExpStr.getExpanded();
    }
    else {
      return nullPvExpStr.getExpanded();
    }

  }
  else {

    if ( !i ) {
      return readPvExpStr.getRaw();
    }
    else {
      return nullPvExpStr.getRaw();
    }

  }

}

void activeSlacBarClass::changeDisplayParams (
  unsigned int _flag,
  char *_fontTag,
  int _alignment,
  char *_ctlFontTag,
  int _ctlAlignment,
  char *_btnFontTag,
  int _btnAlignment,
  int _textFgColor,
  int _fg1Color,
  int _fg2Color,
  int _offsetColor,
  int _bgColor,
  int _topShadowColor,
  int _botShadowColor )
{

  if ( _flag & ACTGRF_FG1COLOR_MASK )
    barColor.setColorIndex( _fg1Color, actWin->ci );

  if ( _flag & ACTGRF_TEXTFGCOLOR_MASK )
    fgColor.setColorIndex( _textFgColor, actWin->ci );

  if ( _flag & ACTGRF_BGCOLOR_MASK )
    bgColor.setColorIndex( _bgColor, actWin->ci );

  if ( _flag & ACTGRF_CTLFONTTAG_MASK ) {
    strcpy( fontTag, _ctlFontTag );
    actWin->fi->loadFontTag( fontTag );
    fs = actWin->fi->getXFontStruct( fontTag );
    updateDimensions();
  }

}

void activeSlacBarClass::changePvNames (
  int flag,
  int numCtlPvs,
  char *ctlPvs[],
  int numReadbackPvs,
  char *readbackPvs[],
  int numNullPvs,
  char *nullPvs[],
  int numVisPvs,
  char *visPvs[],
  int numAlarmPvs,
  char *alarmPvs[] )
{

  if ( flag & ACTGRF_CTLPVS_MASK ) {
    if ( numCtlPvs ) {
      controlPvExpStr.setRaw( ctlPvs[0] );
    }
  }

  if ( flag & ACTGRF_READBACKPVS_MASK ) {
    if ( numReadbackPvs ) {
      readPvExpStr.setRaw( readbackPvs[0] );
    }
  }

  if ( flag & ACTGRF_NULLPVS_MASK ) {
    if ( numNullPvs ) {
      nullPvExpStr.setRaw( nullPvs[0] );
    }
  }

}

void activeSlacBarClass::getPvs (
  int max,
  ProcessVariable *pvs[],
  int *n ) {

  if ( max < 2 ) {
    *n = 0;
    return;
  }

  *n = 2;
  pvs[0] = readPvId;
  pvs[1] = nullPvId;

}

// crawler functions may return blank pv names
char *activeSlacBarClass::crawlerGetFirstPv ( void ) {

  crawlerPvIndex = 0;
  return readPvExpStr.getExpanded();

}

char *activeSlacBarClass::crawlerGetNextPv ( void ) {

  if ( crawlerPvIndex >= 1 ) return NULL;

  crawlerPvIndex++;

  if ( crawlerPvIndex == 1 ) {
    return nullPvExpStr.getExpanded();
  }

  return NULL;

}

#ifdef __cplusplus
extern "C" {
#endif

void *create_activeSlacBarClassPtr ( void ) {

activeSlacBarClass *ptr;

  ptr = new activeSlacBarClass;
  return (void *) ptr;

}

void *clone_activeSlacBarClassPtr (
  void *_srcPtr )
{

activeSlacBarClass *ptr, *srcPtr;

  srcPtr = (activeSlacBarClass *) _srcPtr;

  ptr = new activeSlacBarClass( srcPtr );

  return (void *) ptr;

}

#ifdef __cplusplus
}
#endif
