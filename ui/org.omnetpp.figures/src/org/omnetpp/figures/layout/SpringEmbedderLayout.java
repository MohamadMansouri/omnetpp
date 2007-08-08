package org.omnetpp.figures.layout;

import java.util.List;

import org.eclipse.draw2d.Connection;
import org.eclipse.draw2d.IFigure;
import org.eclipse.draw2d.LayoutManager;
import org.eclipse.draw2d.XYLayout;
import org.eclipse.draw2d.geometry.Dimension;
import org.eclipse.draw2d.geometry.Point;
import org.eclipse.draw2d.geometry.Rectangle;
import org.omnetpp.figures.CompoundModuleFigure;


public class SpringEmbedderLayout extends XYLayout {

	private static final int DEFAULT_MAX_WIDTH = 680;
	private static final int DEFAULT_MAX_HEIGHT = 450;
	protected IFigure edgeParent;
	protected IFigure nodeParent;
	protected long algSeed = 1971;
    private boolean requestAutoLayout = true;

	/**
	 *
	 * @param nodeParent The parent figure of the nodes
	 * @param connectionParent The parent figure of the connection figures
	 */
	public SpringEmbedderLayout(IFigure nodeParent, IFigure edgeParent) {
		super();
		this.nodeParent = nodeParent;
		this.edgeParent = edgeParent;
 	}

	/**
	 * Calls the autoLayout algorithm using all currently specified constraints.
	 * @param nodeParent The Parent figure of the nodes
	 * @param edgeParent The parent figure of the connections (usually the ConnectionLayer)
	 */
	protected AbstractGraphLayoutAlgorithm createAutoLayouter(IFigure nodeParent, IFigure edgeParent) {
        BasicSpringEmbedderLayoutAlgorithm autoLayouter = new BasicSpringEmbedderLayoutAlgorithm();
		autoLayouter.setDefaultEdgeLength(100);
		autoLayouter.setRepulsiveForce(500.0);
		autoLayouter.setMaxIterations(500);
		// set the layouting area
		int width = nodeParent.getPreferredSize().width;
		int height = nodeParent.getPreferredSize().height;
		// if the size is not present, use a default size;
		if (width <= 0) width = DEFAULT_MAX_WIDTH;
		if (height <= 0) height = DEFAULT_MAX_HEIGHT;
		//autoLayouter.setConfineToArea(width, height, 50);
		autoLayouter.setScaleToArea(width, height, 50);

		// iterate over the nodes and add them to the algorithm
		// all child figures on this layer are considered as node
		for(IFigure node : (List<IFigure>)nodeParent.getChildren()) {
			// get the associated constraint (coordinates) if any
            Rectangle constr = (Rectangle)getConstraint(node);

            if (constr == null || constr.x == Integer.MIN_VALUE && constr.y == Integer.MIN_VALUE) {
            	autoLayouter.addMovableNode(node, node.getBounds().x, node.getBounds().y, node.getPreferredSize().width, node.getPreferredSize().height);
            }
            else
            	// add as fixed node
            	autoLayouter.addFixedNode(node,
            			constr.x, constr.y,
            			constr.width, constr.height);
		}

		// iterate over the connections and add them to the algorithm
		// all child figures of type Connection on this layer are considered as edge
		for(IFigure edge : (List<IFigure>)edgeParent.getChildren())
			if (edge instanceof Connection) {
				Connection conn = (Connection)edge;
				IFigure srcFig = conn.getSourceAnchor().getOwner();
				IFigure targetFig = conn.getTargetAnchor().getOwner();
                // add the edge ONLY if it has figures at both side
                if (srcFig == null || targetFig == null)
                    continue;
				// if this is an edge coming from outside to a submodule
				if (srcFig instanceof CompoundModuleFigure) {
					autoLayouter.addEdgeToBorder(targetFig, 0);
				} // else if this is an edge going out from a submodule
				else if (targetFig instanceof CompoundModuleFigure) {
					autoLayouter.addEdgeToBorder(srcFig, 0);
				} else {  // both are submodules
					autoLayouter.addEdge(srcFig, targetFig, 0);
				}

			}


		return autoLayouter;
	}


	/**
     * Implements the algorithm to layout the components of the given container figure.
     * Each component is laid out using its own layout constraint specifying its size
     * and position. Copied from XYLayout BUT places the middle point of the children to the
     * specified constraint location.
     *
     * @see LayoutManager#layout(IFigure)
     */
    @Override
    public void layout(IFigure parent) {
        System.out.println("layout()");
        AbstractGraphLayoutAlgorithm alg = null;
        // find the place of movable nodes if auto-layout requested
        if (requestAutoLayout) {
            alg = createAutoLayouter(nodeParent, edgeParent);
            alg.setSeed(algSeed);
            alg.execute();
            requestAutoLayout = false;
        }

    	// lay out the children according to the auto-layouter
        Point offset = getOrigin(parent);
        for (IFigure f : (List<IFigure>)parent.getChildren()) {
            Rectangle bounds = new Rectangle();
            // get the computed location from the auto-layout algorithm (if any)
            Point loc = alg != null ? alg.getNodePosition(f) : null;
            // if the algorithm does not have info about this figure or the figure is fixed
            // (use the figure's location)
            bounds = ((Rectangle)getConstraint(f)).getCopy();
            if (loc != null) {
                // get the position from the algorithm (the size is still coming from the constraint)
                bounds.setLocation(loc);
            }

            // if no info is coming from the algorithm or from the layout manager just skip it
            // do not change the location
            if (bounds == null || bounds.x == Integer.MIN_VALUE || bounds.y == Integer.MIN_VALUE)
                continue;

            //FIXME is this "-1" stuff needed?
            if (bounds.width == -1 || bounds.height == -1) {
                Dimension preferredSize = f.getPreferredSize(bounds.width, bounds.height);
                bounds = bounds.getCopy();
                if (bounds.width == -1)
                    bounds.width = preferredSize.width;
                if (bounds.height == -1)
                    bounds.height = preferredSize.height;
            }
            bounds = bounds.getTranslated(offset);
            // translate to the middle of the figure
            bounds.translate(-bounds.width / 2, -bounds.height / 2);
            f.setBounds(bounds);
        }
    }

    /**
     * Returns the point (0,0) as the origin. This is required for correct freeform handling.
     * As we want to use it in a FreeformLayer
     * @see XYLayout#getOrigin(IFigure)
     */
    @Override
    public Point getOrigin(IFigure figure) {
        return new Point();
    }

	/**
	 * Set a new seed for the layouting algorithm. The next layout will use this seed
	 * @param algSeed
	 */
	public void setSeed(long algSeed) {
		this.algSeed = algSeed;
	}

	/**
	 * @return The current seed value
	 */
	public long getSeed() {
		return algSeed;
	}

	/**
	 * After calling this, the next layout process will call an auto layout process
	 */
	public void requestAutoLayout() {
	    requestAutoLayout = true;
	}
}
