#!/usr/bin/env python3
"""
Bumble Bluetooth Bridge for LibrePods on Windows

This script provides a bridge between the Qt C++ application and the Bumble
Bluetooth stack for Windows. It communicates via stdin/stdout with simple
text-based protocol.

Protocol:
- Commands (from C++ to Python):
  CONNECT:XX:XX:XX:XX:XX:XX
  DISCONNECT
  SEND:hexdata
  QUIT

- Responses (from Python to C++):
  CONNECTED
  DISCONNECTED
  DATA:hexdata
  ERROR:message
  LOG:message
"""

import asyncio
import sys
import logging
from typing import Optional

# Configure logging to stderr so it doesn't interfere with stdout protocol
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
    stream=sys.stderr
)
logger = logging.getLogger("bumble_bridge")

try:
    from bumble.device import Device
    from bumble.host import Host
    from bumble.transport import open_transport
    from bumble.l2cap import ClassicChannelSpec
    from bumble.core import BT_BR_EDR_TRANSPORT
except ImportError as e:
    print("ERROR:Bumble not installed. Run: pip install bumble", flush=True)
    sys.exit(1)


class BumbleBridge:
    """Bridge between Qt application and Bumble Bluetooth stack"""
    
    PSM_AAP = 0x1001  # Apple Accessory Protocol PSM
    HANDSHAKE = bytes.fromhex("00 00 04 00 01 00 02 00 00 00 00 00 00 00 00 00")
    REQUEST_NOTIFICATIONS = bytes.fromhex("04 00 04 00 0F 00 FF FF FF FF")
    
    def __init__(self):
        self.device: Optional[Device] = None
        self.transport = None
        self.channel = None
        self.target_address: Optional[str] = None
        self.connected = False
        self.running = True
        
    async def initialize_transport(self):
        """Initialize Bumble transport for Windows"""
        try:
            # On Windows, Bumble typically uses USB transport
            # Format: usb:0 for first USB Bluetooth adapter
            transport_spec = "usb:0"
            
            logger.info(f"Opening transport: {transport_spec}")
            self.transport = await open_transport(transport_spec)
            
            # Create host and device
            host = Host(controller_source=self.transport.source,
                       controller_sink=self.transport.sink)
            
            self.device = Device.with_hci(
                "LibrePods",
                "F0:F1:F2:F3:F4:F5",  # Random address, will be overwritten
                host
            )
            
            await self.device.power_on()
            logger.info("Bumble device initialized and powered on")
            return True
            
        except Exception as e:
            logger.error(f"Failed to initialize transport: {e}")
            print(f"ERROR:Failed to initialize Bluetooth: {e}", flush=True)
            return False
    
    async def connect(self, address: str):
        """Connect to AirPods at specified address"""
        try:
            if not self.device:
                if not await self.initialize_transport():
                    return
            
            self.target_address = address
            logger.info(f"Connecting to {address}")
            
            # Connect to device
            connection = await self.device.connect(
                address,
                transport=BT_BR_EDR_TRANSPORT
            )
            
            logger.info("ACL connection established, opening L2CAP channel")
            
            # Open L2CAP channel with AAP PSM
            self.channel = await connection.create_l2cap_channel(
                spec=ClassicChannelSpec(psm=self.PSM_AAP)
            )
            
            logger.info("L2CAP channel opened")
            
            # Set up data receive handler
            def on_data_received(data):
                hex_data = data.hex()
                print(f"DATA:{hex_data}", flush=True)
            
            self.channel.sink = on_data_received
            
            # Send handshake
            await self.channel.write(self.HANDSHAKE)
            logger.info("Handshake sent")
            
            # Small delay for handshake to process
            await asyncio.sleep(0.1)
            
            # Request notifications
            await self.channel.write(self.REQUEST_NOTIFICATIONS)
            logger.info("Notifications requested")
            
            self.connected = True
            print("CONNECTED", flush=True)
            
        except Exception as e:
            logger.error(f"Connection failed: {e}")
            print(f"ERROR:Connection failed: {e}", flush=True)
            self.connected = False
    
    async def disconnect(self):
        """Disconnect from AirPods"""
        try:
            if self.channel:
                await self.channel.close()
                self.channel = None
            
            self.connected = False
            print("DISCONNECTED", flush=True)
            logger.info("Disconnected")
            
        except Exception as e:
            logger.error(f"Disconnect failed: {e}")
            print(f"ERROR:Disconnect failed: {e}", flush=True)
    
    async def send_data(self, hex_data: str):
        """Send data to connected AirPods"""
        try:
            if not self.connected or not self.channel:
                print("ERROR:Not connected", flush=True)
                return
            
            data = bytes.fromhex(hex_data)
            await self.channel.write(data)
            logger.debug(f"Sent {len(data)} bytes")
            
        except Exception as e:
            logger.error(f"Send failed: {e}")
            print(f"ERROR:Send failed: {e}", flush=True)
    
    async def handle_command(self, command: str):
        """Handle a command from Qt application"""
        parts = command.strip().split(':', 1)
        if not parts:
            return
        
        cmd = parts[0]
        arg = parts[1] if len(parts) > 1 else None
        
        if cmd == "CONNECT":
            if arg:
                await self.connect(arg)
            else:
                print("ERROR:CONNECT requires address", flush=True)
                
        elif cmd == "DISCONNECT":
            await self.disconnect()
            
        elif cmd == "SEND":
            if arg:
                await self.send_data(arg)
            else:
                print("ERROR:SEND requires data", flush=True)
                
        elif cmd == "QUIT":
            self.running = False
            await self.disconnect()
            
        else:
            print(f"ERROR:Unknown command: {cmd}", flush=True)
    
    async def run(self):
        """Main event loop"""
        print("LOG:Bumble bridge started", flush=True)
        
        # Read commands from stdin in non-blocking way
        loop = asyncio.get_event_loop()
        
        def read_stdin():
            try:
                line = sys.stdin.readline()
                if line:
                    return line.strip()
            except:
                pass
            return None
        
        while self.running:
            # Check for commands
            command = await loop.run_in_executor(None, read_stdin)
            if command:
                await self.handle_command(command)
            else:
                # No command, just wait a bit
                await asyncio.sleep(0.01)
        
        # Cleanup
        if self.device:
            await self.device.power_off()
        
        if self.transport:
            await self.transport.close()
        
        logger.info("Bridge shutdown complete")


def main():
    """Entry point"""
    bridge = BumbleBridge()
    
    try:
        asyncio.run(bridge.run())
    except KeyboardInterrupt:
        logger.info("Interrupted by user")
    except Exception as e:
        logger.error(f"Fatal error: {e}")
        print(f"ERROR:Fatal error: {e}", flush=True)
        sys.exit(1)


if __name__ == "__main__":
    main()
