.PHONY: all clean broker client run install

all: broker client

broker:
	@echo "🔧 Building IoTher Broker..."
	cd src/broker && gcc -pthread -o iother_broker iother_broker.c -Wall -O2
	@echo "✓ Broker built successfully"

client:
	@echo "🔧 Building IoTher Device Simulator..."
	cd src/client && g++ -pthread -std=c++11 -o iother_device main.cpp device_simulator.cpp -Wall -O2
	@echo "✓ Device simulator built successfully"

clean:
	@echo "🧹 Cleaning build artifacts..."
	rm -f src/broker/iother_broker
	rm -f src/client/iother_device
	rm -f iother.log
	@echo "✓ Clean complete"

run-broker:
	@echo "🚀 Starting IoTher Broker..."
	cd src/broker && ./iother_broker

run-client:
	@echo "🚀 Starting IoTher Devices..."
	cd src/client && ./iother_device

run: all
	@echo "🚀 Starting IoTher Simulation..."
	@gnome-terminal --tab --title="IoTher Broker" -- bash -c "cd src/broker && ./iother_broker; exec bash" &
	@sleep 2
	@gnome-terminal --tab --title="IoTher Devices" -- bash -c "cd src/client && ./iother_device; exec bash" &
	@echo "✓ IoTher is running in separate terminals"

install:
	@echo "📦 Installing IoTher to /usr/local/bin..."
	sudo cp src/broker/iother_broker /usr/local/bin/
	sudo cp src/client/iother_device /usr/local/bin/
	@echo "✓ IoTher installed successfully"

help:
	@echo "IoTher - Virtual Sensor Simulation Platform"
	@echo ""
	@echo "Available commands:"
	@echo "  make          - Build both broker and client"
	@echo "  make broker   - Build only the broker"
	@echo "  make client   - Build only the client"
	@echo "  make run      - Run complete simulation"
	@echo "  make clean    - Remove build artifacts"
	@echo "  make install  - Install to system"
	@echo "  make help     - Show this help"
