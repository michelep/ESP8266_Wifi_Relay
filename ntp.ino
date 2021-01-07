// Written by Michele <o-zone@zerozone.it> Pinassi
// Released under GPLv3 - No any warranty

#include <NtpClientLib.h>

// ************************************
// processSyncEvent()
//
// manage NTP sync events and warn in case of error
// ************************************
void processSyncEvent(NTPSyncEvent_t ntpEvent) {
  DEBUG("[DEBUG] processSyncEvent() ");
  if (ntpEvent) {
    DEBUG("[NTP] Time Sync error: ");
    if (ntpEvent == noResponse)
      DEBUG("[NTP] NTP server not reachable");
    else if (ntpEvent == invalidAddress)
      DEBUG("[NTP] Invalid NTP server address");
  } else {
    DEBUG("[NTP] Got NTP time: "+String(NTP.getTimeDateString(NTP.getLastNTPSync())));
  }
}
