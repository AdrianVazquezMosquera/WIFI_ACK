/*
 * Copyright (C) 2017, 2018, 2019 Theia Space, Universidad Politécnica de Madrid
 *
 * This file is part of Theia Space's ESAT Wifi library.
 *
 * Theia Space's ESAT Wifi library is free software: you can
 * redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * Theia Space's ESAT Wifi library is distributed in the hope that it
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Theia Space's ESAT Util library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "ESAT_Wifi.h"
#include "ESAT_Wifi-hardware/ESAT_WifiConfiguration.h"
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
#include "ESAT_Wifi-telemetry/ESAT_WifiConnectionStateTelemetry.h"
#include "ESAT_Wifi-telemetry/ESAT_WifiACKTelemetry.h"
#include <ESAT_Buffer.h>
#include <ESAT_CCSDSPacketToKISSFrameWriter.h>

void ESAT_WifiClass::addTelecommand(ESAT_CCSDSTelecommandPacketHandler& telecommand)
{
  telecommandPacketDispatcher.add(telecommand);
}

void ESAT_WifiClass::addTelemetry(ESAT_CCSDSTelemetryPacketContents& telemetry)
{
  telemetryPacketBuilder.add(telemetry);
  enableTelemetry(telemetry.packetIdentifier());
}

void ESAT_WifiClass::begin(byte radioBuffer[],
                           const unsigned long radioBufferLength,
                           byte serialBuffer[],
                           const unsigned long serialBufferLength,
                           const byte networkConnectionTimeoutSeconds)
{
  beginTelemetry();
  beginTelecommands();
  beginHardware(radioBuffer,
                radioBufferLength,
                serialBuffer,
                serialBufferLength,
                networkConnectionTimeoutSeconds);
}

void ESAT_WifiClass::beginHardware(byte radioBuffer[],
                                   const unsigned long radioBufferLength,
                                   byte serialBuffer[],
                                   const unsigned long serialBufferLength,
                                   const byte networkConnectionTimeoutSeconds)
{
  ESAT_WifiConfiguration.begin();
  ESAT_WifiConfiguration.readConfiguration();
  ESAT_WifiRadio.begin(radioBuffer,
                       radioBufferLength,
                       networkConnectionTimeoutSeconds);
  // We pass packets around the serial interface in KISS frames.
  serialReader = ESAT_CCSDSPacketFromKISSFrameReader(Serial,
                                                     serialBuffer,
                                                     serialBufferLength);
  // We use two pins to control the flow of packets through the serial
  // interface:
  // - one to signal the on-board computer that we aren't connected
  //   and we cannot accept many packets;
  // - one to accept telemetry requests from the on-board computer.
  pinMode(NOT_CONNECTED_SIGNAL_PIN, OUTPUT);
  digitalWrite(NOT_CONNECTED_SIGNAL_PIN, HIGH);
  pinMode(RESET_TELEMETRY_QUEUE_PIN, INPUT_PULLUP);
  attachInterrupt(RESET_TELEMETRY_QUEUE_PIN,
                  signalTelemetryQueueReset,
                  FALLING);
}

void ESAT_WifiClass::beginTelecommands()
{
  addTelecommand(ESAT_WifiConnectTelecommand);
  addTelecommand(ESAT_WifiDisconnectTelecommand);
  addTelecommand(ESAT_WifiSetNetworkSSIDTelecommand);
  addTelecommand(ESAT_WifiSetNetworkPassphraseTelecommand);
  addTelecommand(ESAT_WifiSetServerAddressTelecommand);
  addTelecommand(ESAT_WifiSetServerPortTelecommand);
  addTelecommand(ESAT_WifiReadConfigurationTelecommand);
  addTelecommand(ESAT_WifiWriteConfigurationTelecommand);
  addTelecommand(ESAT_WifiSetTimeTelecommand);
  addTelecommand(ESAT_WifiEnableTelemetryTelecommand);
  addTelecommand(ESAT_WifiDisableTelemetryTelecommand);
}

void ESAT_WifiClass::beginTelemetry()
{
  addTelemetry(ESAT_WifiConnectionStateTelemetry);
  enableTelemetry(ESAT_WifiConnectionStateTelemetry.packetIdentifier());
  addTelemetry(ESAT_WifiACKTelemetry);
  enableTelemetry(ESAT_WifiACKTelemetry.packetIdentifier());
}

void ESAT_WifiClass::disableTelemetry(const byte identifier)
{
  enabledTelemetry.clear(identifier);
}

void ESAT_WifiClass::enableTelemetry(const byte identifier)
{
  enabledTelemetry.set(identifier);
}

void ESAT_WifiClass::handleTelecommand(ESAT_CCSDSPacket& packet)
{
  // We hide the complexity of handling telecommands with a
  // telecommand packet dispatcher.
  (void) telecommandPacketDispatcher.dispatch(packet);

  if (telecommandPacketDispatcher.dispatch(packet))
  {
    ESAT_WifiACKTelemetry.saveSecondaryHeader(packet);
    bool ACKAvaliable = true;
    ESAT_WifiACKTelemetry.makeAvaliable(ACKAvaliable);
  }
}

boolean ESAT_WifiClass::readPacketFromRadio(ESAT_CCSDSPacket& packet)
{
  return ESAT_WifiRadio.read(packet);
}

boolean ESAT_WifiClass::readPacketFromSerial(ESAT_CCSDSPacket& packet)
{
  return serialReader.read(packet);
}

boolean ESAT_WifiClass::readTelemetry(ESAT_CCSDSPacket& packet)
{
  // We hide the complexity of building telemetry packets with a
  // telemetry packet builder.
  // We build telemetry packets as long as they are pending.
  if (pendingTelemetry.available())
  {
    const byte identifier = byte(pendingTelemetry.readNext());
    pendingTelemetry.clear(identifier);
    return telemetryPacketBuilder.build(packet, identifier);
  }
  else
  {
    return false;
  }
}

void ESAT_WifiClass::resetTelemetryQueue()
{
  ESAT_Wifi.pendingTelemetry =
    (ESAT_Wifi.pendingTelemetry | ESAT_Wifi.telemetryPacketBuilder.available())
    & ESAT_Wifi.enabledTelemetry;
}

void ESAT_WifiClass::signalTelemetryQueueReset()
{
  // This interrupt handler just signals that the telemetry queue must
  // be reset instead of actually resetting the telemetry queue.  This
  // way, the interrupt handler is as fast as it gets and, in addition,
  // it doesn't call any other function that may lie out of RAM.
  // ESAT_Wifi.update() will reset the telemetry queue when necessary.
  ESAT_Wifi.mustResetTelemetryQueue = true;
}

void ESAT_WifiClass::setTime(const ESAT_Timestamp timestamp)
{
  clock.write(timestamp);
}

void ESAT_WifiClass::update()
{
  // The telemetry queue must be reset when needed.
  // ESAT_WifiRadio handles the state of the radio/wifi interface.
  // After that, we must notify the on-board computer about our
  // connection state.
  if (mustResetTelemetryQueue)
  {
    resetTelemetryQueue();
    mustResetTelemetryQueue = false;
  }
  ESAT_WifiRadio.update();
  if (ESAT_WifiRadio.readConnectionState() == ESAT_WifiRadio.CONNECTED)
  {
    digitalWrite(NOT_CONNECTED_SIGNAL_PIN, LOW);
  }
  else
  {
    digitalWrite(NOT_CONNECTED_SIGNAL_PIN, HIGH);
  }
}


void ESAT_WifiClass::writePacketToRadio(ESAT_CCSDSPacket& packet)
{
  ESAT_WifiRadio.write(packet);
}

void ESAT_WifiClass::writePacketToSerial(ESAT_CCSDSPacket& packet)
{
  // Packets are passed around in KISS frames.
  ESAT_CCSDSPacketToKISSFrameWriter serialWriter(Serial);
  (void) serialWriter.unbufferedWrite(packet);
}

ESAT_WifiClass ESAT_Wifi;
