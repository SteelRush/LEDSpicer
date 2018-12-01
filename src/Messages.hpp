/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/**
 * @file      Messages.hpp
 * @since     Jul 8, 2018
 * @author    Patricio A. Rossi (MeduZa)
 * @copyright Copyright © 2018 Patricio A. Rossi (MeduZa)
 */

#include <queue>
using std::queue;

#include "utility/Socks.hpp"
#include "utility/Utility.hpp"
#include "Message.hpp"

#ifndef MESSAGES_HPP_
#define MESSAGES_HPP_ 1

#define LOCALHOST "127.0.0.1"

namespace LEDSpicer {

/**
 * LEDSpicer::Messages
 */
class Messages : protected Socks {

public:

	struct Player {

		enum class Controllers : uint8_t {
			None,
			Joy8,
			Joy4,
			Joy2,
			Doublejoy,
			Trackball,
			LightGun,
			Dial,
			Mouse,
			Stick,
			Keypad,
			Pedal,
			Other
		};

		Controllers controller = Controllers::None;
		uint8_t buttons = 0;
	};

	struct Emulator {

		enum class Rotations : uint8_t {Degree90, Degree180, Degree270, Degree360};

		enum class Genres : uint8_t {
			Action,
			Adventure,
			Sport,
			Driving,
			Educational,
			Fighting,
			Fitness,
			Simulator,
			Gambling,
			Music,
			Pinball,
			Platformer,
			Puzzle,
			Role,
			Shooter,
			Strategy,
			Trivia,
			Other
		};

		vector<Player> Players;
		uint8_t coins = 0;
		Rotations rotate = Rotations::Degree90;
		Genres genre = Genres::Other;
	};

	Messages(const string& port) : Socks(LOCALHOST, port, true) {}

	virtual ~Messages() {}

	Message getMessage();

	bool read();

protected:

	queue<Message> messages;

};

} /* namespace LEDSpicer */

#endif /* MESSAGES_HPP_ */
