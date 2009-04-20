package org.omnetpp.runtimeenv.editors;

import java.util.ArrayList;
import java.util.List;

import org.eclipse.core.runtime.Assert;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.draw2d.IFigure;
import org.eclipse.draw2d.LayoutListener;
import org.eclipse.draw2d.XYLayout;
import org.eclipse.draw2d.geometry.Dimension;
import org.eclipse.draw2d.geometry.Rectangle;
import org.eclipse.jface.action.MenuManager;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.jface.viewers.StructuredSelection;
import org.eclipse.swt.SWT;
import org.eclipse.swt.custom.ScrolledComposite;
import org.eclipse.swt.events.ControlAdapter;
import org.eclipse.swt.events.ControlEvent;
import org.eclipse.swt.events.MenuDetectEvent;
import org.eclipse.swt.events.MenuDetectListener;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Event;
import org.eclipse.swt.widgets.Listener;
import org.eclipse.ui.IEditorInput;
import org.eclipse.ui.IEditorSite;
import org.eclipse.ui.PartInitException;
import org.eclipse.ui.part.EditorPart;
import org.omnetpp.common.ui.FigureCanvas;
import org.omnetpp.common.ui.SelectionProvider;
import org.omnetpp.experimental.simkernel.swig.cObject;
import org.omnetpp.figures.ITooltipTextProvider;

/**
 * 
 * @author Andras
 */
//XXX snap to grid for the move/resize?
public class ModelCanvas extends EditorPart implements ISelectionRequestHandler {
    public static final String EDITOR_ID = "org.omnetpp.runtimeenv.editors.ModelCanvas";
    protected ScrolledComposite sc;
    protected FigureCanvas canvas;
    protected List<IInspectorPart> inspectors = new ArrayList<IInspectorPart>();

    @Override
    public void init(IEditorSite site, IEditorInput input) throws PartInitException {
        if (!(input instanceof BlankCanvasEditorInput))
            throw new PartInitException("Invalid input: it must be a module in the simulation");
        
        setSite(site);
        setInput(input);
        setPartName(input.getName());
        
        site.setSelectionProvider(new SelectionProvider());
    }

    @Override
    public void dispose() {
    	for (IInspectorPart inspectorPart : inspectors.toArray(new IInspectorPart[]{}))
    		removeInspectorPart(inspectorPart);
    	super.dispose();
    }
    
    @Override
    public void createPartControl(Composite parent) {
        sc = new ScrolledComposite(parent, SWT.V_SCROLL | SWT.H_SCROLL);
        
        // create canvas
        canvas = new FigureCanvas(sc, SWT.DOUBLE_BUFFERED);
        sc.setContent(canvas);
        canvas.setBackground(new Color(null, 235, 235, 235));
        canvas.getRootFigure().setLayoutManager(new XYLayout());

        // recalculate canvas size when figures change or editor area gets resized
        canvas.getRootFigure().addLayoutListener(new LayoutListener.Stub() {
            @Override
            public void postLayout(IFigure container) {
                recalculateCanvasSize();
            }
        });
        sc.addControlListener(new ControlAdapter() {
            @Override
            public void controlResized(ControlEvent e) {
                recalculateCanvasSize();
            }
        });
        
        // create context menu
        final MenuManager contextMenuManager = new MenuManager("#popup");
        canvas.setMenu(contextMenuManager.createContextMenu(canvas));
        canvas.addMenuDetectListener(new MenuDetectListener() {
            @Override
            public void menuDetected(MenuDetectEvent e) {
                contextMenuManager.removeAll();
                Point p = canvas.toControl(e.x, e.y);
                IInspectorPart inspectorPart = InspectorPart.findInspectorPartAt(canvas, p.x, p.y);
                if (inspectorPart != null)
                	inspectorPart.populateContextMenu(contextMenuManager, p);
            }
        });
        
        // SWT-based tooltip for figures
        Listener listener = new Listener() {
            public void handleEvent(Event e) {
                switch (e.type) {
                    case SWT.MouseMove: canvas.setToolTipText(""); break;
                    case SWT.MouseHover: canvas.setToolTipText(getFigureTooltip(e.x, e.y)); break;
                }
            }       
        };
        canvas.addListener(SWT.MouseMove, listener);
        canvas.addListener(SWT.MouseHover, listener);
    }

