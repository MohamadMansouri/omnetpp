//==========================================================================
//   GLOBALS.H  - header for
//                             OMNeT++
//            Discrete System Simulation in C++
//
//==========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 1992-2004 Andras Varga

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/

#ifndef __GLOBALS_H
#define __GLOBALS_H

#include "cobject.h"
#include "onstartup.h"   // for cSingleton

//=== classes mentioned
class  cModuleInterface;
class  cModuleType;
class  cChannelType;
class  cLinkType;
class  cFunctionType;
class  cNetworkType;
class  cEnum;

//=== Global objects:

//< Internal: list in which objects are accumulated if there's no simple module in context.
//< @see cObject::setDefaultOwner() and cSimulation::setContextModule())
SIM_API extern cDefaultList defaultList;

SIM_API extern cSingleton<cArray> networks;       ///< List of available networks.
SIM_API extern cSingleton<cArray> modinterfaces;  ///< List of all module interfaces.
SIM_API extern cSingleton<cArray> modtypes;       ///< List of all module types.
SIM_API extern cSingleton<cArray> channeltypes;   ///< List of channel types.
SIM_API extern cSingleton<cArray> functions;      ///< List of function types.
SIM_API extern cSingleton<cArray> classes;        ///< List of cClassRegister objects.
SIM_API extern cSingleton<cArray> enums;          ///< List of cEnum objects.


/**
 * @name Finding global objects by name.
 * @ingroup Functions
 */
//@{

/** Find a cNetworkType. */
cNetworkType *findNetwork(const char *s);

/** Find a cModuleType. */
cModuleType *findModuleType(const char *s);

/** Find a cModuleInterface. */
cModuleInterface *findModuleInterface(const char *s);

/** Find a cChannelType. */
cChannelType *findChannelType(const char *s);

/** DEPRECATED. */
cLinkType *findLink(const char *s);

/** Find a cFunctionType. */
cFunctionType *findFunction(const char *s,int argcount);

/** Find a cEnum. */
cEnum *findEnum(const char *s);
//@}

#endif

