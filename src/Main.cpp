/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/**
 * @file		Main.cpp
 * @since		Jun 6, 2018
 * @author		Patricio A. Rossi (MeduZa)
 * @copyright	Copyright © 2018 Patricio A. Rossi (MeduZa)
 */

#include "Main.hpp"

#ifdef DEVELOP
#include <execinfo.h>
#endif

using namespace LEDSpicer;

bool Main::running = false;

Main::Main(Modes mode) : messages(DataLoader::portNumber) {
	profiles.push_back(DataLoader::defaultProfile);
	if (mode == Modes::Dump)
		return;

#ifndef DRY_RUN
	if (mode == Modes::Normal) {
		Log::debug("Daemonizing");
		if (daemon(0, 0) == -1) {
			throw Error("Unable to daemonize.");
		}
		Log::debug("Daemonized");
	}
	for (auto device : DataLoader::devices)
		device->initialize();
#endif

	running = true;
}

Main::~Main() {

	delete DataLoader::defaultProfile;
	for (auto d : DataLoader::devices)
		delete d;
	ConnectionUSB::terminate();
}

void Main::run() {

	Log::info("LEDSpicer Running");

	while (running) {

		if (messages.read()) {
			Message msg = messages.getMessage();
			switch (msg.type) {
			case Message::Types::LoadProfile: {
				if (not tryProfiles(msg.data)) {
					Log::debug("All requested profiles failed");
					break;
				}
				break;
			}

			case Message::Types::FinishLastProfile:
				if (profiles.size() == 1)
					break;
				Log::debug("Profile terminate " + to_string(profiles.size()));
				profiles.back()->terminate();
				break;

			default:
				break;
			}
		}
		runCurrentProfile();
	}
	profiles[0]->terminate();

	// Wait for termination.
	while (not profiles[0]->isRunning())
		profiles[0]->runFrame();
}

void Main::testLeds() {
	cout << "Test LEDs (q to quit)" << endl;
	string inp;
	Device* device = selectDevice();
	if (not device)
		return;

	while (true) {
		uint8_t led;
		std::cin.clear();
		cout << endl << "Select a Led (r to reset, q to quit):" << endl;
		std::getline(std::cin, inp);

		if (inp == "r") {
			device->setLeds(0);
			device->transfer();
			continue;
		}

		if (inp == "q")
			return;

		try {
			led = std::stoi(inp);
			if (led > device->getNumberOfLeds() or led < 1)
				throw "";
			device->setLed(led, 255);
			device->transfer();
		}
		catch (...) {
			cerr << "Invalid pin number" << endl;
		}
	}
}

void Main::testElements() {
	cout << "Test Elements (q to quit)" << endl;
	string inp;
	Device* device = selectDevice();
	if (not device)
		return;

	while (true) {
		std::cin.clear();

		for (auto e : *device->getElements())
			cout << e.second.getName() << endl;
		cout << endl << "Enter an element name (r to reset, q to quit):" << endl;

		std::getline(std::cin, inp);

		if (inp == "r") {
			device->setLeds(0);
			device->transfer();
			continue;
		}

		if (inp == "q")
			return;

		try {
			Element* element = device->getElement(inp);
			element->setColor(Color::getColor("White"));
			device->transfer();
		}
		catch (...) {
			cerr << "Invalid pin number" << endl;
		}
	}
}

void Main::terminate() {
	running = false;
}

void signalHandler(int sig) {

	if (sig == SIGTERM) {
		Log::info(PACKAGE_NAME " terminated by signal");
		Main::terminate();
		return;
	}

	// display back trace
#ifdef DEVELOP
	void* array[10];
	size_t size = backtrace(array, 10);
	backtrace_symbols_fd(array, size, STDERR_FILENO);
#endif
	exit(EXIT_FAILURE);
}

void Main::dumpConfiguration() {
	cout << endl << "System Configuration:" << endl << "Colors:" << endl;
	Color::drawColors();
	cout  << endl << "Hardware:" << endl;
	for (auto d : DataLoader::devices)
		d->drawHardwarePinMap();
	cout << "Interval: " << (int)ConnectionUSB::getInterval() << "ms" << endl;
	cout << "Total Elements registered: " << (int)DataLoader::allElements.size() << endl;
	cout << endl << "Layout:";
	for (auto group : DataLoader::layout) {
		cout << endl << "Group: '" << group.first << "' with ";
		group.second.drawElements();
	}
	cout << endl << "Default Profile:" << endl;
	DataLoader::defaultProfile->drawConfig();
}