	protected void recalculateCanvasSize() {
        Dimension size = canvas.getRootFigure().getPreferredSize();
        org.eclipse.swt.graphics.Rectangle clientArea = sc.getClientArea();
        canvas.setSize(Math.max(size.width, clientArea.width), Math.max(size.height, clientArea.height));
    } 

    public void addInspectorPart(IInspectorPart inspectorPart) {
        int lastY = canvas.getRootFigure().getPreferredSize().height;
        addInspectorPart(inspectorPart, 0, lastY+5);
    }
    
    public void addInspectorPart(IInspectorPart inspectorPart, final int x, final int y) {
        final IFigure moduleFigure = inspectorPart.getFigure();
        canvas.getRootFigure().add(moduleFigure);
        canvas.getRootFigure().setConstraint(moduleFigure, new Rectangle(x, y, -1, -1));
        
        // reveal new inspector on canvas (later when layouting already took place)
        Display.getCurrent().asyncExec(new Runnable() {
            @Override
            public void run() {
                if (!sc.isDisposed())
                    reveal(moduleFigure); //XXX also select it
            }});

        // register the inspector
        inspectors.add(inspectorPart);

        // add move/resize/selection support
        new InspectorMouseListener(inspectorPart); //XXX
        
        // listen on selection changes
        inspectorPart.setSelectionRequestHandler(this);        
    }

    public void removeInspectorPart(IInspectorPart inspectorPart) {
        Assert.isTrue(inspectors.contains(inspectorPart));
        inspectors.remove(inspectorPart);
        canvas.getRootFigure().remove(inspectorPart.getFigure());
        inspectorPart.setSelectionRequestHandler(null);        
        //XXX what else?
    }
    
    public void reveal(IFigure figure) {
        Rectangle bounds = figure.getBounds(); //XXX maybe not good if coords are parent-relative
        sc.setOrigin(bounds.x, bounds.y);
    }
    
    @Override
    public void setFocus() {
        canvas.setFocus();
    }

	@Override
	@SuppressWarnings("unchecked")
	public void select(cObject object, boolean removeOthers) {
		if (removeOthers) {
			fireSelectionChange(new StructuredSelection(object));
		}
		else {
			IStructuredSelection selection = (IStructuredSelection)getSite().getSelectionProvider().getSelection();
			if (!selection.toList().contains(object)) {
				List list = new ArrayList(selection.toList());
				list.add(object);
				fireSelectionChange(new StructuredSelection(list));
			}
		}
	}

	protected String getFigureTooltip(int x, int y) {
    	for (IFigure f = canvas.getRootFigure().findFigureAt(x,y); f != null; f = f.getParent())
    		if (f instanceof ITooltipTextProvider)
    			return ((ITooltipTextProvider)f).getTooltipText(x,y);
		return null;
	}

	@Override
	public void toggleSelection(cObject object) {
		IStructuredSelection selection = (IStructuredSelection)getSite().getSelectionProvider().getSelection();
		if (selection.toList().contains(object))
			deselect(object);
		else 
			select(object, false);
	}

	@Override
	@SuppressWarnings("unchecked")
	public void deselect(cObject object) {
		IStructuredSelection selection = (IStructuredSelection)getSite().getSelectionProvider().getSelection();
		if (selection.toList().contains(object)) {
			List list = new ArrayList(selection.toList());
			list.remove(object);
			fireSelectionChange(new StructuredSelection(list));
		}
	}

	@Override
	public void deselectAll() {
		fireSelectionChange(new StructuredSelection());
	}

    protected void fireSelectionChange(IStructuredSelection selection) {
    	System.out.println("ModelCanvas: distributing selection " + selection);
    	getSite().getSelectionProvider().setSelection(selection);
    	for (IInspectorPart inspector : inspectors)
    		inspector.selectionChanged(selection);
    }

	@Override
    public void doSave(IProgressMonitor monitor) {
        // Nothing
    }

    @Override
    public void doSaveAs() {
        // Nothing
    }

    @Override
    public boolean isDirty() {
        return false;
    }

    @Override
    public boolean isSaveAsAllowed() {
        return false;
    }
}
