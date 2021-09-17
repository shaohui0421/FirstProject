#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/wait.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <inttypes.h>
#include <sstream>
#include "VM_Manage.h"
#include "rc_json.h"
using namespace RcJson;
using namespace::std;

#define ESTPORT                         "5900"
#define VMMODE_INI_PATH                 "/etc/RCC_Client/vmmode.ini"
static const string CLEAR_INST =        "rm -f /opt/lessons/guest.img";
int VmManage::_mode = 1;
int VmManage::_run = 1;

VmManage::VmManage(Application* app)
    :_app(app)
    ,_process_loop(this)
    ,_vmShutdownMonitorTimer(new VmShutdownMonitorTimer(this))
    ,_xserver_exited(false)
    ,_modeInfo(NULL)
    ,_userInfo(NULL)
    ,_driver_install_parament("")
    ,_main_thread(new UnjoinableThread(VmManage::thread_main, this))
    ,_monitor_thread(new UnjoinableThread(VmManage::start_monitor, this))

{
    _allow_userdisk = -1;
}

VmManage::~VmManage()
{
    _vmShutdownMonitorTimer->unref();
    _main_thread->cancel();
    delete _main_thread;
    _monitor_thread->cancel();
    delete _monitor_thread;
}

void* VmManage::start_monitor(void *data)
{
    while(1)
    {
        sleep(1);
        monitor_vm_shutdown();
    }
}

void VmManage::action()
{
    cout << "test" << endl;
}

/*
**thread main
*/
void* VmManage::thread_main(void* data)
{
    VmManage* vmManage = (VmManage*)data;
    vmManage->_process_loop.run();
    return NULL;
}
/*
**import.xml serial_port parallel_port net_passthrough
** json --> xml
**infos={"hostdev":["eth1"],"parallel":["parport0"],"serial":["ttyS0","ttyS1","ttyS2","ttyS3"],
**       "kk":"11","ee":{"jj":"we2","fg":"er4","hy":["t5t"],"hu":{"e3":"3e"}}}
**×ª»»£º<hostdev>eth1</hostdev>
**    <parallel>parport0</parallel>
**    <serial>ttyS0</serial>
**    <serial>ttyS1</serial>
**    <serial>ttyS2</serial>
**    <serial>ttyS3</serial>
**    <kk>11</kk>
**    <ee jj="we2" fg="er4">
**      <hy>t5t</hy>
**      <hu e3="3e"/>
**    </ee>
*/
void VmManage::create_dev_inter_xml(xmlNodePtr &devicesNode, const string& infos, string xmlname, bool isObject, bool isList)
{
    if (infos != ""){
        cJSON* json_array = NULL;
        string childinfo = "";
        string name = "";
        //child node
        cJSON *json_ssid = NULL;
        json_array = cJSON_Parse(infos.c_str());
        int size = cJSON_GetArraySize(json_array);
        LOG_DEBUG("json child size: %d\n", size);
    
        for (int i = 0; i < size; i++) {
            //example:json_ssid --> "hostdev":["eth1"]
            //example:cJSON_Print(json_ssid)-->["eth1"]
            //example:json_ssid->string --> "hostdev"
            json_ssid = cJSON_GetArrayItem(json_array, i);
            LOG_WARNING(cJSON_Print(json_ssid));
            if (json_ssid == NULL) {
                LOG_WARNING("cJSON_GetArrayItem get null");
                continue;
            }
            childinfo = cJSON_Print(json_ssid);
            //Dictionary or not {}
            if (infos.find("{") != string::npos){
                name = json_ssid->string;
            }
            // parent node name and list£º
            if (xmlname == ""|| !isList){
                xmlname = name;
            }
            //Whether the child node is an object--> name:{}
            if(cJSON_Object == json_ssid->type) {
                LOG_WARNING("cJSON_GetArrayItem cJSON_Object");
                xmlNodePtr interfaceNode = xmlNewNode(NULL, BAD_CAST name.c_str());
                create_dev_inter_xml(interfaceNode, childinfo, name, true);
                xmlAddChild(devicesNode,interfaceNode);   
            }
            else if (childinfo.find("[") != string::npos) {
                //childinfo = ["",""] list Recursive processing
                create_dev_inter_xml(devicesNode, childinfo, name, false, true);
            }
            else if (childinfo != ""){
                //remove ""
                if (childinfo.find('"') != string::npos){
                    childinfo = childinfo.replace(childinfo.find('"'), 1, "");
                    childinfo = childinfo.replace(childinfo.find('"'), 1, "");
                }
                //generate XML, example : <hostdev>eth1</hostdev>
                if (childinfo != "" && !isObject) {
                    xmlNodePtr nodeptr = xmlNewNode(NULL, BAD_CAST xmlname.c_str()); 
                    xmlNodePtr deviceContent = xmlNewText(BAD_CAST childinfo.c_str());
                    xmlAddChild(devicesNode, nodeptr);
                    xmlAddChild(nodeptr, deviceContent);
                }else if (childinfo != "") {
                    //example: <hu e3="3e"/>
                    xmlNewProp(devicesNode, BAD_CAST name.c_str(), BAD_CAST childinfo.c_str());
                }
            }
        }
        cJSON_Delete(json_array);
    }  
}
#if 0
void VmManage::testxml()
{
    xmlDocPtr doc = xmlNewDoc(BAD_CAST"1.0");
    xmlNodePtr root_node = xmlNewNode(NULL, BAD_CAST "domain");
    xmlDocSetRootElement(doc, root_node);
    xmlNodePtr devicesNode = xmlNewNode(NULL,BAD_CAST "devices");
    xmlAddChild(root_node,devicesNode);
    string inter_name = "";
    Application* app = Application::get_application();
    DevInterfaceInfo inter_info;
    app->get_UsrUserInfoMgr()->get_dev_interface_info(inter_info);        
    create_dev_inter_xml(devicesNode,inter_info.interface_info,inter_name );
    xmlSaveFormatFileEnc(XML_FILE.c_str(),doc, "UTF-8", 1);
    xmlFreeDoc(doc);
}
#endif

