<?xml version="1.0" encoding="UTF-8"?>
<xswt xmlns:x="http://sweet_swt.sf.net/xswt">

  <import xmlns="http://sweet_swt.sf.net/xswt">
    <package name="java.lang"/>
    <package name="org.eclipse.swt.widgets" />
    <package name="org.eclipse.swt.graphics" />
    <package name="org.eclipse.swt.layout" />
    <package name="org.omnetpp.common.wizard.support" />
    <package name="org.omnetpp.cdt.wizard.support" />
  </import>
  <layout x:class="GridLayout" x:numColumns="1"/>
  <x:children>
    <text x:style="MULTI|READ_ONLY|WRAP|BORDER">
        <layoutData x:class="GridData" horizontalAlignment="FILL" x:grabExcessHorizontalSpace="true"/>
        <text x:p0="This Example Project Wizard demonstrates how to write a wizard that
encapsulates a topology generator Java library. The library (a .jar file) is bundled
with the wizard, and gets loaded when the wizard is activated."/>
    </text>
  </x:children>

</xswt>
