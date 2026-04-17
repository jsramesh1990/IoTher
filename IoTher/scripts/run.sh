#!/bin/bash

# IoTher - Virtual Sensor Simulation Platform
# Where devices breathe and data flows

echo "╔═══════════════════════════════════════════════════════════╗"
echo "║                                                           ║"
echo "║     IoTher - Virtual Sensor Simulation Platform          ║"
echo "║     Where devices breathe and data flows                 ║"
echo "║                                                           ║"
echo "╚═══════════════════════════════════════════════════════════╝"
echo ""

# Check if already built
if [ ! -f "src/broker/iother_broker" ] || [ ! -f "src/client/iother_device" ]; then
    echo "🔧 First time setup - Building IoTher..."
    make clean
    make
    if [ $? -ne 0 ]; then
        echo "❌ Build failed. Please check dependencies."
        exit 1
    fi
    echo ""
fi

# Kill any existing instances
pkill -f iother_broker 2>/dev/null
pkill -f iother_device 2>/dev/null

echo "🌬️  Starting IoTher Broker..."
cd src/broker
./iother_broker &
BROKER_PID=$!

sleep 2

if ! kill -0 $BROKER_PID 2>/dev/null; then
    echo "❌ Broker failed to start"
    exit 1
fi

echo "✓ Broker is breathing on port 1883"
echo ""

echo "📡 Starting IoTher Devices..."
cd ../client
./iother_device &
DEVICE_PID=$!

echo "✓ Devices are now flowing data"
echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  IoTher Network is LIVE!"
echo "  Press Ctrl+C in each terminal to stop"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Wait for processes
wait $BROKER_PID $DEVICE_PID