void VmManage::create_parament_xml_file(struct ImportXMLInfo &param)
{
#define XML_VMMODE_PASS                     "passthrough"
#define XML_VMMODE_WINDOWS_XP               "Windowsxp"
#define XML_VMMODE_WINDOWS_10               "Windows10"

    //stringstream vcpu_stream;
    //vcpu_stream<<param.vcpu;
    //const bool NEED_VCPU = true;
    stringstream vm_is_driver;
    const bool NEED_DEVICES = true;
    const bool NEED_VMMODE = true;
    bool virtio_flag = true;
    bool extend_mem_flag = true;
    unsigned int passlist_size = 0;
    stringstream qxl_num_stream;

    // first to check virtio then judge user disk clear status, can not to exchange the order
    _vm_check_virtio_flag(param.base_file, param.img_file, &virtio_flag, &extend_mem_flag);
    //_vm_get_vm_start_quickly_mem(extend_mem_flag);

    Application* app = Application::get_application();

    //init xml
    xmlDocPtr doc = xmlNewDoc(BAD_CAST"1.0");
    xmlNodePtr root_node = xmlNewNode(NULL, BAD_CAST "domain");
    xmlDocSetRootElement(doc, root_node);
#if 0
    if(NEED_VCPU)
    {
        xmlNodePtr vcpuNode = xmlNewNode(NULL, BAD_CAST "vcpu");
        xmlNodePtr vcpuContent = xmlNewText(BAD_CAST vcpu_stream.str().c_str());
        xmlAddChild(root_node,vcpuNode);
        xmlAddChild(vcpuNode,vcpuContent);
        xmlNewProp(vcpuNode,BAD_CAST "placement",BAD_CAST "static");
    }

    int vm_mem_size = (!param.driver_iso.empty()) ? 2048 : _calc_vm_memory_size(param.vmmode);
    stringstream vm_mem_stream;
    vm_mem_stream<<vm_mem_size;
    const bool NEED_MEMORY = (vm_mem_size != 0);
    if(NEED_MEMORY)
    {
        xmlNodePtr memoryNode = xmlNewNode(NULL, BAD_CAST "memory");
        xmlNodePtr memoryContent = xmlNewText(BAD_CAST vm_mem_stream.str().c_str());
        xmlAddChild(root_node, memoryNode);
        xmlAddChild(memoryNode, memoryContent);
        xmlNewProp(memoryNode, BAD_CAST "unit", BAD_CAST "MiB");
        xmlNewProp(memoryNode, BAD_CAST "extend", BAD_CAST (extend_mem_flag ? "1" : "0"));
    }
#endif
    const bool NEED_DRIVER_ISO = !param.driver_iso.empty();
    vm_is_driver<<(NEED_DRIVER_ISO ? 1 : 0);

    xmlNodePtr driverisoNode = xmlNewNode(NULL, BAD_CAST "driver_install"); 
    xmlNodePtr driverContent = xmlNewText(BAD_CAST vm_is_driver.str().c_str());
    xmlAddChild(root_node, driverisoNode);
    xmlAddChild(driverisoNode, driverContent);

    string res_opt_w, res_opt_h;
    if (NEED_VMMODE) {
        xmlNodePtr vmmodeNode = xmlNewNode(NULL,BAD_CAST "vmmode");
        xmlAddChild(root_node, vmmodeNode);

        xmlNodePtr bootNode = xmlNewNode(NULL, BAD_CAST "boot");
        xmlAddChild(vmmodeNode, bootNode);

        xmlNodePtr osNode = xmlNewNode(NULL, BAD_CAST "os");
        xmlAddChild(vmmodeNode, osNode);

        // format changed
        if (param.vmmode == VMMODE_EMULATION_INI) {
            param.vmmode = XML_VMMODE_EMULATION;
        } else {
            param.vmmode = XML_VMMODE_PASS;
        }
        if (param.ostype == OSTYPE_WINDOWS_XP) {
            param.ostype = XML_VMMODE_WINDOWS_XP;
        } else if (param.ostype == OSTYPE_WINDOWS_7) {
            param.ostype = XML_VMMODE_WINDOWS_7;
        } else if (param.ostype == OSTYPE_WINDOWS_10) {
            param.ostype = XML_VMMODE_WINDOWS_10;
        }

        xmlNewProp(bootNode, BAD_CAST "product", BAD_CAST "oa");
        xmlNewProp(bootNode, BAD_CAST "mode", BAD_CAST param.vmmode.c_str());
        xmlNewProp(osNode, BAD_CAST "type", BAD_CAST param.ostype.c_str());

        LOG_DEBUG("create xml passlist size %d",  param.passlist.size());
        for (passlist_size = 0; passlist_size < param.passlist.size(); passlist_size++) {
            xmlNodePtr hostdevNode = xmlNewNode(NULL, BAD_CAST "hostdev");
            xmlAddChild(vmmodeNode,hostdevNode);
            xmlNewProp(hostdevNode, BAD_CAST "passlist", BAD_CAST param.passlist[passlist_size].c_str());
        }

        xmlNodePtr estNode = xmlNewNode(NULL, BAD_CAST "est");
        xmlAddChild(vmmodeNode,estNode);
        xmlNewProp(estNode, BAD_CAST "port", BAD_CAST ESTPORT);

        xmlNodePtr xrandrNode = xmlNewNode(NULL, BAD_CAST "xrandr");
        xmlAddChild(vmmodeNode,xrandrNode);
        app->get_device_interface()->getDisplayBestResolution(0, res_opt_w, res_opt_h);
        if (res_opt_w == param.screen_w && res_opt_h == param.screen_h) {
            xmlNewProp(xrandrNode, BAD_CAST "x", BAD_CAST "0");
            xmlNewProp(xrandrNode, BAD_CAST "y", BAD_CAST "0");
        } else {
            xmlNewProp(xrandrNode, BAD_CAST "x", BAD_CAST param.screen_w.c_str());
            xmlNewProp(xrandrNode, BAD_CAST "y", BAD_CAST param.screen_h.c_str());
        }

        if (param.vmmode == XML_VMMODE_EMULATION) {
            xmlNodePtr qxl = xmlNewNode(NULL, BAD_CAST "qxl");
            xmlAddChild(vmmodeNode,qxl);

            // Windows XP not support 2 qxl devices
            if (param.ostype == XML_VMMODE_WINDOWS_XP) {
                param.qxl_num = 1;
            }
            qxl_num_stream<<param.qxl_num;
            xmlNewProp(qxl, BAD_CAST "qxlnum", BAD_CAST qxl_num_stream.str().c_str());

            if (param.qxl_num >= 2) {
                xmlNodePtr qxlscreen1 = xmlNewNode(NULL, BAD_CAST "qxlscreen1");
                xmlAddChild(vmmodeNode,qxlscreen1);
                xmlNewProp(qxlscreen1, BAD_CAST "x", BAD_CAST param.qxl_screen1_w.c_str());
                xmlNewProp(qxlscreen1, BAD_CAST "y", BAD_CAST param.qxl_screen1_h.c_str());

                xmlNodePtr qxlscreen2 = xmlNewNode(NULL, BAD_CAST "qxlscreen2");
                xmlAddChild(vmmodeNode,qxlscreen2);
                xmlNewProp(qxlscreen2, BAD_CAST "x", BAD_CAST param.qxl_screen2_w.c_str());
                xmlNewProp(qxlscreen2, BAD_CAST "y", BAD_CAST param.qxl_screen2_h.c_str());
            }
        }
    }
    
    if(NEED_DEVICES)
    {
        xmlNodePtr devicesNode = xmlNewNode(NULL,BAD_CAST "devices");
        xmlAddChild(root_node,devicesNode);

        const bool NEED_HDA = true;
        if(NEED_HDA)
        {
            xmlNodePtr diskNode_hda = xmlNewNode(NULL, BAD_CAST "disk");
            xmlAddChild(devicesNode,diskNode_hda);
                    
            xmlNodePtr sourceNode_hda = xmlNewNode(NULL, BAD_CAST "source");
            xmlAddChild(diskNode_hda,sourceNode_hda);
            xmlNewProp(sourceNode_hda, BAD_CAST "file", BAD_CAST param.img_file.c_str());

            xmlNodePtr expandNode = xmlNewNode(NULL, BAD_CAST "expand");
            xmlAddChild(diskNode_hda,expandNode);
            xmlNewProp(expandNode, BAD_CAST "size", BAD_CAST "0G");

            xmlNodePtr backingStoreNode = xmlNewNode(NULL, BAD_CAST "backingStore");
            xmlAddChild(diskNode_hda,backingStoreNode);
            xmlNodePtr sourceBackingNode = xmlNewNode(NULL, BAD_CAST "source");
            xmlAddChild(backingStoreNode,sourceBackingNode);
            xmlNewProp(sourceBackingNode, BAD_CAST "file", BAD_CAST param.base_file.c_str());
            
            xmlNodePtr targetNode_hda = xmlNewNode(NULL, BAD_CAST "target");
            xmlAddChild(diskNode_hda,targetNode_hda);
            xmlNewProp(targetNode_hda, BAD_CAST "dev", BAD_CAST (virtio_flag ? "vda" : "hda"));
            xmlNewProp(targetNode_hda, BAD_CAST "bus", BAD_CAST (virtio_flag ? "virtio" : "ide"));
        }

        const bool NEED_HDB = param.data_disk;
        const bool NEED_HDC = param.is_layer_exist;
        const bool NEED_HDD = param.is_print;
        if(NEED_DRIVER_ISO)
        {
            xmlNodePtr diskNode_driver_iso = xmlNewNode(NULL, BAD_CAST "disk");
            xmlAddChild(devicesNode, diskNode_driver_iso);
            xmlNewProp(diskNode_driver_iso, BAD_CAST "type", BAD_CAST "file");
            xmlNewProp(diskNode_driver_iso, BAD_CAST "device", BAD_CAST "cdrom");
            
            xmlNodePtr driverNode_driver_iso = xmlNewNode(NULL, BAD_CAST "driver");
            xmlAddChild(diskNode_driver_iso, driverNode_driver_iso);
            xmlNewProp(driverNode_driver_iso, BAD_CAST "name", BAD_CAST "qemu");
            xmlNewProp(driverNode_driver_iso, BAD_CAST "type", BAD_CAST "raw");
            xmlNewProp(driverNode_driver_iso, BAD_CAST "cache", BAD_CAST "writeback");
            
            xmlNodePtr sourceNode_driver_iso = xmlNewNode(NULL, BAD_CAST "source");
            xmlAddChild(diskNode_driver_iso, sourceNode_driver_iso);
            xmlNewProp(sourceNode_driver_iso, BAD_CAST "file", BAD_CAST param.driver_iso.c_str());
            
            xmlNodePtr targetNode_driver_iso = xmlNewNode(NULL, BAD_CAST "target");
            xmlAddChild(diskNode_driver_iso, targetNode_driver_iso);
            xmlNewProp(targetNode_driver_iso, BAD_CAST "dev", BAD_CAST "hdb");
            xmlNewProp(targetNode_driver_iso, BAD_CAST "bus", BAD_CAST "ide");
        }
        else if(NEED_HDB)
        {
            xmlNodePtr diskNode_hdb = xmlNewNode(NULL, BAD_CAST "disk");
            xmlAddChild(devicesNode,diskNode_hdb);
            xmlNodePtr sourceNode_hdb = xmlNewNode(NULL, BAD_CAST "source");
            xmlAddChild(diskNode_hdb,sourceNode_hdb);
            xmlNewProp(sourceNode_hdb, BAD_CAST "file", BAD_CAST USER_DISK_FILE.c_str());
            xmlNodePtr targetNode_hdb = xmlNewNode(NULL, BAD_CAST "target");
            xmlAddChild(diskNode_hdb,targetNode_hdb);
            xmlNewProp(targetNode_hdb, BAD_CAST "dev", BAD_CAST (virtio_flag ? "vdb" : "hdb"));
            xmlNewProp(targetNode_hdb, BAD_CAST "bus", BAD_CAST (virtio_flag ? "virtio" : "ide"));
        }

        if (!NEED_DRIVER_ISO && NEED_HDC) {
            xmlNodePtr diskNode_hdc = xmlNewNode(NULL, BAD_CAST "disk");
            xmlAddChild(devicesNode,diskNode_hdc);

            xmlNodePtr sourceNode_hdc = xmlNewNode(NULL, BAD_CAST "source");
            xmlAddChild(diskNode_hdc,sourceNode_hdc);
            xmlNewProp(sourceNode_hdc, BAD_CAST "file", BAD_CAST param.layer_file.c_str());

            xmlNodePtr seriallNode_hdc = xmlNewNode(NULL, BAD_CAST "serial");
            xmlAddChild(diskNode_hdc, seriallNode_hdc);
            xmlNodePtr serialContent = xmlNewText(BAD_CAST param.layer_serial.c_str());
            xmlAddChild(seriallNode_hdc, serialContent);

            xmlNodePtr backingStoreLayerNode = xmlNewNode(NULL, BAD_CAST "backingStore");
            xmlAddChild(diskNode_hdc,backingStoreLayerNode);
            xmlNodePtr sourceBackingLayerNode = xmlNewNode(NULL, BAD_CAST "source");
            xmlAddChild(backingStoreLayerNode,sourceBackingLayerNode);
            xmlNewProp(sourceBackingLayerNode, BAD_CAST "file", BAD_CAST LAYER_TEMPLETE_FILE.c_str());

            xmlNodePtr targetNode_hdc = xmlNewNode(NULL, BAD_CAST "target");
            xmlAddChild(diskNode_hdc,targetNode_hdc);
            xmlNewProp(targetNode_hdc, BAD_CAST "dev", BAD_CAST (virtio_flag ? "vdc" : "hdc"));
            xmlNewProp(targetNode_hdc, BAD_CAST "bus", BAD_CAST (virtio_flag ? "virtio" : "ide"));

            xmlNodePtr typeNode_hdc = xmlNewNode(NULL, BAD_CAST "driver");
            xmlAddChild(diskNode_hdc,typeNode_hdc);
            xmlNewProp(typeNode_hdc, BAD_CAST "type", BAD_CAST "qcow2");
        }

        if (!NEED_DRIVER_ISO && NEED_HDD) {
            xmlNodePtr diskNode_hdd = xmlNewNode(NULL, BAD_CAST "disk");
            xmlAddChild(devicesNode,diskNode_hdd);

            xmlNodePtr sourceNode_hdd = xmlNewNode(NULL, BAD_CAST "source");
            xmlAddChild(diskNode_hdd, sourceNode_hdd);
            xmlNewProp(sourceNode_hdd, BAD_CAST "file", BAD_CAST PRINT_DISK_FILE.c_str());

            xmlNodePtr targetNode_hdd = xmlNewNode(NULL, BAD_CAST "target");
            xmlAddChild(diskNode_hdd,targetNode_hdd);
            xmlNewProp(targetNode_hdd, BAD_CAST "dev", BAD_CAST (virtio_flag ? "vdd" : "hdd"));
            xmlNewProp(targetNode_hdd, BAD_CAST "bus", BAD_CAST (virtio_flag ? "virtio" : "ide"));

            xmlNodePtr typeNode_hdd = xmlNewNode(NULL, BAD_CAST "driver");
            xmlAddChild(diskNode_hdd,typeNode_hdd);
            xmlNewProp(typeNode_hdd, BAD_CAST "type", BAD_CAST "qcow2");
        }

        const bool NEED_INTERFACE = true;
        if(NEED_INTERFACE)
        {
            string nat_type;
            if(app->get_nat_policy())
            {
                nat_type = "nat";
            }
            else
            {
                nat_type = "bridge";
            }
            xmlNodePtr interfaceNode = xmlNewNode(NULL, BAD_CAST "interface");
            xmlNewProp(interfaceNode, BAD_CAST "type", BAD_CAST nat_type.c_str());
            xmlAddChild(devicesNode,interfaceNode);
            xmlNodePtr macNode = xmlNewNode(NULL, BAD_CAST "mac");
            xmlAddChild(interfaceNode,macNode);
            xmlNewProp(macNode, BAD_CAST "address", BAD_CAST param.vm_mac.c_str());
            if (param.vm_mac2 != ""){
                xmlNodePtr interfaceNode2 = xmlNewNode(NULL, BAD_CAST "interface2");
                xmlNewProp(interfaceNode2, BAD_CAST "type", BAD_CAST "bridge");
                xmlAddChild(devicesNode,interfaceNode2);
                xmlNodePtr macNode2 = xmlNewNode(NULL, BAD_CAST "mac");
                xmlAddChild(interfaceNode2,macNode2);
                xmlNewProp(macNode2, BAD_CAST "address", BAD_CAST param.vm_mac2.c_str());
            }
        }

        bool need_virtio_setup = false;
        if (!param.driver_iso.empty()) {
            need_virtio_setup = true;
        } else if (virtio_flag == false && vm_is_vm_recovery() == false) {
            need_virtio_setup = true;
        }
        if (need_virtio_setup)
        {
            xmlNodePtr virtioNode = xmlNewNode(NULL, BAD_CAST "virtio_setup");
            xmlAddChild(devicesNode, virtioNode);
        }
        create_dev_inter_xml(devicesNode, param.inter_info,"");

    }

    const bool NEED_OFFLINE = _app->is_localmode_startvm();
    if (NEED_OFFLINE)
    {
        xmlNodePtr offlineNode = xmlNewNode(NULL, BAD_CAST "offline");
        xmlAddChild(root_node, offlineNode);
    }

    //win7 emulation mode, add usb_emulation and e1000 netcard node
    if (param.ostype == XML_VMMODE_WINDOWS_7 && param.vmmode == XML_VMMODE_EMULATION && (!_app->is_dev_wlan_up())) {
        const bool using_e1000_netcard = app->get_UsrUserInfoMgr()->is_using_e1000_netcard();
        xmlNodePtr e1000Node = xmlNewNode(NULL, BAD_CAST "e1000_nic"); 
        xmlNodePtr e1000Content = xmlNewText(BAD_CAST (using_e1000_netcard ? "1" : "0"));
        xmlAddChild(root_node, e1000Node);
        xmlAddChild(e1000Node, e1000Content);
    }

    if (param.ostype == XML_VMMODE_WINDOWS_7 && param.vmmode == XML_VMMODE_EMULATION) {
        const bool is_usb_emulation = app->get_UsrUserInfoMgr()->is_usb_emulation();
        xmlNodePtr usbEmuNode = xmlNewNode(NULL, BAD_CAST "usb_emulation"); 
        xmlNodePtr usbEmuContent = xmlNewText(BAD_CAST (is_usb_emulation ? "1" : "0"));
        xmlAddChild(root_node, usbEmuNode);
        xmlAddChild(usbEmuNode, usbEmuContent);
    }

    xmlSaveFormatFileEnc(XML_FILE.c_str(),doc, "UTF-8", 1);
    xmlFreeDoc(doc);

#undef XML_VMMODE_PASS
#undef XML_VMMODE_WINDOWS_XP
#undef XML_VMMODE_WINDOWS_10
}

