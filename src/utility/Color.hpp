/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/**
 * @file		Color.hpp
 * @since		Jun 22, 2018
 * @author		Patricio A. Rossi (MeduZa)
 * @copyright	Copyright © 2018 Patricio A. Rossi (MeduZa)
 */

#ifndef COLOR_HPP_
#define COLOR_HPP_ 1

// For ints.
#include <cstdint>

#include <iomanip>
#include <string>
using std::string;

// To handle unordered map.
#include <unordered_map>
#ifndef umap
	#define umap std::unordered_map
#endif

#include "Log.hpp"

namespace LEDSpicer {

/**
 * LEDSpicer::Color
 */
class Color {

public:

	/**
	 * Normal: will overwrite the background.
	 * Combine: will combine with the background.
	 * Diff: Will difference with the background.
	 */
	enum Filters : uint8_t {Normal, Combine, Diff};

	/**
	 * Creates a new Color class (black color).
	 */
	Color() {}

	/**
	 * Creates a new Color class from RGB independent values.
	 * @param r
	 * @param g
	 * @param b
	 */
	Color(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) {}

	/**
	 * Creates a new Color class from a color name from the color list.
	 * @param color
	 */
	Color(const string& color);

	/**
	 * Creates a new color class from a hex or dec color format.
	 * @param color
	 * @param format
	 */
	Color(const string& color, const string& format);

	virtual ~Color() {}

	void setR(uint8_t color);
	void setG(uint8_t color);
	void setB(uint8_t color);

	void set(uint8_t r, uint8_t g, uint8_t b);
	void set(uint32_t color);

	/**
	 * Sets the color from an hex(ex FFFFFF) or dec (ex: 255,255,255) string.
	 * TODO: implement dec if needed.
	 * @param color
	 * @param format true for hex
	 */
	void set(const string& color, bool formatHex);
	void set(const string& color);
	void set(const Color& color);

	uint8_t getR();
	uint8_t getG();
	uint8_t getB();
	uint32_t getRGB();

	/**
	 * Calculates the fade for this color.
	 * @param percent
	 * @return
	 */
	Color fade(uint8_t percent);

	/**
	 * Calculates the transition between this color and a destination color.
	 * @param destination
	 * @param percent
	 * @return
	 */
	Color transition(const Color& destination, uint8_t percent);

	/**
	 * Creates a linear transition between two colors tones.
	 * @param colorA
	 * @param colorB
	 * @param percent
	 * @return
	 */
	static uint8_t transition(uint8_t colorA, uint8_t colorB, uint8_t percent);

	/**
	 * Imports a set of colors to be used by the program.
	 * @param colorsData
	 * @param format (not used yet, only hex) hex or dec
	 */
	static void loadColors(umap<string, string> colorsData, const string& format);

	/// Loaded colors.
	static umap<string, Color> colors;

	static void drawColors();

	/**
	 * Converts the filter code into string.
	 * @param filter
	 * @return
	 */
	static string filter2str(Filters filter);
	static Filters str2filter(const string& filter);

protected:

	uint8_t
		r = 0,
		g = 0,
		b = 0;
};

} /* namespace LEDSpicer */

#endif /* COLOR_HPP_ */
