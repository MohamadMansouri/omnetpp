package org.omnetpp.ide.properties;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import org.apache.commons.lang.ArrayUtils;
import org.eclipse.core.resources.IContainer;
import org.eclipse.core.resources.IFolder;
import org.eclipse.core.resources.IMarker;
import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.IResource;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.core.runtime.IStatus;
import org.eclipse.core.runtime.Status;
import org.eclipse.jface.dialogs.ErrorDialog;
import org.eclipse.jface.preference.PreferencePage;
import org.eclipse.jface.viewers.CheckStateChangedEvent;
import org.eclipse.jface.viewers.CheckboxTreeViewer;
import org.eclipse.jface.viewers.ICheckStateListener;
import org.eclipse.swt.SWT;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Label;
import org.eclipse.ui.dialogs.PropertyPage;
import org.eclipse.ui.model.WorkbenchContentProvider;
import org.eclipse.ui.model.WorkbenchLabelProvider;
import org.omnetpp.common.project.ProjectUtils;
import org.omnetpp.ide.Activator;

/**
 * This property page is shown for OMNeT++ Projects (projects with the
 * omnetpp nature), and lets the user edit the contents of the ".nedfolders" 
 * file.
 *
 * @author Andras
 */
public class NEDSourceFoldersPropertyPage extends PropertyPage {
	private CheckboxTreeViewer treeViewer;

	/**
	 * Constructor.
	 */
	public NEDSourceFoldersPropertyPage() {
		super();
	}

	/**
	 * @see PreferencePage#createContents(Composite)
	 */
	protected Control createContents(Composite parent) {
		Composite composite = new Composite(parent, SWT.NONE);
		composite.setLayoutData(new GridData(SWT.FILL, SWT.FILL, true, true));
		composite.setLayout(new GridLayout(2, false));

		final IProject project = (IProject) getElement();

		Label label = new Label(composite, SWT.NONE);
		label.setText("NED source folders in project " + project.getName() + ":");
		label.setLayoutData(new GridData(SWT.FILL, SWT.FILL, true, false, 2, 1));
		
		treeViewer = new CheckboxTreeViewer(composite, SWT.BORDER);
		treeViewer.getTree().setLayoutData(new GridData(SWT.FILL, SWT.FILL, true, false));
		((GridData)treeViewer.getTree().getLayoutData()).heightHint = 300;
		
		treeViewer.setLabelProvider(new WorkbenchLabelProvider());
		
		treeViewer.setContentProvider(new WorkbenchContentProvider() {
	        @Override
	        public Object[] getChildren(Object element) {
	            List<Object> filteredChildren = new ArrayList<Object>();
	            for (Object child : super.getChildren(element)) {
	                if (child==project || child instanceof IFolder)
	                    filteredChildren.add(child);
	            }
	            return filteredChildren.toArray();
	        }
	    });
	
		treeViewer.addCheckStateListener(new ICheckStateListener() {
			public void checkStateChanged(CheckStateChangedEvent event) {
				if (event.getChecked()) {
			        // when a folder is checked, uncheck its subtree and all parents up to the root  
					IResource e = (IResource)event.getElement();
					treeViewer.setSubtreeChecked(e, false);
					treeViewer.setChecked(e, true);
				    for (IResource current=e.getParent(); current!=null; current=current.getParent())
				    	treeViewer.setChecked(current, false);
				}
			}
		});

		treeViewer.setInput(project.getParent());

		loadNedFoldersFile();
		
		return composite;
	}

	protected void performDefaults() {
		treeViewer.setAllChecked(false);
		IProject project = (IProject) getElement();
		treeViewer.setChecked(project, true);
	}
	
	public boolean performOk() {
		saveNedFoldersFile();
		return true;
	}

	private void loadNedFoldersFile() {
		try {
			IProject project = (IProject) getElement();
			IContainer[] folders = ProjectUtils.readNedFoldersFile(project);

			for (IContainer folder : folders) {
				treeViewer.setChecked(folder, true);
				for (IResource current=folder.getParent(); current!=null; current=current.getParent())
					treeViewer.setExpandedState(current, true);
			}
			if (treeViewer.getCheckedElements().length == 0)
				treeViewer.setChecked(project, true);
		} 
		catch (IOException e) {
			errorDialog("Cannot read NED Source Paths: ", e);
		} catch (CoreException e) {
			errorDialog("Cannot read NED Source Paths: ", e);
		}
		
	}

	private void saveNedFoldersFile() {
		try {
			IProject project = (IProject)getElement();
			IContainer[] folders = (IContainer[]) ArrayUtils.addAll(new IContainer[]{}, treeViewer.getCheckedElements());
			ProjectUtils.saveNedFoldersFile(project, folders);
		} 
		catch (CoreException e) {
			errorDialog("Cannot store NED Source Folder list: ", e);
		}
	} 

	private void errorDialog(String message, Throwable e) {
		IStatus status = new Status(IMarker.SEVERITY_ERROR, Activator.PLUGIN_ID, e.getMessage(), e);
		ErrorDialog.openError(Display.getCurrent().getActiveShell(), "Error", message, status);
	}
}