/*
**push set_vm event
*/
bool VmManage::vm_set_VM(ModeInfo* modeInfo, UserInfo* userInfo, string imageName, bool recovery, string mac, \
               string sn, int allow_userdisk, const string& driver_install_parament, string mac2)
{
    int ret;
    _vm_sn     = sn;
    _modeInfo  = modeInfo;
    _userInfo  = userInfo;
    _imageName = imageName;
    _recovery  = recovery;
    _vm_mac    = mac;
    _vm_mac2   = mac2;
    _allow_userdisk = allow_userdisk;
    _layer_info.layer_on_1 = "N";
    // default close printer
     _printerdisk_info.is_enable = 0;
    _app->get_vm()->vm_get_vm_layerdisk_info(_layer_info);
    _app->get_UsrUserInfoMgr()->get_disk_info(PRINTER_DISK_NAME, _printerdisk_info);

    // add emulation memory then this config is not used
   // _reserved_memory = _app->get_reserved_memory_data().get();

    _driver_install_parament = driver_install_parament;
    LOG_INFO("login user:%s, while binduser is:%s,modeInfo mode is:%d",_userInfo->username.c_str(),_modeInfo->bind_user.username.c_str(),_modeInfo->mode);

    SetVmEvent* event = new SetVmEvent(this);
    ret = _process_loop.push_event(event);
    event->unref();
    if (ret < 0){
    	return false;
    }else {
    	return true;
    }
}

/*
**push shutdown vm normal event
*/
bool VmManage::vm_shutdown_VM_normal()
{
	int ret;
    ShutDownVmNormalEvent* event = new ShutDownVmNormalEvent(this);
    ret = _process_loop.push_event(event);
    event->unref();
    if (ret < 0){
    	return false;
    }else {
    	return true;
    }
}

/*
**push shutdown vm event
*/
bool VmManage::vm_shutdown_VM()
{
	int ret;
    ShutDownVmEvent* event = new ShutDownVmEvent(this);
    ret = _process_loop.push_event(event);
    event->unref();
    if (ret < 0){
    	return false;
    }else {
    	return true;
    }
}

/*
**push check base exist event
*/
bool VmManage::vm_check_base_exist()
{
	int ret;
    CheckBaseExistEvent* event = new CheckBaseExistEvent(this);
    ret = _process_loop.push_event(event);
    event->unref();
    if (ret < 0){
    	return false;
    }else {
    	return true;
    }
}

/*
 * NOTE:this api only use when vm is running
 */
bool VmManage::vm_is_vm_recovery()
{
    bool vm_recovery;
    ModeInfo mode_info;
    UserInfo user_info;
    if(_modeInfo == NULL)
    {
        mode_info = _app->get_mode_info();
    }
    else
    {
        mode_info = *_modeInfo;
    }
    if(_userInfo == NULL)
    {
        user_info = _app->get_logined_user_info();
    }
    else
    {
        user_info = *_userInfo;
    }

    if(_recovery == true)//recovery mode
    {
        vm_recovery = true;
    }
    else 
    {
        if((mode_info.mode == SPECIAL_MODE && mode_info.bind_user.username == user_info.username)
            || (mode_info.mode == MULTIUSER_MODE && user_info.username != "guest")
            || (mode_info.mode == PUBLIC_MODE))
        {
            vm_recovery = false;
        }else
        {
            vm_recovery = true;
        }
    }
    return vm_recovery;
}

bool VmManage::vm_is_xserver_exited()
{
    return get_file_exist("/tmp/.xserver_forbidden");
}