int main(int argc, char **argv) {

#ifdef DEVELOP
	signal(SIGSEGV, signalHandler);
#endif
	signal(SIGTERM, signalHandler);

	// Process command line options.
	string commandline, configFile = "";

	// Run in the background.
	Main::Modes mode = Main::Modes::Normal;

	for (int i = 1; i < argc; i++) {

		commandline = argv[i];

		// Help text.
		if (commandline == "-h" or commandline == "--help") {
			cout <<
				PACKAGE_NAME " command line usage:\n"
				PACKAGE " <options>\n"
				"options:\n"
				"-c <conf> or --config <conf>\tUse an alternative configuration file\n"
				"-f or --foreground\t\tRun on foreground\n"
				"-d or --dump\t\t\tDump configuration files and quit\n"
				"-l or --leds\t\t\tTest LEDs.\n"
				"-e or --elements\t\tTest registered elements.\n"
				"-v or --version\t\t\tDisplay version information\n"
				"-h or --help\t\t\tDisplay this help screen.\n"
				"Data directory: " PACKAGE_DATA_DIR "/\n"
				"If -c or --config is not provided " PACKAGE_NAME " will use " CONFIG_FILE
				<< endl;
			return EXIT_SUCCESS;
		}

		// Version Text.
		if (commandline == "-v" or commandline == "--version") {
			cout
				<< endl <<
				PACKAGE_STRING " Copyright © 2018 - Patricio A. Rossi (MeduZa)\n\n"
				"For more information visit <" PACKAGE_URL ">\n\n"
				"To report errors or bugs visit <" PACKAGE_BUGREPORT ">\n"
				PACKAGE_NAME " is free software under the GPL 3 license\n\n"
				"See the GNU General Public License for more details <http://www.gnu.org/licenses/>."
				<< endl;
			return EXIT_SUCCESS;
		}

		// Alternative configuration.
		if (commandline == "-c" or commandline == "--config") {
			configFile = argv[++i];
			continue;
		}

		// Dump configuration.
		if (commandline == "-d" or commandline == "--dump") {
			mode = Main::Modes::Dump;
			continue;
		}

		// Test LEDs.
		if (commandline == "-l" or commandline == "--leds") {
			mode = Main::Modes::TestLed;
			continue;
		}

		// Test Elements
		if (commandline == "-e" or commandline == "--elements") {
			mode = Main::Modes::TestElement;
			continue;
		}

		// Force foreground.
		if (mode != Main::Modes::Dump and (commandline == "-f" or commandline == "--foreground")) {
			mode = Main::Modes::Foreground;
			continue;
		}
	}

#ifdef DEVELOP
	Log::initialize(true);
#else
	Log::initialize(mode != Main::Modes::Normal);
#endif

	if (configFile.empty())
		configFile = CONFIG_FILE;

	Log::debug("Reading configuration");

	// Read Configuration.
	try {

		DataLoader config(configFile, "Configuration");
		config.readConfiguration();
	}
	catch(Error& e) {
		Log::error("Error: " + e.getMessage());
		return EXIT_FAILURE;
	}

	// Run mode.
	try {

		Main ledspicer(mode);

		switch (mode) {
		case Main::Modes::Dump:
			ledspicer.dumpConfiguration();
			break;

		case Main::Modes::TestLed:
			ledspicer.testLeds();
			break;

		case Main::Modes::TestElement:
			ledspicer.testElements();
			break;

		default:
			ledspicer.run();
		}
	}
	catch(Error& e) {
		Log::error("Error: " + e.getMessage());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

//################
//# Helper stuff #
//################

void Main::runCurrentProfile() {

	// Reset elements.
	for (auto& eD : DataLoader::allElements)
		eD.second->setColor(profiles.back()->getBackgroundColor());

	profiles.back()->runFrame();

	if (not profiles.back()->isRunning())
		profiles.pop_back();

	// Send data.
	for (auto device : DataLoader::devices)
		device->transfer();

	Actor::advanceFrame();
}

Device* Main::selectDevice() {
	string inp;
	while (true) {
		if (DataLoader::devices.size() == 1) {
			return DataLoader::devices[0];
			break;
		}
		std::cin.clear();
		uint8_t deviceIndex;
		cout << "Select a device:" << endl;
		for (uint8_t c = 0; c < DataLoader::devices.size(); ++c)
			// TODO: add device id.
			cout << c+1 << ": " << DataLoader::devices[c]->getName() << endl;
		std::getline(std::cin, inp);
		if (inp == "q")
			return nullptr;
		try {
			deviceIndex = std::stoi(inp);
			if (deviceIndex > DataLoader::devices.size() or deviceIndex < 1)
				throw "";
			return DataLoader::devices[deviceIndex - 1];
			break;
		}
		catch (...) {
			cerr << "Invalid device number" << endl;
		}
	}
}

Profile* Main::tryProfiles(const vector<string>& data) {
	Profile* profile = nullptr;
	for (auto& p : data) {
		Log::debug("changing profile to " + p);
		try {
			profile = DataLoader::processProfile(p);
			profiles.push_back(profile);
			profile->reset();
			break;
		}
		catch(Error& e) {
			Log::debug(e.getMessage());
			continue;
		}
	}
	return profile;
}
