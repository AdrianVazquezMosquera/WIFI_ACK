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

#include "ESAT_Wifi-telemetry/ESAT_WifiConnectionStateTelemetry.h"
#include "ESAT_Wifi-hardware/ESAT_WifiRadio.h"
#include "ESAT_Wifi-telecommands/ESAT_WifiConnectTelecommand.h"
#include "ESAT_Wifi-telecommands/ESAT_WifiDisableTelemetryTelecommand.h"
#include "ESAT_Wifi-telecommands/ESAT_WifiDisconnectTelecommand.h"
#include "ESAT_Wifi-telecommands/ESAT_WifiEnableTelemetryTelecommand.h"
#include "ESAT_Wifi-telecommands/ESAT_WifiReadConfigurationTelecommand.h"
#include "ESAT_Wifi-telecommands/ESAT_WifiSetNetworkPassphraseTelecommand.h"
#include "ESAT_Wifi-telecommands/ESAT_WifiSetNetworkSSIDTelecommand.h"
#include "ESAT_Wifi-telecommands/ESAT_WifiSetServerAddressTelecommand.h"
#include "ESAT_Wifi-telecommands/ESAT_WifiSetServerPortTelecommand.h"
#include "ESAT_Wifi-telecommands/ESAT_WifiSetTimeTelecommand.h"
#include "ESAT_Wifi-telecommands/ESAT_WifiWriteConfigurationTelecommand.h"

boolean ESAT_WifiConnectionStateTelemetryClass::available()
{
  return true;
}

boolean ESAT_WifiConnectionStateTelemetryClass::fillUserData(ESAT_CCSDSPacket& packet)
{
  packet.writeByte(ESAT_WifiRadio.readConnectionState());
  if (packet.triedToWriteBeyondCapacity())
  {
    return false;
  }
  else
  {
    return true;
  }
}

ESAT_WifiConnectionStateTelemetryClass ESAT_WifiConnectionStateTelemetry;