/*
**start vm
*/
void VmManage::set_VM()
{
    if(is_vm_running())
    {
        LOG_INFO("No need to start VM: vm is running!");
        return;
    }

    string check_base_result = execute_command(CMD_BASE_NUM);

    // default is failed for other START_FAIL case
    _startvm_errcode = START_VM_FAILED;
    if(check_base_result == "error" || atoi(check_base_result.c_str()) == 0) {
        LOG_ERR("Failed to start VM: base does not exist!");
        _startvm_errcode = START_VM_FAILED;
        _app->vm_start_vm_status(START_FAIL);
        return;
    } else {
        // qemu-img check local base and personal img are valid or not
        string check_base_valid = VMMANAGER_CHECK +_imageName + BASE_FORMAT;
        int base_valid_ret = rc_system(check_base_valid.c_str());
        if(base_valid_ret == 1 || base_valid_ret == 2) {
            LOG_CRIT("Failed to start VM: base image is damaged!");
            _startvm_errcode = START_VM_FAILED;
            _app->vm_start_vm_status(START_FAIL);
            return;
        }
        // FIXME,if personal img is damage, we do not allow guest.img to provide?
        if(_app->get_vm()->vm_check_personal_img_exist()) {
            string check_img_valid = VMMANAGER_CHECK + _imageName + IMG_FORMAT;
            int img_valid_ret = rc_system(check_img_valid.c_str());
            if(img_valid_ret == 1 || img_valid_ret == 2) {
                LOG_CRIT("Failed to start VM: personal image is damaged!");
                _startvm_errcode = START_VM_FAILED;
                _app->vm_start_vm_status(START_FAIL);
                return;
            }
        }

        // if layer disk is exist and is recovery then we should check it
        if (!vm_is_vm_recovery() && _app->get_vm()->vm_check_layerdisk_exist()) {
            string check_layer_valid = VMMANAGER_CHECK + _imageName + LAYER_FORMAT;
            int layer_valid_ret = rc_system(check_layer_valid.c_str());
            if(layer_valid_ret == 1 || layer_valid_ret == 2) {
                LOG_CRIT("Failed to start VM: layer is damaged!");
                _startvm_errcode = START_VM_FAILED;
                _app->vm_start_vm_status(START_FAIL);
                return;
            }
        }

        _startvm_errcode = _get_image_ostype_vmmode(_vmmode, false);
        if (_startvm_errcode != START_VM_CHECK_OK) {
            _app->vm_start_vm_status(START_FAIL);
            return;
        }

        if (_vmmode != VMMODE_EMULATION_INI && _vmmode != VMMODE_PASS_INI) {
            _startvm_errcode = START_NOT_SUPPORT_CPU;
            _app->vm_start_vm_status(START_FAIL);
            return;
        }
        string isE1000 = "0";
        string ostype = "";
        string ostypevm = "";
        string vmmodevm = _vmmode;
        _app->get_vm()->vm_get_image_ostype(ostype);
        ostypevm = ostype;
        if (ostypevm == OSTYPE_WINDOWS_7) {
            ostypevm = XML_VMMODE_WINDOWS_7;
        }
        if (vmmodevm == VMMODE_EMULATION_INI) {
            vmmodevm = XML_VMMODE_EMULATION;
        }
        if ((ostypevm == XML_VMMODE_WINDOWS_7) && vmmodevm == XML_VMMODE_EMULATION && (!_app->is_dev_wlan_up())) {
            const bool using_e1000_netcard =_app->get_UsrUserInfoMgr()->is_using_e1000_netcard();
            if (using_e1000_netcard) {
                isE1000 = "1";
            }
        }
        //check ini is exist
        _create_acpi_file(ACPI_CONF, false, isE1000);

        //create import.xml
        LOG_DEBUG("to create xml imageName:%s", _imageName.c_str());

        struct ImportXMLInfo paramxml;
        string img_file = (vm_is_vm_recovery()) ? GUEST_IMG_NAME : (_imageName + IMG_FORMAT);
        string layer_file;

       // paramxml.vcpu = _get_vm_cpu_num();
        paramxml.base_file = IMAGE_PATH + _imageName + BASE_FORMAT;
        paramxml.img_file = IMAGE_PATH + img_file;
        paramxml.driver_iso = "";
        paramxml.data_disk = _has_user_disk();
        paramxml.vm_mac = _vm_mac;
        paramxml.vm_mac2 = _vm_mac2;
        paramxml.vmmode = _vmmode;
        paramxml.layer_file = "";
        paramxml.is_layer_exist = false;
        DevInterfaceInfo inter_info;
        _app->get_UsrUserInfoMgr()->get_dev_interface_info(inter_info);
        paramxml.inter_info = inter_info.interface_info;
        if (inter_info.net_passthrough == 0){
            paramxml.vm_mac2 = "";
        }  
        paramxml.ostype = ostype;

        if (_printerdisk_info.is_enable && _app->get_vm()->vm_check_expand_disk_exist(_printerdisk_info.disk_name)) {
            paramxml.is_print = true;
        } else {
            paramxml.is_print = false;
        }

        // if login is recover mode then not need write layer to vm
        if (!vm_is_vm_recovery()) {
            if (_has_create_layer()) {
                paramxml.layer_file = IMAGE_PATH + _imageName + LAYER_FORMAT;
                paramxml.layer_serial = _layer_info.layer_disk_serial_1;
                paramxml.is_layer_exist = true;
            }
        }

        _get_vmmode_passlist(paramxml.passlist, false);
        _app->get_device_interface()->getCurResolution(paramxml.screen_w, paramxml.screen_h);

        std::map<string, int> ext_reslist[5];
        if (_app->get_device_interface()->getExtDisplayResolutionList(ext_reslist) < 2) {
            paramxml.qxl_num = 1;
        } else {
            paramxml.qxl_num = 2;
        }
        _app->get_UsrUserInfoMgr()->get_ext_display_info(0, paramxml.qxl_screen1_w, paramxml.qxl_screen1_h, ext_reslist[0]);
        _app->get_UsrUserInfoMgr()->get_ext_display_info(1, paramxml.qxl_screen2_w, paramxml.qxl_screen2_h, ext_reslist[1]);
        create_parament_xml_file(paramxml);

        _startvm_errcode = START_VM_FAILED;
        //check xml exist
        bool xmlFileExist = get_file_exist(XML_FILE.c_str());
        if(xmlFileExist == false)
        {
            LOG_ERR("Failed to start VM: set vm while %s does not exist!", XML_FILE.c_str());
            _startvm_errcode = START_VM_FAILED;
            _app->vm_start_vm_status(START_FAIL);
        }else{
            LOG_INFO("%s _recovery:%d", __FUNCTION__, _recovery);
            //if dev policy is recovery, clear inst
            if (_recovery) {
                _app->get_vm()->vm_clear_inst();
                _app->get_vm()->vm_clear_layer();
            }
            //clear guest.inst first
            LOG_INFO("clear guest.img before start vm");
            int clear_result = clear_inst_guest();
            if(clear_result == -1)
            {
                LOG_ERR("Failed to start VM: clear the inst of guest fail before set vm!");
                _startvm_errcode = START_VM_FAILED;
                _app->vm_start_vm_status(START_FAIL);
                return;
            }

            if (!vm_is_vm_recovery()) {
                if (_has_create_layer()) {
                   if (!_app->get_vm()->vm_layer_templete_handle(RCC_LAYER_FILE)) {
                        _startvm_errcode = START_LAYER_TEMPLETE_ERR;
                        _app->vm_start_vm_status(START_FAIL);
                        return;
                   }

                   layer_file = _imageName + LAYER_FORMAT;
                   if (!_app->get_vm()->vm_create_layer_disk(layer_file)) {
                        _startvm_errcode = START_CREATE_CDISK_ERR;
                        _app->vm_start_vm_status(START_FAIL);
                        return;
                   }

                   // To prevent disk full,then clear inst and create new disk
                   if (_app->get_vm()->judge_disk_isfull(_imageName)) {
                        _app->get_vm()->vm_clear_inst();
                   }
                }
            }

            if (!_app->get_vm()->vm_create_c_disk(img_file)) {
                // add ui tips 
                _startvm_errcode = START_CREATE_CDISK_ERR;
                _app->vm_start_vm_status(START_FAIL);
                return;
            }

            _app->set_net_disk_port();

            _stop_xserver();    // stop X server before VM starts.
            int set_vm_ret = rc_system(SET_VM.c_str());
            
            LOG_INFO("command: %s, ret: %d", SET_VM.c_str(), set_vm_ret);

            if(set_vm_ret == 0) {
                LOG_INFO("VM starts sucessfully!");
                _app->vm_start_vm_status(START_SUCCESS);
                DevInterfaceInfo inter_info;
                _app->get_UsrUserInfoMgr()->get_dev_interface_info(inter_info);
                if (inter_info.net_passthrough == 1){
                    if ( _app->get_net2_status()){
                        _app->get_vm()->vm_enable_net2use();
                    }else {
                        _app->get_vm()->vm_disable_net2use();
                    }
                }
            } else if (set_vm_ret == 103) {
                LOG_EMERG("Failed to start VM: set vm fail! ret = %d", set_vm_ret);
                rc_system("touch /root/.rcc_exception_reboot");
                rc_system("sync");
                if (_vmmode == VMMODE_EMULATION_INI) {
                    rc_system("/etc/RCC_Client/scripts/back_to_client_callback.sh");
                } else {
                    rc_system("/usr/bin/back_to_client.sh");
                }
            } else {
                LOG_EMERG("Failed to start VM: set vm fail! ret = %d", set_vm_ret);
                if (set_vm_ret == 157) {
                    _startvm_errcode = START_INTEL_NO_AUDIO_DEVICE;
                } else {
                    _startvm_errcode = START_VM_FAILED;
                }
                _app->vm_start_vm_status(START_FAIL);
            }
        }
    }
}

