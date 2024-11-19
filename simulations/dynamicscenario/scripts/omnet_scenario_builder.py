from lxml import etree

root = etree.Element("script")

##element = etree.SubElement(root, "set-param",
##                        attrib={"t":"5",
##                                "module":"wirelessdevice1.app[0].io",
##                                "par":"destAddress",
##                                "value":"device2"})
##element2 = etree.SubElement(root, "set-param",
##                       attrib={"t":"5",
##                                "module":"wirelessdevice1.app[0].io",
##                                "par":"destPort",
##                                "value":"1002"})
##element3 = etree.SubElement(root, "at", attrib={"t":"2"})
##
##element4 = etree.SubElement(element3, "delete-module", attrib={
##                                    "module":"wirelessdevice2"
##    })
##
##element5 = etree.SubElement(root,"create-module",
##                            attrib={"t":"3",
##                                "parent":"Dynamicscenario",
##                                "submodule":"wirelessdevice3",
##                                "type":"inet.node.tsn.TsnDevice",
##                                 "vector":"false"
##                                    })
##element5 = etree.SubElement(root, "connect", attrib={
##                                    "t":"3.5",
##                                    "src-module":"wirelessdevice3",
##                                    "src-gate":"wirelessdevice3.ethg$i[0]",
##                                    "dest-module":"d6g.devices.detCom",
##                                    "dest-gate":"detCom.dsttg$o[2]",
##                                    "channel-type":"inet.node.ethernet.EthernetLink"})

##element4 = etree.SubElement(root, "delete-module", attrib={
##                                    "t":"10",
##                                    "module":"wirelessdevice1"})


##element6 = etree.SubElement(root, "set-histogram", attrib={
##                                "t":"30",
##                                "module":"histogramContainer",
##                                "key":"Downlink",
##                                "histogram":"../../../deterministic6g_data/PD-Wireless-5G-1/test.xml"})
##
##element7 = etree.SubElement(root, "set-param", attrib={
##                                "t":"30",
##                                "module":"detCom.dstt[1]",
##                                "par":"delayDownlink",
##                                "expr":'rngProvider("histogramContainer","Downlink")'})
##
##element8 = etree.SubElement(root, "set-histogram", attrib={
##                                "t":"60",
##                                "module":"histogramContainer",
##                                "key":"DownlinkOld",
##                                "histogram":"../../../deterministic6g_data/PD-Wireless-5G-1/s10-DL_wall.xml"})
##
##
##element9 = etree.SubElement(root, "set-param", attrib={
##                                "t":"60",
##                                "module":"detCom.dstt[1]",
##                                "par":"delayDownlink",
##                                "expr":'rngProvider("histogramContainer","DownlinkOld")'})
##
##element10 = etree.SubElement(root, "set-param", attrib={
##                                "t":"90",
##                                "module":"detCom.dstt[1]",
##                                "par":"delayDownlink",
##                                "expr":"truncnormal(0.3,0.1)*1ms"})


element11 = etree.SubElement(root, "shutdown", attrib={
                                    "t":"5",
                                    "module":"device2.app[1]"})


##### operationen

##element8 = etree.SubElement(root, "operation", attrib={
##                                "t":"50",
##                                "name":"provider"})
##
##
##element8.text = 'rngProvider("histogramContainer","Downlink")'


tree = etree.ElementTree(root)
tree.write("app-shutdown.xml", pretty_print=True, xml_declaration=True, encoding="UTF-8")
