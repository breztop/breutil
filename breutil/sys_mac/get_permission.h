#pragma once
#pragma error("This file help you build your own permission check system, " \
              "this is no way with only header file"                        \
              "you need to implement the function in cpp file")

enum class Permission {
    AudioDeviceAccess = 0,  //
    VideoDeviceAccess = 1,
    ScreenCapture = 2,
    Accessibility = 3
};

enum class PermissionStatus {
    PermissionNotDetermined = 0,
    PermissionRestricted = 1,
    PermissionDenied = 2,
    PermissionAuthorized = 3,
};

class GetPermission {
public:
    static PermissionStatus GetPermissionStatus(Permission permission) {
        return checkPermissionWithPrompt(permission, false);
    }

    static bool RequestPermission(Permission permission) {
        PermissionStatus status = checkPermissionWithPrompt(permission, true);
        return status == PermissionStatus::PermissionAuthorized;
    }

    static void OpenMacOSPrivacyPreferences(const char *tab);

private:
    static PermissionStatus checkPermissionWithPrompt(Permission permission,
                                                      bool prompt_for_permission);
};


#include <AVFoundation/AVFoundation.h>
#include <AppKit/AppKit.h>
#include <ApplicationServices/ApplicationServices.h>
#include <CoreFoundation/CoreFoundation.h>


inline void GetPermission::OpenMacOSPrivacyPreferences(const char *tab) {
    NSURL *url =
        [NSURL URLWithString:[NSString stringWithFormat:@"x-apple.systempreferences:com.apple."
                                                        @"preference.security?Privacy_%s",
                                                        tab]];
    [[NSWorkspace sharedWorkspace] openURL:url];
}


PermissionStatus GetPermission::CheckPermissionWithPrompt(MacPermissionType type,
                                                          bool prompt_for_permission) {
    __block MacPermissionStatus permissionResponse = kPermissionNotDetermined;

    switch (type) {
        case kAudioDeviceAccess: {
            AVAuthorizationStatus audioStatus =
                [AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeAudio];

            if (audioStatus == AVAuthorizationStatusNotDetermined && prompt_for_permission) {
                os_event_t *block_finished;
                os_event_init(&block_finished, OS_EVENT_TYPE_MANUAL);
                [AVCaptureDevice requestAccessForMediaType:AVMediaTypeAudio
                                         completionHandler:^(BOOL granted __attribute((unused))) {
                                           os_event_signal(block_finished);
                                         }];
                os_event_wait(block_finished);
                os_event_destroy(block_finished);
                audioStatus = [AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeAudio];
            }

            permissionResponse = (MacPermissionStatus)audioStatus;

            blog(LOG_INFO, "[macOS] Permission for audio device access %s.",
                 permissionResponse == kPermissionAuthorized ? "granted" : "denied");

            break;
        }
        case kVideoDeviceAccess: {
            AVAuthorizationStatus videoStatus =
                [AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeVideo];

            if (videoStatus == AVAuthorizationStatusNotDetermined && prompt_for_permission) {
                os_event_t *block_finished;
                os_event_init(&block_finished, OS_EVENT_TYPE_MANUAL);
                [AVCaptureDevice requestAccessForMediaType:AVMediaTypeVideo
                                         completionHandler:^(BOOL granted __attribute((unused))) {
                                           os_event_signal(block_finished);
                                         }];

                os_event_wait(block_finished);
                os_event_destroy(block_finished);
                videoStatus = [AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeVideo];
            }

            permissionResponse = (MacPermissionStatus)videoStatus;

            blog(LOG_INFO, "[macOS] Permission for video device access %s.",
                 permissionResponse == kPermissionAuthorized ? "granted" : "denied");

            break;
        }
        case kScreenCapture: {
            permissionResponse =
                (CGPreflightScreenCaptureAccess() ? kPermissionAuthorized : kPermissionDenied);

            if (permissionResponse != kPermissionAuthorized && prompt_for_permission) {
                permissionResponse =
                    (CGRequestScreenCaptureAccess() ? kPermissionAuthorized : kPermissionDenied);
            }

            blog(LOG_INFO, "[macOS] Permission for screen capture %s.",
                 permissionResponse == kPermissionAuthorized ? "granted" : "denied");

            break;
        }
        case kAccessibility: {
            permissionResponse = (AXIsProcessTrusted() ? kPermissionAuthorized : kPermissionDenied);

            if (permissionResponse != kPermissionAuthorized && prompt_for_permission) {
                NSDictionary *options = @{(__bridge id)kAXTrustedCheckOptionPrompt : @YES};
                permissionResponse =
                    (AXIsProcessTrustedWithOptions((CFDictionaryRef)options) ? kPermissionAuthorized
                                                                             : kPermissionDenied);
            }

            blog(LOG_INFO, "[macOS] Permission for accessibility %s.",
                 permissionResponse == kPermissionAuthorized ? "granted" : "denied");
            break;
        }
    }

    return permissionResponse;
}