void VmManage::set_VM_driver_install()
{
    if(is_vm_running())
    {
        LOG_INFO("No need to start VM: vm is running!");
        return;
    }

    //create import.xml
    struct ImportXMLInfo paramxml = {0};
    string ostype = "";

    _startvm_errcode = START_VM_FAILED;
    //paramxml.vcpu = _get_vm_cpu_num();
    paramxml.base_file = rc_json_get_string(_driver_install_parament, "path_name") + "/" + rc_json_get_string(_driver_install_parament, "base_name") + BASE_FORMAT;
    paramxml.img_file = rc_json_get_string(_driver_install_parament, "path_name") + "/" + rc_json_get_string(_driver_install_parament, "base_name") + IMG_FORMAT;

    paramxml.driver_iso = "";
    if((!rc_json_get_string(_driver_install_parament, "driver_path_name").empty()) && (!rc_json_get_string(_driver_install_parament, "driver_name").empty()))
        paramxml.driver_iso   = rc_json_get_string(_driver_install_parament, "driver_path_name") + "/" + rc_json_get_string(_driver_install_parament, "driver_name");

    ostype = rc_json_get_string(_driver_install_parament, "ostype");
    paramxml.data_disk = false;
    paramxml.vm_mac = _vm_mac;
    paramxml.vm_mac2 = _vm_mac2;
    paramxml.ostype = ostype;

    // if ostype field not exist  then default vmmode is Passthrough
    if (ostype.empty()) {
        _vmmode = VMMODE_PASS_INI;
        LOG_DEBUG("set driver install ostype is empty");
    } else {
        _startvm_errcode = _get_image_ostype_vmmode(_vmmode, true, ostype);
        LOG_DEBUG("set driver install vmmode %s", _vmmode.c_str());
        if (_startvm_errcode != START_VM_CHECK_OK) {
            _app->vm_start_vm_status(START_FAIL);
            return;
        }
    }

#if 0
    // judge if vmmode adapt this cpu and ostype ,emulation vmmode not need to install driver then we should send this msg to web
    if(_vmmode != VMMODE_PASS_INI) {
        _startvm_errcode = START_DRIVER_OSTYPE_NOTADAPT;
        _app->vm_start_vm_status(START_FAIL);
        return;
    }
#endif
    DevInterfaceInfo inter_info;
    _app->get_UsrUserInfoMgr()->get_dev_interface_info(inter_info);
    paramxml.inter_info = inter_info.interface_info;
    if (inter_info.net_passthrough == 0){
        paramxml.vm_mac2 = "";
    }
    paramxml.vmmode = _vmmode;
    _get_vmmode_passlist(paramxml.passlist, true, ostype);
    _app->get_device_interface()->getCurResolution(paramxml.screen_w, paramxml.screen_h);

    map<string, int> ext_reslist[5];
    if (_app->get_device_interface()->getExtDisplayResolutionList(ext_reslist) < 2) {
        paramxml.qxl_num = 1;
    } else {
        paramxml.qxl_num = 2;
    }
     _app->get_UsrUserInfoMgr()->get_ext_display_info(0, paramxml.qxl_screen1_w, paramxml.qxl_screen1_h, ext_reslist[0]);
    _app->get_UsrUserInfoMgr()->get_ext_display_info(1, paramxml.qxl_screen2_w, paramxml.qxl_screen2_h, ext_reslist[1]);
    string isE1000 = "0";
    string ostypevm = paramxml.ostype;
    string vmmodevm = paramxml.vmmode;
    if (ostypevm == OSTYPE_WINDOWS_7) {
        ostypevm = XML_VMMODE_WINDOWS_7;
    }
    if (vmmodevm == VMMODE_EMULATION_INI) {
        vmmodevm = XML_VMMODE_EMULATION;
    }
    if (ostypevm == XML_VMMODE_WINDOWS_7 && vmmodevm == XML_VMMODE_EMULATION && (!_app->is_dev_wlan_up())) {
        const bool using_e1000_netcard =_app->get_UsrUserInfoMgr()->is_using_e1000_netcard();
        LOG_DEBUG("using_e1000_netcard sd:%d", using_e1000_netcard);
        if (using_e1000_netcard) {
            isE1000 = "1";
        }
    }
     //we donnot care about whether base file is OK in nfs
    _create_acpi_file(ACPI_CONF, true, isE1000);
    create_parament_xml_file(paramxml);

    _startvm_errcode = START_VM_FAILED;
    //check xml exist
    bool xmlFileExist = get_file_exist(XML_FILE.c_str());
    if(xmlFileExist == false) {
        LOG_ERR("Failed to start VM: set vm while %s does not exist!", XML_FILE.c_str());
        _startvm_errcode = START_VM_FAILED;
        _app->vm_start_vm_status(START_FAIL);
    } else {
        _stop_xserver();    // stop X server before VM starts.
        int set_vm_ret = rc_system(SET_VM.c_str());
        LOG_INFO("command: %s, ret: %d", SET_VM.c_str(), set_vm_ret);
        
        if (set_vm_ret == 0) {
            LOG_INFO("VM starts sucessfully!");
            _app->vm_start_vm_status(START_SUCCESS);
            DevInterfaceInfo inter_info;
            _app->get_UsrUserInfoMgr()->get_dev_interface_info(inter_info);
            if (inter_info.net_passthrough == 1){
                if ( _app->get_net2_status()){
                    _app->get_vm()->vm_enable_net2use();
                }else {
                    _app->get_vm()->vm_disable_net2use();
                }
            }
        } else {
            LOG_EMERG("Failed to start VM: set vm fail! ret = %d", set_vm_ret);
            if (set_vm_ret == 157) {
                _startvm_errcode = START_INTEL_NO_AUDIO_DEVICE;
            } else {
                _startvm_errcode = START_VM_FAILED;
            }
            _app->vm_start_vm_status(START_FAIL);
        }
    }
}


/*
**shutdown vm normal
*/
void VmManage::shutdown_VM_normal()
{
    //retry 2 times to shutdown vm normal
    for(int num = 0; num < 2; num++)
    {
        LOG_INFO("shutdown vm normal try time:%d",num+1);
        if(is_vm_running())
        {
            rc_system(SHUTDOWN_VM_NORMAL.c_str());
            LOG_INFO("shutdown vm normal command execute!");
        }
        sleep(45);
    }
    //FIXME
    //we assert vm shutdown in 2 times shutdown normal, if vm is still runnig, we shutdown vm force
    if(is_vm_running())
    {
        LOG_INFO("vm will be shutdown force!");
        shutdown_VM();
    }
}

/*
**shutdown vm
*/
void VmManage::shutdown_VM()
{
    string result = execute_command(SHUTDOWN_VM);
    if(result == "error")
    {
        LOG_ERR("shutdown vm fail!");
		_app->vm_shutdown_vm_status(SHUTDOWN_FAIL);
    }else{
        LOG_INFO("shutdown vm command execute!");
    }
}

/*
**clear guest.inst
*/
int VmManage::clear_inst_guest()
{
    string result = execute_command(CLEAR_INST);
    if(result == "error")
    {
        LOG_ERR("clear inst of guest fail!")
        return -1;
    }else{
        return 0;
    }
}

/*
**check /opt/lessons has .base file or not
*/
void VmManage::check_base_exist()
{
    string result = execute_command(CMD_BASE_NUM);
    if(result == "error")
    {
        LOG_ERR("check base exist fail!");
        _app->vm_image_exist_status(NO_EXIST);
    }else{
        if(atoi(result.c_str()) == 0)
        {

            LOG_DEBUG("not base file in path:/opt/lessons");
            _app->vm_image_exist_status(NO_EXIST);
        }else{
            _app->vm_image_exist_status(EXIST);
        }
    }
}


/*
 * check vm is running or not
 */
bool VmManage::is_vm_running()
{
    string result = execute_command("ps aux | grep qemu-system-x86_64 | grep -v grep | wc -l");
    if (result == "error") {
        LOG_INFO("check vm running fail!");
        return false;
    } else {
        //LOG_DEBUG("check vm result: %s", result.c_str());
        int count = atoi(result.c_str());
        if (count > 0) {
            LOG_INFO("vm is running!");
            return true;
        } else {
            LOG_INFO("vm is not running!");
            return false;
        }
    }
}

/**
*function :check vmmode err type
*
*return : err code about vmmode
*/
int VmManage::get_start_vm_err()
{
    return _startvm_errcode;
}

/**
*function : judget if vmmode is emulation
*is_driver: get vmmode value wether or not when driver installing
*
*int : 0: start vmmode is not emulation
*/
int VmManage::get_start_vm_is_emulation(bool is_driver, string driver_ostype)
{
    string vmmode;
    int ret = START_VM_CHECK_OK;

    if (!is_driver) { 
        ret = _get_image_ostype_vmmode(vmmode, is_driver);
    } else {
        if (driver_ostype == "") {
            vmmode = VMMODE_PASS_INI;
            ret = START_VM_CHECK_OK;
        } else {
            ret = _get_image_ostype_vmmode(vmmode, is_driver, driver_ostype);
        }
    }

    LOG_DEBUG("vmmode : %s", vmmode.c_str());
    if (ret == START_VM_CHECK_OK) {
        if (vmmode != VMMODE_EMULATION_INI) {
            return 0;
        } else {
            return 1;
        }
    } else {
        return 0;
    }
}

void VmManage::restart_nat()
{
    LOG_INFO("VmManage::restart_nat");
    int ret = rc_system("vmmanager --restartnat");
    if (ret != 0) {
        LOG_ERR("restartnat fail!")
    }
}
void VmManage::switch_nat()
{
    LOG_INFO("VmManage::switch_nat");
    int ret = rc_system("vmmanager --switchnat");
    if (ret != 0) {
        //ret=13 means not running vm
        //ret=36 means no need switch
        LOG_ERR("switchnat fail!")
    }
}
void VmManage::switch_bridge()
{
    LOG_INFO("VmManage::switch_bridge");
    int ret = rc_system("vmmanager --switchbridge");
    if (ret != 0) {
        //ret=13 means not running vm
        //ret=36 means no need switch
        LOG_ERR("switchbridge fail!")
    }
}

void VmManage::connectClose(virConnectPtr conn ATTRIBUTE_UNUSED,
                         int reason,
                         void *opaque ATTRIBUTE_UNUSED)
{
    switch (reason) {
    case VIR_CONNECT_CLOSE_REASON_ERROR:
        fprintf(stderr, "Connection closed due to I/O error\n");
        break;
    case VIR_CONNECT_CLOSE_REASON_EOF:
        fprintf(stderr, "Connection closed due to end of file\n");
        break;
    case VIR_CONNECT_CLOSE_REASON_KEEPALIVE:
        fprintf(stderr, "Connection closed due to keepalive timeout\n");
        break;
    case VIR_CONNECT_CLOSE_REASON_CLIENT:
        fprintf(stderr, "Connection closed due to client request\n");
        break;
    default:
        fprintf(stderr, "Connection closed due to unknown reason\n");
        break;
    };
    _run = 0;
}

