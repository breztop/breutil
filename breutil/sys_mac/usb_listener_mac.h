#ifndef USB_EVENT_H
#define USB_EVENT_H

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/usb/IOUSBLib.h>

#include <chrono>
#include <iostream>
#include <set>
#include <thread>

#include "../usb_listener.hpp"

namespace bre {

class UsbListenerMac : public bre::UsbListener {
public:
    using Shared = std::shared_ptr<UsbListenerMac>;

    static Shared Create();

    UsbListenerMac();

    void Run() override;

    ~UsbListenerMac() {
        if (_notificationPort) {
            IONotificationPortDestroy(_notificationPort);
        }
    }

private:
    static void deviceAdded(void *refCon, io_iterator_t iterator) {
        auto self = static_cast<UsbListenerMac *>(refCon);
        io_service_t usbDevice;
        while ((usbDevice = IOIteratorNext(iterator))) {
            CFStringRef deviceName = (CFStringRef)IORegistryEntryCreateCFProperty(
                usbDevice, CFSTR(kUSBProductString), kCFAllocatorDefault, 0);
            if (deviceName) {
                char deviceNameCString[256];
                CFStringGetCString(deviceName, deviceNameCString, sizeof(deviceNameCString),
                                   kCFStringEncodingUTF8);

                std::string deviceNameStr(deviceNameCString);
                self->_devices.insert(deviceNameStr);

                // 触发回调函数
                if (self->_callback) {
                    try {
                        self->_callback(deviceNameStr, TypeUsbEvent::USB_DEVICE_CONNECTED);
                    } catch (const std::exception &e) {
                        std::cerr << "Error in USB device added callback: " << e.what()
                                  << std::endl;
                    }
                }

                CFRelease(deviceName);
            }
            IOObjectRelease(usbDevice);
        }
    }

    static void deviceRemoved(void *refCon, io_iterator_t iterator) {
        auto self = static_cast<UsbListenerMac *>(refCon);
        io_service_t usbDevice;
        while ((usbDevice = IOIteratorNext(iterator))) {
            CFStringRef deviceName = (CFStringRef)IORegistryEntryCreateCFProperty(
                usbDevice, CFSTR(kUSBProductString), kCFAllocatorDefault, 0);
            if (deviceName) {
                char deviceNameCString[256];
                CFStringGetCString(deviceName, deviceNameCString, sizeof(deviceNameCString),
                                   kCFStringEncodingUTF8);

                std::string deviceNameStr(deviceNameCString);
                self->_devices.erase(deviceNameStr);

                // 触发回调函数
                if (self->_callback) {
                    try {
                        self->_callback(deviceNameStr, TypeUsbEvent::USB_DEVICE_DISCONNECTED);
                    } catch (const std::exception &e) {
                        std::cerr << "Error in USB device removed callback: " << e.what()
                                  << std::endl;
                    }
                }

                CFRelease(deviceName);
            }
            IOObjectRelease(usbDevice);
        }
    }

    void registerForUsbNotifications() {
        _notificationPort = IONotificationPortCreate(kIOMainPortDefault);
        if (!_notificationPort) {
            std::cerr << "Failed to create notification port" << std::endl;
            return;
        }

        CFRunLoopSourceRef runLoopSource = IONotificationPortGetRunLoopSource(_notificationPort);
        CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopDefaultMode);

        CFMutableDictionaryRef matchingDict = IOServiceMatching(kIOUSBDeviceClassName);
        if (!matchingDict) {
            std::cerr << "Failed to create matching dictionary" << std::endl;
            return;
        }

        kern_return_t kr =
            IOServiceAddMatchingNotification(_notificationPort, kIOMatchedNotification,
                                             matchingDict, deviceAdded, this, &_addedIter);
        if (kr != KERN_SUCCESS) {
            std::cerr << "Failed to add matching notification for device added" << std::endl;
            return;
        }

        deviceAdded(this, _addedIter);

        CFMutableDictionaryRef matchingDictRemoved = IOServiceMatching(kIOUSBDeviceClassName);
        if (!matchingDictRemoved) {
            std::cerr << "Failed to create matching dictionary for device removed" << std::endl;
            return;
        }

        kr = IOServiceAddMatchingNotification(_notificationPort, kIOTerminatedNotification,
                                              matchingDictRemoved, deviceRemoved, this,
                                              &_removedIter);
        if (kr != KERN_SUCCESS) {
            std::cerr << "Failed to add matching notification for device removed" << std::endl;
            return;
        }

        deviceRemoved(this, _removedIter);
    }

private:
    IONotificationPortRef _notificationPort = nullptr;
    io_iterator_t _addedIter = 0;
    io_iterator_t _removedIter = 0;

    std::set<std::string> _devices;
};

inline UsbListenerMac::Shared UsbListenerMac::Create() {
    return std::make_shared<UsbListenerMac>();
}

inline UsbListenerMac::UsbListenerMac() { registerForUsbNotifications(); }

inline void UsbListenerMac::Run() { CFRunLoopRun(); }



}  // namespace bre


#endif  // USB_EVENT_H
