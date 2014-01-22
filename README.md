vmdevctl
========

Connect or disconnect virutal devices from VMware virtual host.

On some environment you should add the following properties in advance
to use this command.

    isolation.device.connectable.disable = "FALSE"
    isolation.device.edit.disable = "FALSE"

See http://pubs.vmware.com/vsphere-55/index.jsp#com.vmware.vsphere.vm_admin.doc/GUID-B8AEEAAC-5E0D-4A5E-974E-64FE81949AE0.html
for these properties.

This program is using the technical information from
[VMB Back](https://sites.google.com/site/chitchatvmback/index).