void VmManage::stop(int sig)
{
    LOG_DEBUG("Exiting on signal %d\n", sig);
    _run = 0;
}

int VmManage::monitor_vm_shutdown()
{
    int ret_lifecycle      = -1;
    int ret_reboot         = -1;

    struct sigaction action_stop;
    memset(&action_stop,0,sizeof(action_stop));
    action_stop.sa_handler = stop;

    if(virInitialize() < 0){
        LOG_ERR("Failed to initialize libvirt!");
        return -1;
    }
    if(virEventRegisterDefaultImpl() < 0)
    {
        virErrorPtr err = virGetLastError();
        LOG_ERR("Failed to register event implementation:%s!",err && err->message ? err->message : "Unknown error");
        return -1;
    }

    virConnectPtr dconn = NULL; 
    dconn = virConnectOpenAuth(NULL, virConnectAuthPtrDefault, VIR_CONNECT_RO);
    if(!dconn){
        LOG_DEBUG("error opening libvirtd");
        rc_system("service libvirtd restart");
        sleep(2);
        return -1;
    }
    _run =1;
    virConnectRegisterCloseCallback(dconn, connectClose, NULL, NULL);
    //ctrl+c and kill do not need monitor
    //sigaction(SIGTERM, &action_stop, NULL);
    //sigaction(SIGINT, &action_stop, NULL);
    LOG_DEBUG("Registering event cbs");

    ret_lifecycle  = virConnectDomainEventRegisterAny(dconn,
                                                    NULL,
                                                    VIR_DOMAIN_EVENT_ID_LIFECYCLE,
                                                    VIR_DOMAIN_EVENT_CALLBACK(domainEventLifeCycleCallback),
													strdup("callback life cycle"), freeFunc);
    ret_reboot     = virConnectDomainEventRegisterAny(dconn,
                                                      NULL,
                                                      VIR_DOMAIN_EVENT_ID_REBOOT,
                                                      VIR_DOMAIN_EVENT_CALLBACK(domainEventRebootCallback),
                                                      strdup("callbask reboot"),freeFunc);


    if((ret_lifecycle != -1) && (ret_reboot != -1))  {
        if (virConnectSetKeepAlive(dconn, 5, 3) < 0) {
            virErrorPtr err = virGetLastError();
            fprintf(stderr, "Failed to start keepalive protocol: %s\n",
                    err && err->message ? err->message : "Unknown error");
            _run = 0;
        }

        while (_run) {
            if (virEventRunDefaultImpl() < 0) {
                virErrorPtr err = virGetLastError();
                fprintf(stderr, "Failed to run event loop: %s\n",
                        err && err->message ? err->message : "Unknown error");
            }
        }
        LOG_DEBUG("Deregistering event handlers");
        virConnectDomainEventDeregisterAny(dconn, ret_lifecycle);
        virConnectDomainEventDeregisterAny(dconn, ret_reboot);
    }

    virConnectUnregisterCloseCallback(dconn, connectClose);

    LOG_DEBUG("Closing connection");
    if (dconn && virConnectClose(dconn) < 0) {
        LOG_DEBUG("error closing\n");
    }
    LOG_DEBUG("done\n");
    return 0;
}

void VmManage::freeFunc(void *opaque)
{
    char* str = (char*)opaque;
    LOG_DEBUG("%s: Freeing [%s]\n", __func__, str);
    free(str);
}

int VmManage::domainEventLifeCycleCallback(virConnectPtr conn ATTRIBUTE_UNUSED,
                                  virDomainPtr dom,
                                  int event,
                                  int detail,
                                  void *opaque ATTRIBUTE_UNUSED)
{
    LOG_DEBUG("%s EVENT: Domain %s(%d) %s %s\n", __func__, virDomainGetName(dom),
           virDomainGetID(dom), eventToString(event).c_str(),
           eventDetailToString(event, detail).c_str());
    return 0;
}

int VmManage::domainEventRebootCallback(virConnectPtr conn ATTRIBUTE_UNUSED,
                                        virDomainPtr dom,
                                        void *opaque ATTRIBUTE_UNUSED)
{
    LOG_DEBUG("%s EVENT: Domain %s(%d)\n", __func__, virDomainGetName(dom),virDomainGetID(dom));
    execute_cmd_after_vm_reboot();
    return 0;
}

string VmManage::eventToString(int event) {
    string ret = "";
    switch ((virDomainEventType) event) {
        case VIR_DOMAIN_EVENT_DEFINED:
            ret = "Defined";
            break;
        case VIR_DOMAIN_EVENT_UNDEFINED:
            ret = "Undefined";
            break;
        case VIR_DOMAIN_EVENT_STARTED:
            ret = "Started";
            break;
        case VIR_DOMAIN_EVENT_SUSPENDED:
            ret = "Suspended";
            break;
        case VIR_DOMAIN_EVENT_RESUMED:
            ret = "Resumed";
            break;
        case VIR_DOMAIN_EVENT_STOPPED:
            ret = "Stopped";
            break;
        case VIR_DOMAIN_EVENT_SHUTDOWN:
            ret = "Shutdown";
            break;
        case VIR_DOMAIN_EVENT_PMSUSPENDED:
            ret = "PMSuspended";
            break;
        case VIR_DOMAIN_EVENT_CRASHED:
            ret = "Crashed";
            break;
    }
    return ret;
}

string VmManage::eventDetailToString(int event, int detail) {
    string ret = "";
    switch ((virDomainEventType) event) {
        case VIR_DOMAIN_EVENT_DEFINED:
            if (detail == VIR_DOMAIN_EVENT_DEFINED_ADDED)
                ret = "Added";
            else if (detail == VIR_DOMAIN_EVENT_DEFINED_UPDATED)
                ret = "Updated";
            break;
        case VIR_DOMAIN_EVENT_UNDEFINED:
            if (detail == VIR_DOMAIN_EVENT_UNDEFINED_REMOVED)
                ret = "Removed";
            break;
        case VIR_DOMAIN_EVENT_STARTED:
            switch ((virDomainEventStartedDetailType) detail) {
            case VIR_DOMAIN_EVENT_STARTED_BOOTED:
                ret = "Booted";
                break;
            case VIR_DOMAIN_EVENT_STARTED_MIGRATED:
                ret = "Migrated";
                break;
            case VIR_DOMAIN_EVENT_STARTED_RESTORED:
                ret = "Restored";
                break;
            case VIR_DOMAIN_EVENT_STARTED_FROM_SNAPSHOT:
                ret = "Snapshot";
                break;
            case VIR_DOMAIN_EVENT_STARTED_WAKEUP:
                ret = "Event wakeup";
                break;
            }
            break;
        case VIR_DOMAIN_EVENT_SUSPENDED:
            switch ((virDomainEventSuspendedDetailType) detail) {
            case VIR_DOMAIN_EVENT_SUSPENDED_PAUSED:
                ret = "Paused";
                break;
            case VIR_DOMAIN_EVENT_SUSPENDED_MIGRATED:
                ret = "Migrated";
                break;
            case VIR_DOMAIN_EVENT_SUSPENDED_IOERROR:
                ret = "I/O Error";
                break;
            case VIR_DOMAIN_EVENT_SUSPENDED_WATCHDOG:
                ret = "Watchdog";
                break;
            case VIR_DOMAIN_EVENT_SUSPENDED_RESTORED:
                ret = "Restored";
                break;
            case VIR_DOMAIN_EVENT_SUSPENDED_FROM_SNAPSHOT:
                ret = "Snapshot";
                break;
            case VIR_DOMAIN_EVENT_SUSPENDED_API_ERROR:
                ret = "API error";
                break;
            }
            break;
        case VIR_DOMAIN_EVENT_RESUMED:
            switch ((virDomainEventResumedDetailType) detail) {
            case VIR_DOMAIN_EVENT_RESUMED_UNPAUSED:
                ret = "Unpaused";
                break;
            case VIR_DOMAIN_EVENT_RESUMED_MIGRATED:
                ret = "Migrated";
                break;
            case VIR_DOMAIN_EVENT_RESUMED_FROM_SNAPSHOT:
                ret = "Snapshot";
                break;
            }
            break;
        case VIR_DOMAIN_EVENT_STOPPED:
            switch ((virDomainEventStoppedDetailType) detail) {
            case VIR_DOMAIN_EVENT_STOPPED_SHUTDOWN:
                LOG_INFO("VIR_DOMAIN_EVENT_STOPPED_SHUTDOWN");
                ret = "Shutdown";
                execute_cmd_after_vm_shutdown();
                break;
            case VIR_DOMAIN_EVENT_STOPPED_DESTROYED:
                LOG_INFO("VIR_DOMAIN_EVENT_STOPPED_DESTROYED");
                ret = "Destroyed";
                execute_cmd_after_vm_shutdown();
                break;
            case VIR_DOMAIN_EVENT_STOPPED_CRASHED:
                LOG_INFO("VIR_DOMAIN_EVENT_STOPPED_CRASHED");
                ret = "Crashed";
                break;
            case VIR_DOMAIN_EVENT_STOPPED_MIGRATED:
                LOG_INFO("VIR_DOMAIN_EVENT_STOPPED_MIGRATED");
                ret = "Migrated";
                break;
            case VIR_DOMAIN_EVENT_STOPPED_SAVED:
                LOG_INFO("VIR_DOMAIN_EVENT_STOPPED_SAVED");
                ret = "Saved";
                break;
            case VIR_DOMAIN_EVENT_STOPPED_FAILED:
                LOG_INFO("VIR_DOMAIN_EVENT_STOPPED_FAILED");
                ret = "Failed";
                execute_cmd_after_vm_shutdown();
                break;
            case VIR_DOMAIN_EVENT_STOPPED_FROM_SNAPSHOT:
                LOG_INFO("VIR_DOMAIN_EVENT_STOPPED_FROM_SNAPSHOT");
                ret = "Snapshot";
                break;
            }
            break;
        case VIR_DOMAIN_EVENT_SHUTDOWN:
            switch ((virDomainEventShutdownDetailType) detail) {
            case VIR_DOMAIN_EVENT_SHUTDOWN_FINISHED:
                ret = "Finished";
                break;
            }
            break;
        case VIR_DOMAIN_EVENT_PMSUSPENDED:
            switch ((virDomainEventPMSuspendedDetailType) detail) {
            case VIR_DOMAIN_EVENT_PMSUSPENDED_MEMORY:
                ret = "Memory";
                break;
            case VIR_DOMAIN_EVENT_PMSUSPENDED_DISK:
                ret = "Disk";
                break;
            }
            break;
        case VIR_DOMAIN_EVENT_CRASHED:
           switch ((virDomainEventCrashedDetailType) detail) {
           case VIR_DOMAIN_EVENT_CRASHED_PANICKED:
               ret = "Panicked";
               break;
           }
           break;
    }
    return ret;
}

