/*
 * Copyright (C) 2018 Theia Space, Universidad Politécnica de Madrid
 *
 * This file is part of Theia Space's ESAT OBC library.
 *
 * Theia Space's ESAT OBC library is free software: you can
 * redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * Theia Space's ESAT OBC library is distributed in the hope that it
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Theia Space's ESAT OBC library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef ESAT_WifiACKTelemetry_h
#define ESAT_WifiACKTelemetry_h

#include <Arduino.h>
#include <ESAT_CCSDSTelemetryPacketContents.h>
#include <ESAT_CCSDSSecondaryHeader.h>
#include <ESAT_CCSDSTelecommandPacketHandler.h>

// ESAT Wifi ACK packet contents.
class ESAT_WifiACKTelemetryClass: public ESAT_CCSDSTelemetryPacketContents
{
  public:
    // Return true when a new telemetry packet is available; otherwise
    // return false.
    boolean available();
    boolean makeAvaliable(bool isAvaliable);
    bool activated = false;

    // Return the packet identifier.
    byte packetIdentifier()
    {
      return 0x05;
    }

    // Fill the user data field of the given packet.
    // The write pointer of the packet is already at the start
    // of the user data field.
    // Return true on success; otherwise return false.
    boolean fillUserData(ESAT_CCSDSPacket& packet);

    boolean handlerIsCompatibleWithPacket(byte packetIdentifier, ESAT_CCSDSSecondaryHeader secondaryHeader);
      
    ESAT_CCSDSSecondaryHeader saveSecondaryHeader(ESAT_CCSDSPacket &packet);
    ESAT_CCSDSSecondaryHeader datum;
};

// Global instance of ESAT_WifiACKTelemetry.  
extern ESAT_WifiACKTelemetryClass ESAT_WifiACKTelemetry;

#endif /* ESAT_WifiACKTelemetry_h */
