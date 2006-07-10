package org.omnetpp.experimental.animation.primitives;

import org.eclipse.draw2d.IFigure;
import org.eclipse.draw2d.LayoutManager;
import org.eclipse.draw2d.geometry.Rectangle;
import org.omnetpp.experimental.animation.controller.TimerQueue;
import org.omnetpp.experimental.animation.replay.ReplayAnimationController;

public abstract class AbstractAnimationPrimitive implements IAnimationPrimitive {
	protected ReplayAnimationController animationController;
	
	protected double beginSimulationTime;
	
	protected long eventNumber;
	
	protected boolean shown;
	
	public AbstractAnimationPrimitive(ReplayAnimationController animationController, long eventNumber, double beginSimulationTime) {
		this.animationController = animationController;
		this.beginSimulationTime = beginSimulationTime;
		this.eventNumber = eventNumber;
	}
	
	public void setAnimationController(ReplayAnimationController animationController) {
		this.animationController = animationController;
	}
	
	public long getEventNumber() {
		return eventNumber;
	}

	public double getBeginSimulationTime() {
		return beginSimulationTime;
	}

	public double getEndSimulationTime() {
		return beginSimulationTime;
	}

	public void animateAt(long eventNumber, double simulationTime) {
	}
	
	protected TimerQueue getTimerQueue() {
		return animationController.getTimerQueue();
	}

	protected IFigure getRootFigure() {
		return animationController.getCanvas().getRootFigure();
	}

	protected LayoutManager getLayoutManager() {
		return getRootFigure().getLayoutManager();
	}

	protected void addFigure(IFigure figure) {
		getRootFigure().add(figure);
	}

	protected void removeFigure(IFigure figure) {
		getRootFigure().remove(figure);
	}

	protected void showFigure(IFigure figure) {
		figure.setVisible(true);

		if (figure.getParent() == null)
			addFigure(figure);
	}

	protected void hideFigure(IFigure figure) {
		//if (figure.getParent() != null)
		//	removeFigure(figure);
		
		figure.setVisible(false);
	}
	
	protected void show() {
		shown = true;
	}
	
	protected void hide() {
		shown = false;
	}

	protected void setConstraint(IFigure figure, Rectangle constraint) {
		getLayoutManager().setConstraint(figure, constraint);
	}
}