/**
 * clear inst 
*/
void VmManage::execute_cmd_after_vm_shutdown()
{
	Application *app = Application::get_application();

	LOG_INFO("clear guest.img after shut down vm");
    int clear_result = clear_inst_guest();
    if (clear_result == -1) {
        LOG_ERR("clear the inst of guest fail!");
        app->vm_shutdown_vm_status(SHUTDOWN_FAIL);
    } else {
        LOG_INFO("============ VM shutdown OK ============");
        app->vm_shutdown_vm_status(SHUTDOWN_SUCCESS);
    }
    //TODO img combine
    //TODO: if kernel supports unbind graphic chip, X server will restart.
    //_start_xserver();
}

/**
 * retransimit vm info to guesttool
 * netdisk/sunny info
 * usb/net policy
 */
void VmManage::execute_cmd_after_vm_reboot()
{
    //FIXME
    //this api will do twice when vm reboot
    Application *app = Application::get_application();

    LOG_DEBUG("cmd after vm reboot");
    LOG_INFO("============ VM reboot OK ============");
    app->vm_reboot_vm_status(REBOOT_SUCCESS);
}

void VmManage::_start_xserver()
{
    // delete file .xserver_forbidden, rcc_startx.sh will restart X server soon.
    rc_system("rm -f /tmp/.xserver_forbidden");
    sleep(1);
    _xserver_exited = false;
}

void VmManage::_stop_xserver()
{
    LOG_DEBUG("Stopping X server ...\n");
    //_app->vm_ui_thread_quit();
    _xserver_exited = true;
    LOG_DEBUG("Stopping X server over");
}

void __del_space_and_emptyline(const string& filename)
{
    const string tmp_file = "/tmp/del_space_and_emptyline";
    string command;
    command = "sed s/[[:space:]]//g " + filename + " > " + tmp_file;
    rc_system(command.c_str());
    command = "sed '/^\\s*$/d' " + tmp_file + " > " + filename;
    rc_system(command.c_str());
    return;
}
void VmManage::_create_acpi_file(const string& filename, bool edit,const string& isE1000)
{
    char buf[16];
    string printer_manager;
    string desktop_redir = "Y";
    
    /* the Guest tool need this to expand c disk */
    ImageInfo imageInfo;
    string cDiskSize;
    
#if 0
    string command_buf = "echo host=rcd > " + filename;
    rc_system(command_buf.c_str());
    return;
#endif
    //common part
    dictionary *d = dictionary_new(0);
    iniparser_set(d, "config", "");
    iniparser_set(d, "config:arch",         "IDV");
    iniparser_set(d, "config:host",         "TERM");
    iniparser_set(d, "config:vmname",       _vm_sn.c_str());
    iniparser_set(d, "config:rcdip",        _app->get_server_ip().c_str());
   
    if (_app->get_vm() != NULL ) {
        imageInfo = _app->get_vm()->vm_get_vm_imageinfo();
        cDiskSize = std::to_string(imageInfo.virt_size);
        iniparser_set(d, "config:sysdisksize", cDiskSize.c_str()); 
        LOG_DEBUG("in acpi sysdisksize is %s.", cDiskSize.c_str());
    } else {
        LOG_ERR("No vm found");
    }
    
    if (edit) {
        iniparser_set(d, "config:run",      "BASE");
    } else {
        iniparser_set(d, "config:run",      "USER");
        iniparser_set(d, "config:username", _userInfo->username.c_str());
        iniparser_set(d, "config:vmtype",   (vm_is_vm_recovery() ? "RESTORE" : "SAVE"));
        if (!_userInfo->pcname.empty()) {
            iniparser_set(d, "config:pcname", _userInfo->pcname.c_str());
        }
        if (vm_is_vm_recovery()) {
            _app->get_UsrUserInfoMgr()->get_vm_desktop_redir(desktop_redir);
            iniparser_set(d, "config:desktopRedirect", desktop_redir.c_str());
        }
    }

    // layer part
    if (!vm_is_vm_recovery()) {
        if (!edit && _has_create_layer()) {
            iniparser_set(d, "layer", "");
            sprintf(buf, "%d", _layer_info.layer_disk_number);
            iniparser_set(d, "layer:layer_disk_number", buf);
            iniparser_set(d, "layer:layer_disk_serial_1", _layer_info.layer_disk_serial_1.c_str());
            iniparser_set(d, "layer:layer_on_1", "Y");
            iniparser_set(d, "layer:layer_x64_1", _layer_info.layer_x64_1.c_str());
        }
    }

    //idv part
    iniparser_set(d, "idv", "");
    iniparser_set(d, "idv:product_id",      _app->get_basic_info().product_id.c_str());
    iniparser_set(d, "idv:product_name",    _app->get_basic_info().product_name.c_str());
    iniparser_set(d, "idv:sn",              _app->get_basic_info().serial_number.c_str());
    iniparser_set(d, "idv:version",         _app->get_basic_info().software_version.c_str());
    iniparser_set(d, "idv:bios_version",         _app->get_basic_info().bios_version.c_str());
    iniparser_set(d, "idv:term_name",       _app->get_hostname().c_str());
    iniparser_set(d, "idv:termip",          _app->get_local_networkinfo().ip.c_str());
    iniparser_set(d, "idv:mac",             _vm_mac.c_str());
    iniparser_set(d, "idv:e1000",           isE1000.c_str());
    if (!_driver_install_parament.empty()) {
        iniparser_set(d, "idv:install_driver",  (rc_json_get_int(_driver_install_parament, "action") == 2) ? "manual" : "auto");
    }

    if (!edit) {
        string base_file = _imageName  + BASE_FORMAT;
        iniparser_set(d, "idv:img",             base_file.c_str());
        // decode && encode
        string decode_key, key;
        decode_key = gloox::password_codec_xor(_userInfo->password, false);
        key = gloox::password_codec(decode_key, true);
        iniparser_set(d, "idv:key",             key.c_str());
        iniparser_set(d, "idv:password_xor",    _userInfo->password.c_str());
        sprintf(buf, "%d", _modeInfo->mode);
        iniparser_set(d, "idv:idvmode",         buf);
        sprintf(buf, "%d", _app->is_localmode_startvm());
        iniparser_set(d, "idv:localmode",       buf);
        sprintf(buf, "%d", get_random());
        iniparser_set(d, "idv:random",          buf);
    }

    LOG_INFO("acpi vmmode %s", _vmmode.c_str());
    iniparser_set(d, "idv:vmmode",         _vmmode.c_str());

    if (!edit) {
        _get_printer_manager_code(_printerdisk_info, printer_manager);
        LOG_INFO("acpi printerManager %s", printer_manager.c_str());
        iniparser_set(d, "idv:printerManager", printer_manager.c_str());
    }

    iniparser_dump_ini(d, filename.c_str());
    iniparser_freedict(d);
    __del_space_and_emptyline(filename);
    return;
}

void VmManage::_get_printer_manager_code(const DiskInfo_t &disk_info, string& printer_manager)
{
    int printer = PRINTER_MANAGER_CLOSE;

    if (disk_info.is_enable) {
        printer = PRINTER_MANAGER_OPEN;
        // expand space not enough or create error
        if (_app->get_vm()->cal_expand_disk() == 0) {
            printer = PRINTER_MANAGER_SPACE_NOT_ENOUGH;
        } else if (!_app->get_vm()->vm_check_expand_disk_exist(disk_info.disk_name)) {
            if (!_app->get_vm()->vm_check_file_md5(RCC_PRINT_FILE)) {
                printer = PRINTER_MANAGER_UNPACK_ERROR;
            } else {
                printer = PRINTER_MANAGER_SPACE_CREATE_ERROR;
            }
        }
    } else {
        printer = PRINTER_MANAGER_CLOSE;
    }
    LOG_DEBUG("_get_printer_manager_code printer %d", printer);
    printer_manager = to_string(printer);
}

bool VmManage::_has_user_disk()
{
	if (_allow_userdisk == -1) {
		if (_modeInfo->mode == PUBLIC_MODE || (_modeInfo->mode == SPECIAL_MODE && _modeInfo->bind_user.username == _userInfo->username)) {
			return true;
		} else {
			return false;
		}
	} else {
		if (_app->get_vm()->vm_get_user_disk_size() == 0) {
			return false;
		} else if (_userInfo->username == "guest" || (_modeInfo->mode == SPECIAL_MODE && _modeInfo->bind_user.username != _userInfo->username)) {
			return false;
		} else {
			return _allow_userdisk;
		}
	}
}

