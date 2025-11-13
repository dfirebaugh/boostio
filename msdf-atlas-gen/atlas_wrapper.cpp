#include "atlas_wrapper.h"
#include <msdfgen.h>
#include <msdfgen-ext.h>
#include "msdf-atlas-gen/Charset.h"
#include "msdf-atlas-gen/FontGeometry.h"
#include "msdf-atlas-gen/GlyphGeometry.h"
#include "msdf-atlas-gen/TightAtlasPacker.h"
#include "msdf-atlas-gen/ImmediateAtlasGenerator.h"
#include "msdf-atlas-gen/BitmapAtlasStorage.h"
#include "msdf-atlas-gen/glyph-generators.h"
#include "msdf-atlas-gen/image-save.h"
#include "msdf-atlas-gen/json-export.h"
#include <string>
#include <vector>
#include <cstring>

using namespace msdf_atlas;

namespace {

const int CHARSET_VERSION = 2;

void addBasicAsciiCharacters(Charset* charset) {
	for (int i = 32; i < 127; ++i) {
		charset->add(i);
	}
}

void addMathSymbols(Charset* charset) {
	charset->add(0xB1);
	charset->add(0xD7);
	charset->add(0xF7);
}

void addCommonPunctuation(Charset* charset) {
	charset->add(0x2022);
	charset->add(0x2026);
	charset->add(0x2013);
	charset->add(0x2014);
	charset->add(0x2018);
	charset->add(0x2019);
	charset->add(0x201C);
	charset->add(0x201D);
	charset->add(0xB0);
}

void addArrowSymbols(Charset* charset) {
	charset->add(0x2190);
	charset->add(0x2191);
	charset->add(0x2192);
	charset->add(0x2193);
}

void addMusicSymbols(Charset* charset) {
	charset->add(0x266A);
	charset->add(0x266B);
}

Charset* createDefaultCharset() {
	Charset* charset = new Charset();
	addBasicAsciiCharacters(charset);
	addMathSymbols(charset);
	addCommonPunctuation(charset);
	addArrowSymbols(charset);
	addMusicSymbols(charset);
	return charset;
}

Charset* getSharedCharset() {
	static Charset* charset = createDefaultCharset();
	return charset;
}

void applyEdgeColoringToGlyphs(std::vector<GlyphGeometry>& glyphs) {
	for (GlyphGeometry& glyph : glyphs) {
		glyph.edgeColoring(msdfgen::edgeColoringInkTrap, 3.0, 0);
	}
}

msdfgen::Bitmap<unsigned char, 4> convertFloatBitmapToByte(
		const msdfgen::BitmapConstRef<float, 4>& bitmap,
		int width,
		int height) {
	msdfgen::Bitmap<unsigned char, 4> byteBitmap(width, height);
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			const float *src = bitmap(x, y);
			unsigned char *dst = byteBitmap(x, y);
			for (int i = 0; i < 4; ++i) {
				dst[i] = msdfgen::pixelFloatToByte(src[i]);
			}
		}
	}
	return byteBitmap;
}

}

struct AtlasGeneratorImpl {
	std::string error_message;
	msdfgen::FreetypeHandle* ft_handle;

	AtlasGeneratorImpl() : ft_handle(nullptr) {
		ft_handle = msdfgen::initializeFreetype();
		if (!ft_handle) {
			error_message = "Failed to initialize FreeType";
		}
	}

	~AtlasGeneratorImpl() {
		if (ft_handle) {
			msdfgen::deinitializeFreetype(ft_handle);
		}
	}
};

extern "C" {

	struct msdf_atlas_gen* atlas_generator_create(void) {
		return reinterpret_cast<struct msdf_atlas_gen*>(new AtlasGeneratorImpl());
	}

	void atlas_generator_destroy(struct msdf_atlas_gen* gen) {
		delete reinterpret_cast<AtlasGeneratorImpl*>(gen);
	}

	int atlas_generator_generate_mtsdf(
			struct msdf_atlas_gen* gen,
			const char* font_path,
			const char* png_path,
			const char* json_path,
			double font_size,
			double pixel_range
			) {
		AtlasGeneratorImpl* impl = reinterpret_cast<AtlasGeneratorImpl*>(gen);
		if (!impl || !impl->ft_handle) {
			return -1;
		}

		msdfgen::FontHandle* font = msdfgen::loadFont(impl->ft_handle, font_path);
		if (!font) {
			impl->error_message = "Failed to load font: ";
			impl->error_message += font_path;
			return -2;
		}

		Charset* charset = getSharedCharset();

		GeneratorAttributes attributes;
		attributes.config.overlapSupport = true;
		attributes.scanlinePass = true;

		std::vector<GlyphGeometry> glyphs;
		FontGeometry fontGeometry(&glyphs);
		int glyphsLoaded = fontGeometry.loadCharset(font, 1.0, *charset);

		if (glyphsLoaded == 0) {
			impl->error_message = "No glyphs loaded from font";
			msdfgen::destroyFont(font);
			return -3;
		}

		fontGeometry.loadKerning(font);
		applyEdgeColoringToGlyphs(glyphs);

		TightAtlasPacker packer;
		packer.setPixelRange(pixel_range);
		packer.setMiterLimit(1.0);
		packer.setSpacing(1);
		packer.setScale(font_size);
		packer.pack(&glyphs[0], glyphs.size());

		int width = 0, height = 0;
		packer.getDimensions(width, height);

		ImmediateAtlasGenerator<
			float, 4, 
			mtsdfGenerator, 
			BitmapAtlasStorage<float, 4>
				> generator(width, height);

		generator.setAttributes(attributes);
		generator.setThreadCount(4);
		generator.generate(&glyphs[0], glyphs.size());

		msdfgen::BitmapConstRef<float, 4> floatBitmap = generator.atlasStorage();
		msdfgen::Bitmap<unsigned char, 4> byteBitmap = 
			convertFloatBitmapToByte(floatBitmap, width, height);

		if (!msdfgen::savePng(byteBitmap, png_path)) {
			impl->error_message = "Failed to save PNG";
			msdfgen::destroyFont(font);
			return -4;
		}

		JsonAtlasMetrics metrics = {};
		metrics.size = font_size;
		metrics.width = width;
		metrics.height = height;
		metrics.yDirection = YDirection::BOTTOM_UP;
		metrics.distanceRange = msdfgen::Range(pixel_range);

		if (!exportJSON(&fontGeometry, 1, ImageType::MTSDF, metrics, json_path, true)) {
			impl->error_message = "Failed to save JSON";
			msdfgen::destroyFont(font);
			return -5;
		}

		msdfgen::destroyFont(font);
		return 0;
	}

	const char* atlas_generator_get_error(struct msdf_atlas_gen* gen) {
		if (!gen) return "Invalid generator";
		AtlasGeneratorImpl* impl = reinterpret_cast<AtlasGeneratorImpl*>(gen);
		return impl->error_message.c_str();
	}

}
