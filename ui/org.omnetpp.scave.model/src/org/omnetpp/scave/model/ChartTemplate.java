package org.omnetpp.scave.model;

import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.omnetpp.scave.model.Chart.ChartType;
import org.omnetpp.scave.model.Chart.DialogPage;

/**
 * Represents a chart template.
 */
public class ChartTemplate {
    private String id;
    private String name;
    private String description;
    private ChartType chartType;
    private String iconPath;
    private int supportedResultTypes; // a bitwise OR of the constants in ResultFileManager
    private String pythonScript;
    private List<DialogPage> dialogPages;
    private int toolbarOrder = -1;
    private String toolbarIconPath;
    private Map<String,String> properties;
    private String originFolder; // workspace path or plugin-relative path (latter starts with "plugin:")

    public ChartTemplate(String id, String name, String description, ChartType chartType, String iconPath, int supportedResultTypes,
            String pythonScript, List<DialogPage> dialogPages, int toolbarOrder, String toolbarIconPath, Map<String,String> properties, String originFolder) {
        this.id = id;
        this.name = name;
        this.description = description;
        this.chartType = chartType;
        this.iconPath = iconPath;
        this.supportedResultTypes = supportedResultTypes;
        this.pythonScript = pythonScript;
        this.dialogPages = dialogPages;
        this.toolbarOrder = toolbarOrder;
        this.toolbarIconPath = toolbarIconPath;
        this.properties = properties;
        this.originFolder = originFolder;
    }

    public String getId() {
        return id;
    }


    public String getName() {
        return name;
    }

    public String getDescription() {
        return description;
    }

    public Chart.ChartType getChartType() {
        return chartType;
    }

    public String getIconPath() {
        return iconPath;
    }

    public int getSupportedResultTypes() {
        return supportedResultTypes;
    }

    public String getPythonScript() {
        return pythonScript;
    }

    public List<DialogPage> getDialogPages() {
        return Collections.unmodifiableList(dialogPages);
    }

    public int getToolbarOrder() {
        return toolbarOrder;
    }

    public String getToolbarIconPath() {
        return toolbarIconPath;
    }

    public Set<String> getPropertyNames() {
        return Collections.unmodifiableSet(properties.keySet());
    }

    public Map<String,String> getProperties() {
        return Collections.unmodifiableMap(properties);
    }

    public String getPropertyDefault(String name) {
        return properties.get(name);
    }

    public String getOriginFolder() {
        return originFolder;
    }

    @Override
    public String toString() {
        return "'" + getName() + "' (id=" + getId() + ")";
    }

}