bool VmManage::_has_create_layer()
{
    if (_app->get_UsrUserInfoMgr()->get_app_layer_switch() != 1) {
        LOG_DEBUG("layer manage switch off, no need to create layer disk!");
        return false;
    }

    if (!vm_is_vm_recovery()) {
        if (_layer_info.layer_on_1  == "Y") {
            if (_app->get_vm()->vm_check_personal_img_exist() && !_app->get_vm()->vm_check_layerdisk_exist()) {
                return false;
            }
        }

        if (_layer_info.layer_on_1  == "Y" || _app->get_vm()->vm_check_layerdisk_exist()) {
            return true;
        }
    }

    return false;
}

#if 0
int VmManage::_get_vm_cpu_num()
{
#define DEFAULT_VCPU_NUM 4
    int vcpu = 0;
    string vcpu_result = execute_command("cat /proc/cpuinfo | grep '^processor' | wc -l");
    LOG_INFO("vcpu num:%s",vcpu_result.c_str());
    vcpu = atoi(vcpu_result.c_str());
    if(vcpu == 0)
    {
        vcpu = DEFAULT_VCPU_NUM;
    }
    return vcpu;
#undef DEFAULT_VCPU_NUM
}
#endif

void VmManage::_get_cpu_codename(string &codename)
{
    string result = execute_command("dmidecode -t system | grep Family | awk -F\": \" \'{print $2}\'");

    if (result == "error") {
        LOG_ERR("read cpu code name err!");
    } else {
        LOG_DEBUG("codename: %s", result.c_str());
        codename = result;
    }
}

int VmManage::_check_vmmodeini_exist()
{
    bool ret = get_file_exist(VMMODE_INI_PATH);

   if(!ret) {
        return START_VMMODEINT_LOST;
    } else {
        return START_VM_CHECK_OK;
    }
}

int VmManage::_get_vmmodeini_param(string &ostype, string &cpu)
{
    // if ostype is exist
    bool ret = false;
    struct BasicInfo basicInfo;

    if (ostype == OSTYPE_UNKNOWN) {
        return START_OSTYPE_UNKNOWN;
    }
    
    ret = _app->get_UsrUserInfoMgr()->checkvmmode_section(ostype);
    if (!ret) {
        return START_NOT_SUPPORT_OSTYPE;
    }

    _app->vm_get_basic_info(&basicInfo);
    cpu = basicInfo.cpu;
    ret =  _app->get_UsrUserInfoMgr()->checkvmmode_entry(ostype, basicInfo.cpu);
    if (!ret) {
        return START_NOT_SUPPORT_CPU;
    }

    return START_VM_CHECK_OK;
}

int VmManage::_get_image_ostype_vmmode(string &vmmode, bool is_driver, string driver_ostype)
{
    int ret = START_VM_CHECK_OK;
    string ostype, cpu;
    string codename = "";

    vmmode = VMMODE_PASS_INI;
    if (!is_driver) {
        // if vm_image_info.ini ostype is empty or it value is null then start with passthough and get ostype from vm_image_info.ini
        if(!_app->get_vm()->vm_get_image_ostype(ostype) || ostype == "") {
            vmmode = VMMODE_PASS_INI;
            return START_VM_CHECK_OK;
        }
    } else {
        ostype = driver_ostype;
    }

    ret = _check_vmmodeini_exist();
    if (ret != START_VM_CHECK_OK) {
        return ret;
    }

    LOG_INFO("vm_get_image_ostype:%s is_driver:%d", ostype.c_str(), is_driver);
    ret = _get_vmmodeini_param(ostype, cpu);
    if (ret != START_VM_CHECK_OK) {
        return ret;
    }

    _get_cpu_codename(codename);
    if (!codename.empty() && _app->get_UsrUserInfoMgr()->checkvmmode_entry(ostype, codename)) {
        _app->get_UsrUserInfoMgr()->getvmmode_value(ostype, codename, vmmode);
    } else {
        _app->get_UsrUserInfoMgr()->getvmmode_value(ostype, cpu, vmmode);
    }

    return ret;
}

int VmManage::_get_vmmode_passlist(std::vector<string> &passlist, bool is_driver, string driver_ostype)
{
    string ostype;
    string pass_list, pass_value;

    passlist.clear();
    if (!is_driver) {
        // if vm_image_info.ini ostype is empty the passlist not need to send
        if(!_app->get_vm()->vm_get_image_ostype(ostype) || ostype == "") {
            return 0;
        }
    } else {
        ostype = driver_ostype;
    }

    if (_app->get_UsrUserInfoMgr()->getvmmode_value(ostype, "passlist", pass_list) != 0) {
        LOG_ERR("_get_vmmode_passlist error");
        return -1;
    }

    // split
    istringstream  pass(pass_list);
    while(getline(pass, pass_value, ' ')) {
        LOG_INFO("pass value: %s", pass_value.c_str());
        passlist.push_back(pass_value);
    }

    return 0;
}

#if 0
int VmManage::_calc_vm_memory_size(const string &vmmode)
{
    int vm_mem_size;
    int reserved_mem_size;
    string total_mem_stream = execute_command("free | grep Mem | awk '{print $2}'");
    int total_mem_size = atoi(total_mem_stream.c_str()) / 1024;
    int cache_mem_size = 0;
    
    if (vmmode == VMMODE_EMULATION_INI) {
        reserved_mem_size = RCC_RESERVED_MEMORY_EMULATION_DEFAULT;
     } else if(vmmode == VMMODE_PASS_INI) {
        reserved_mem_size = RCC_RESERVED_MEMORY_PASS_DEFAULT;
     } else {
        LOG_WARNING("vmmode %s is error", vmmode.c_str());
        return 0;
     }

    if (reserved_mem_size >= total_mem_size) {
        LOG_WARNING("total_mem_size %d MB reserved memory size is :%d MB!", total_mem_size, reserved_mem_size);
        return 0;
    }

    if (vmmode == VMMODE_PASS_INI && total_mem_size > 4096) {
        //bugid 546582: avoid qemu process oom
        string cache_mem_stream = execute_command(
                "sed -n '/^Node 0, zone   Normal/,/^Node 0, zone/p' /proc/zoneinfo | grep min | awk '{print $2}' | head -1");
        int cache_mem = atoi(cache_mem_stream.c_str());
        if (cache_mem > 0) {
            cache_mem_size = 5 * (cache_mem * 4 / 1024 - 29);
            LOG_DEBUG("cache memory size:%d MB", cache_mem_size);
        }
    }
    vm_mem_size = total_mem_size - reserved_mem_size - cache_mem_size;
    vm_mem_size = vm_mem_size - (vm_mem_size % 4);
    if (vm_mem_size < 0)
    {
        LOG_WARNING("total memory size < 600 MB!");
        vm_mem_size = 0;
    }
    LOG_INFO("total memory size:%d MB, vm memory size:%d MB", total_mem_size, vm_mem_size);
    return vm_mem_size;
}
#endif

void VmManage::_vm_check_virtio_flag(string base_file, string img_file, bool* virtio_flag, bool* extend_mem)
{
#define VIRTIO_FLAG       (1 << 2)
#define EXTEND_MEM_FLAG   (1 << 3)
    string filename;
    string cmd;
    int ret;

    if (get_file_exist(img_file.c_str())) {
        filename = img_file;
    } else {
        filename = base_file;
    }
    if (filename.empty()) {
        LOG_ERR("base file not exist!");
        return;
    }
    cmd = CHECK_VIRTIO_FLAG + " " + filename;
    ret = rc_system(cmd.c_str());
    *virtio_flag = ((ret & VIRTIO_FLAG) != 0);
    *extend_mem  = ((ret & EXTEND_MEM_FLAG) != 0);
    LOG_INFO("cmd: %s, checkvflag: ret = %d, virtio_flag = %d, extend_mem = %d", cmd.c_str(), ret, *virtio_flag, *extend_mem);
#undef VIRTIO_FLAG
#undef EXTEND_MEM_FLAG
}


/**
* vm start quickly off or on
* @ extend_mem: the extend memory mode
*
* return: 0: ok, !=0: not get the operation from file
*/
#if 0
int VmManage::_vm_get_vm_start_quickly_mem(bool &extend_mem)
{
    int ret = 0;
    int clear = 0;
    bool last_online = false; // false: is online true: local mode
    int  last_is_guest = 0;
    string ostype;

    if (!_driver_install_parament.empty()) {
        return -1;
    }

    _app->get_vm()->vm_get_image_ostype(ostype);

    if (ostype == "Windows 10") {
        // if clear user disk  then set extend_mem to true to help vm solve problem
        ret = _app->get_UsrUserInfoMgr()->get_user_disk_clear(clear);
        LOG_INFO("ret = %d, clear %d, extend_mem %d", ret, clear, extend_mem);
        if (!ret) {
            extend_mem = (clear? true : false);
        }

        // if Device Driver is diff thena set extend_mem to true to help vm solve Blue screen problem
        if (!extend_mem) {
            ret = _app->get_UsrUserInfoMgr()->get_user_online_status(last_online);
            LOG_INFO("ret = %d, last_online %d, extend_mem %d", ret, last_online, extend_mem);
            if (!ret) {
                extend_mem = ((last_online != _app->is_localmode_startvm())? true : false);
            }
        }

        // if last is guest login then set extend_mem to true to help gt solve print function
        if (!extend_mem) {
            ret = _app->get_UsrUserInfoMgr()->get_guestlogin_status(last_is_guest);
            LOG_INFO("ret = %d, last_is_guest %d, extend_mem %d", ret, last_is_guest, extend_mem);
            if (!ret) {
                extend_mem = (last_is_guest ? true : false);
            }
        }
    } else {
        return -1;
    }

    LOG_INFO("_vm_get_vm_start_quickly_mem ret = %d, clear %d, extend_mem %d ostype %s", ret, clear, extend_mem, ostype.c_str());
    return ret;
}
#endif

