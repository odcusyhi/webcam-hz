// webcam-hz: set the anti-flicker (power line frequency) of a USB webcam on macOS.
// Usage: webcam-hz [50|60|auto|off]   (no argument = show current setting)
#include <stdio.h>
#include <string.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/usb/IOUSBLib.h>

#define VIDEO_CLASS 14
#define VIDEO_CONTROL 1
#define PU_POWER_LINE_FREQ 0x05
static const char *names[] = {"off", "50 Hz", "60 Hz", "auto"};

static int find_pu_id(io_service_t intf) {
    io_service_t dev = 0;
    if (IORegistryEntryGetParentEntry(intf, kIOServicePlane, &dev) != KERN_SUCCESS) return -1;
    IOCFPlugInInterface **plug = NULL; SInt32 score; int id = -1;
    if (IOCreatePlugInInterfaceForService(dev, kIOUSBDeviceUserClientTypeID, kIOCFPlugInInterfaceID, &plug, &score) == KERN_SUCCESS && plug) {
        IOUSBDeviceInterface **d = NULL;
        (*plug)->QueryInterface(plug, CFUUIDGetUUIDBytes(kIOUSBDeviceInterfaceID), (LPVOID *)&d);
        (*plug)->Release(plug);
        IOUSBConfigurationDescriptorPtr cfg = NULL;
        if (d && (*d)->GetConfigurationDescriptorPtr(d, 0, &cfg) == kIOReturnSuccess) {
            UInt8 *p = (UInt8 *)cfg, *end = p + USBToHostWord(cfg->wTotalLength);
            while (p + 3 <= end && p[0] > 0) {
                if (p[1] == 0x24 && p[2] == 0x05) { id = p[3]; break; } // CS_INTERFACE / VC_PROCESSING_UNIT
                p += p[0];
            }
        }
        if (d) (*d)->Release(d);
    }
    IOObjectRelease(dev);
    return id;
}

int main(int argc, char **argv) {
    int want = -1;
    if (argc > 1) {
        if (!strcmp(argv[1], "off")) want = 0;
        else if (!strcmp(argv[1], "50")) want = 1;
        else if (!strcmp(argv[1], "60")) want = 2;
        else if (!strcmp(argv[1], "auto")) want = 3;
        else { fprintf(stderr, "Usage: webcam-hz [50|60|auto|off]\n"); return 2; }
    }
    CFMutableDictionaryRef match = IOServiceMatching("IOUSBHostInterface");
    int cls = VIDEO_CLASS, sub = VIDEO_CONTROL;
    CFNumberRef c = CFNumberCreate(NULL, kCFNumberIntType, &cls), s = CFNumberCreate(NULL, kCFNumberIntType, &sub);
    CFDictionarySetValue(match, CFSTR("bInterfaceClass"), c);
    CFDictionarySetValue(match, CFSTR("bInterfaceSubClass"), s);
    io_iterator_t it;
    if (IOServiceGetMatchingServices(kIOMainPortDefault, match, &it) != KERN_SUCCESS) return 1;

    int found = 0; io_service_t intf;
    while ((intf = IOIteratorNext(it))) {
        char name[128] = "USB camera";
        io_service_t dev; 
        if (IORegistryEntryGetParentEntry(intf, kIOServicePlane, &dev) == KERN_SUCCESS) {
            IORegistryEntryGetName(dev, name); IOObjectRelease(dev);
        }
        int pu = find_pu_id(intf);
        IOCFPlugInInterface **plug = NULL; SInt32 score;
        IOUSBInterfaceInterface **ui = NULL;
        if (pu < 0 || IOCreatePlugInInterfaceForService(intf, kIOUSBInterfaceUserClientTypeID, kIOCFPlugInInterfaceID, &plug, &score) != KERN_SUCCESS) {
            fprintf(stderr, "%s: cannot talk to camera\n", name); IOObjectRelease(intf); continue;
        }
        (*plug)->QueryInterface(plug, CFUUIDGetUUIDBytes(kIOUSBInterfaceInterfaceID), (LPVOID *)&ui);
        (*plug)->Release(plug);
        UInt8 num = 0; (*ui)->GetInterfaceNumber(ui, &num);
        UInt8 val = (UInt8)want;
        IOUSBDevRequest r = { .wValue = PU_POWER_LINE_FREQ << 8, .wIndex = (pu << 8) | num, .wLength = 1, .pData = &val };
        if (want >= 0) {
            r.bmRequestType = 0x21; r.bRequest = 0x01; // SET_CUR
            IOReturn e = (*ui)->ControlRequest(ui, 0, &r);
            if (e) fprintf(stderr, "%s: could not set (error 0x%x)\n", name, e);
        }
        r.bmRequestType = 0xA1; r.bRequest = 0x81; val = 0xFF; // GET_CUR
        if ((*ui)->ControlRequest(ui, 0, &r) == kIOReturnSuccess && val < 4)
            printf("%s: anti-flicker = %s\n", name, names[val]), found++;
        else fprintf(stderr, "%s: no anti-flicker setting\n", name);
        (*ui)->Release(ui); IOObjectRelease(intf);
    }
    IOObjectRelease(it);
    if (!found) { fprintf(stderr, "No USB webcam with anti-flicker found.\n"); return 1; }
    return 0;
}
