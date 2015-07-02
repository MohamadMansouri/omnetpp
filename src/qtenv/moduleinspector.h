//==========================================================================
//  MODULEINSPECTOR.H - part of
//
//                     OMNeT++/OMNEST
//            Discrete System Simulation in C++
//
//==========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 1992-2015 Andras Varga
  Copyright (C) 2006-2015 OpenSim Ltd.

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/

#ifndef __OMNETPP_QTENV_MODULEINSPECTOR_H
#define __OMNETPP_QTENV_MODULEINSPECTOR_H

#include "inspector.h"

class QBoxLayout;
class QMouseEvent;
class QContextMenuEvent;

namespace omnetpp {
class cObject;
class cModule;
class cGate;
class cCanvas;

namespace qtenv {

class CanvasRenderer;
class ModuleGraphicsView;

enum SendAnimMode {ANIM_BEGIN, ANIM_END, ANIM_THROUGH};

class QTENV_API ModuleInspector : public Inspector
{
   Q_OBJECT

   private slots:
      void runUntil();
      void fastRunUntil();
      void stopSimulation();
      void relayout();
      void zoomIn(int x = 0, int y = 0);
      void zoomOut(int x = 0, int y = 0);

      void doubleClick(QMouseEvent *event);
      void createContextMenu(QContextMenuEvent *event);

      void layers();
      void toggleLabels();
      void toggleArrowheads();
      void zoomIconsBy();

   protected:
      CanvasRenderer *canvasRenderer;
      ModuleGraphicsView *view;

   protected:
      //TODO Where is getCanvas() right place? Here or in ModuleGraphicsView.
      cCanvas *getCanvas();
      static const char *animModeToStr(SendAnimMode mode);

      void addToolBar(QBoxLayout *layout);
      void createView(QWidget *parent);

      void zoomBy(double mult, bool snaptoone = false, int x = 0, int y = 0);

   public:
      ModuleInspector(QWidget *parent, bool isTopLevel, InspectorFactory *f);
      ~ModuleInspector();
      virtual void doSetObject(cObject *obj) override;
      virtual void refresh() override;
      virtual void clearObjectChangeFlags() override;
      virtual int inspectorCommand(int argc, const char **argv) override;

      bool needsRedraw();

      // implementations of inspector commands:
      virtual int getDefaultLayoutSeed();
      virtual int getLayoutSeed(int argc, const char **argv);
      virtual int setLayoutSeed(int argc, const char **argv);
      virtual int getSubmoduleCount(int argc, const char **argv);
      virtual int getSubmodQ(int argc, const char **argv);
      virtual int getSubmodQLen(int argc, const char **argv);

      // drawing methods:
      virtual void redraw() override;

      // notifications from envir:
      virtual void submoduleCreated(cModule *newmodule);
      virtual void submoduleDeleted(cModule *module);
      virtual void connectionCreated(cGate *srcgate);
      virtual void connectionDeleted(cGate *srcgate);
      virtual void displayStringChanged();
      virtual void displayStringChanged(cModule *submodule);
      virtual void displayStringChanged(cGate *gate);
      virtual void bubble(cComponent *subcomponent, const char *text);

      virtual void animateMethodcallAscent(cModule *srcSubmodule, const char *methodText);
      virtual void animateMethodcallDescent(cModule *destSubmodule, const char *methodText);
      virtual void animateMethodcallHoriz(cModule *srcSubmodule, cModule *destSubmodule, const char *methodText);
      static void animateMethodcallDelay(Tcl_Interp *interp);
      virtual void animateMethodcallCleanup();
      virtual void animateSendOnConn(cGate *srcGate, cMessage *msg, SendAnimMode mode);
      virtual void animateSenddirectAscent(cModule *srcSubmodule, cMessage *msg);
      virtual void animateSenddirectDescent(cModule *destSubmodule, cMessage *msg, SendAnimMode mode);
      virtual void animateSenddirectHoriz(cModule *srcSubmodule, cModule *destSubmodule, cMessage *msg, SendAnimMode mode);
      virtual void animateSenddirectCleanup();
      virtual void animateSenddirectDelivery(cModule *destSubmodule, cMessage *msg);
      static void performAnimations(Tcl_Interp *interp);
};

} // namespace qtenv
} // namespace omnetpp

#endif