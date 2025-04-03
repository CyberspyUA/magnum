/*
    This file is part of Magnum.

    Copyright © 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019,
                2020, 2021, 2022, 2023, 2024, 2025
              Vladimír Vondruš <mosra@centrum.cz>

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
*/

/* In order to have the CORRADE_PLUGIN_REGISTER() macro not a no-op. Doesn't
   affect anything else. */
#define CORRADE_STATIC_PLUGIN

#include <string>
#include <unordered_map>
#include <Corrade/Containers/Array.h>
#include <Corrade/Containers/GrowableArray.h>
#include <Corrade/Containers/Optional.h>
#include <Corrade/Containers/StridedArrayView.h>
#include <Corrade/Containers/StringView.h>
#include <Corrade/Containers/StringStl.h> /** @todo remove once file callbacks are <string>-free */
#include <Corrade/Containers/Triple.h>
#include <Corrade/PluginManager/Manager.h>
#include <Corrade/Utility/Algorithms.h>
#include <Corrade/Utility/Path.h>
#include <Corrade/Utility/Resource.h>

#include "Magnum/FileCallback.h"
#include "Magnum/Image.h"
#include "Magnum/ImageView.h"
#include "Magnum/PixelFormat.h"
#include "Magnum/Math/Color.h"
#include "Magnum/Math/Matrix3.h"
#include "Magnum/Math/Range.h"
#include "Magnum/Text/AbstractFont.h"
#include "Magnum/Text/AbstractFontConverter.h"
#include "Magnum/Text/AbstractGlyphCache.h"
#include "Magnum/Text/AbstractShaper.h"
#include "Magnum/Text/Direction.h"
#include "Magnum/Text/Feature.h"
#include "Magnum/Text/Script.h"
#include "Magnum/TextureTools/Atlas.h"

#define DOXYGEN_ELLIPSIS(...) __VA_ARGS__

using namespace Magnum;
using namespace Magnum::Math::Literals;

namespace MyNamespace {

struct MyFont: Text::AbstractFont {
    explicit MyFont(PluginManager::AbstractManager& manager, Containers::StringView plugin): Text::AbstractFont{manager, plugin} {}

    Text::FontFeatures doFeatures() const override { return {}; }
    bool doIsOpened() const override { return false; }
    void doClose() override {}
    void doGlyphIdsInto(const Containers::StridedArrayView1D<const char32_t>&, const Containers::StridedArrayView1D<UnsignedInt>&) override {}
    Vector2 doGlyphSize(UnsignedInt) override { return {}; }
    Vector2 doGlyphAdvance(UnsignedInt) override { return {}; }
    Containers::Pointer<Text::AbstractShaper> doCreateShaper() override { return nullptr; }
};
struct MyFontConverter: Text::AbstractFontConverter {
    explicit MyFontConverter(PluginManager::AbstractManager& manager, Containers::StringView plugin): Text::AbstractFontConverter{manager, plugin} {}

    Text::FontConverterFeatures doFeatures() const override { return {}; }
};

}

/* [MAGNUM_TEXT_ABSTRACTFONT_PLUGIN_INTERFACE] */
CORRADE_PLUGIN_REGISTER(MyFont, MyNamespace::MyFont,
    MAGNUM_TEXT_ABSTRACTFONT_PLUGIN_INTERFACE)
/* [MAGNUM_TEXT_ABSTRACTFONT_PLUGIN_INTERFACE] */

/* [MAGNUM_TEXT_ABSTRACTFONTCONVERTER_PLUGIN_INTERFACE] */
CORRADE_PLUGIN_REGISTER(MyFontConverter, MyNamespace::MyFontConverter,
    MAGNUM_TEXT_ABSTRACTFONTCONVERTER_PLUGIN_INTERFACE)
/* [MAGNUM_TEXT_ABSTRACTFONTCONVERTER_PLUGIN_INTERFACE] */

/* Make sure the name doesn't conflict with any other snippets to avoid linker
   warnings, unlike with `int main()` there now has to be a declaration to
   avoid -Wmisssing-prototypes */
void mainText();
void mainText() {
{
PluginManager::Manager<Text::AbstractFont> manager;
Containers::Pointer<Text::AbstractFont> font =
    manager.loadAndInstantiate("StbTrueTypeFont");
struct: Text::AbstractGlyphCache {
    using Text::AbstractGlyphCache::AbstractGlyphCache;

    Text::GlyphCacheFeatures doFeatures() const override { return {}; }
} cache{PixelFormat::R8Unorm, Vector2i{256}};
/* [AbstractFont-glyph-cache-by-id] */
CORRADE_INTERNAL_ASSERT_OUTPUT(font->fillGlyphCache(cache, {
    font->glyphForName("fi"),
    font->glyphForName("f_f"),
    font->glyphForName("fl"),
    DOXYGEN_ELLIPSIS()
}));
/* [AbstractFont-glyph-cache-by-id] */

/* [AbstractFont-glyph-cache-all] */
Containers::Array<UnsignedInt> glyphs{NoInit, font->glyphCount()};
for(UnsignedInt i = 0; i != font->glyphCount(); ++i)
    glyphs[i] = i;

CORRADE_INTERNAL_ASSERT_OUTPUT(font->fillGlyphCache(cache, glyphs));
/* [AbstractFont-glyph-cache-all] */
}

{
PluginManager::Manager<Text::AbstractFont> manager;
Containers::Pointer<Text::AbstractFont> font =
    manager.loadAndInstantiate("StbTrueTypeFont");
/* [AbstractFont-usage-data] */
Utility::Resource rs{"data"};
Containers::ArrayView<const char> data = rs.getRaw("font.ttf");
if(!font->openData(data, 12.0f))
    Fatal{} << "Can't open font data with StbTrueTypeFont";
/* [AbstractFont-usage-data] */
}

#if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
{
/* -Wnonnull in GCC 11+  "helpfully" says "this is null" if I don't initialize
   the font pointer. I don't care, I just want you to check compilation errors,
   not more! */
PluginManager::Manager<Text::AbstractFont> manager;
Containers::Pointer<Text::AbstractFont> font = manager.loadAndInstantiate("SomethingWhatever");
/* [AbstractFont-usage-callbacks] */
struct Data {
    std::unordered_map<std::string, Containers::Optional<
        Containers::Array<const char, Utility::Path::MapDeleter>>> files;
} data;

font->setFileCallback([](const std::string& filename,
    InputFileCallbackPolicy policy, Data& data)
        -> Containers::Optional<Containers::ArrayView<const char>>
    {
        auto found = data.files.find(filename);

        /* Discard the memory mapping, if not needed anymore */
        if(policy == InputFileCallbackPolicy::Close) {
            if(found != data.files.end()) data.files.erase(found);
            return {};
        }

        /* Load if not there yet. If the mapping fails, remember that to not
           attempt to load the same file again next time. */
        if(found == data.files.end()) found = data.files.emplace(
            filename, Utility::Path::mapRead(filename)).first;

        if(!found->second) return {};
        return Containers::arrayView(*found->second);
    }, data);

font->openFile("magnum-font.conf", 13.0f);
/* [AbstractFont-usage-callbacks] */
}
#endif

{
/* -Wnonnull in GCC 11+  "helpfully" says "this is null" if I don't initialize
   the font pointer. I don't care, I just want you to check compilation errors,
   not more! */
PluginManager::Manager<Text::AbstractFont> manager;
Containers::Pointer<Text::AbstractFont> font = manager.loadAndInstantiate("SomethingWhatever");
/* [AbstractFont-setFileCallback] */
font->setFileCallback([](const std::string& filename,
    InputFileCallbackPolicy, void*) {
        Utility::Resource rs{"data"};
        return Containers::optional(rs.getRaw(filename));
    });
/* [AbstractFont-setFileCallback] */
}

{
/* -Wnonnull in GCC 11+  "helpfully" says "this is null" if I don't initialize
   the font pointer. I don't care, I just want you to check compilation errors,
   not more! */
PluginManager::Manager<Text::AbstractFont> manager;
Containers::Pointer<Text::AbstractFont> font = manager.loadAndInstantiate("SomethingWhatever");
/* [AbstractFont-setFileCallback-template] */
const Utility::Resource rs{"data"};
font->setFileCallback([](const std::string& filename,
    InputFileCallbackPolicy, const Utility::Resource& rs) {
        return Containers::optional(rs.getRaw(filename));
    }, rs);
/* [AbstractFont-setFileCallback-template] */
}

{
/* -Wnonnull in GCC 11+  "helpfully" says "this is null" if I don't initialize
   the font pointer. I don't care, I just want you to check compilation errors,
   not more! */
PluginManager::Manager<Text::AbstractFont> manager;
struct: Text::AbstractGlyphCache {
    using Text::AbstractGlyphCache::AbstractGlyphCache;

    Text::GlyphCacheFeatures doFeatures() const override { return {}; }
} cache{PixelFormat::R8Unorm, Vector2i{256}};
/* [AbstractGlyphCache-usage-fill] */
Containers::Pointer<Text::AbstractFont> font = DOXYGEN_ELLIPSIS(manager.loadAndInstantiate(""));

if(!font->fillGlyphCache(cache, "abcdefghijklmnopqrstuvwxyz"
                                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                "0123456789?!:;,. "))
    Fatal{} << "Glyph cache too small to fit all characters";
/* [AbstractGlyphCache-usage-fill] */
}

{
struct: Text::AbstractGlyphCache {
    using Text::AbstractGlyphCache::AbstractGlyphCache;

    Text::GlyphCacheFeatures doFeatures() const override { return {}; }
} cache{PixelFormat::R8Unorm, Vector2i{256}};
/* [AbstractGlyphCache-filling-images] */
Containers::Array<Image2D> images = DOXYGEN_ELLIPSIS({}); /* or ImageView2D, Trade::ImageData2D... */
/* [AbstractGlyphCache-filling-images] */

/* [AbstractGlyphCache-filling-font] */
UnsignedInt fontId = cache.addFont(images.size());
/* [AbstractGlyphCache-filling-font] */

/* [AbstractGlyphCache-filling-atlas] */
Containers::Array<Vector2i> offsets{NoInit, images.size()};

cache.atlas().clearFlags(
    TextureTools::AtlasLandfillFlag::RotatePortrait|
    TextureTools::AtlasLandfillFlag::RotateLandscape);
Containers::Optional<Range2Di> range = cache.atlas().add(
    stridedArrayView(images).slice(&Image2D::size),
    offsets);
CORRADE_INTERNAL_ASSERT(range);
/* [AbstractGlyphCache-filling-atlas] */

/* [AbstractGlyphCache-filling-glyphs] */
/* The glyph cache is just 2D, so copying to the first slice */
Containers::StridedArrayView3D<char> dst = cache.image().pixels()[0];
for(UnsignedInt i = 0; i != images.size(); ++i) {
    Range2Di rectangle = Range2Di::fromSize(offsets[i], images[i].size());
    cache.addGlyph(fontId, i, {}, rectangle);

    /* Copy assuming all input images have the same pixel format */
    Containers::StridedArrayView3D<const char> src = images[i].pixels();
    Utility::copy(src, dst.sliceSize({
        std::size_t(offsets[i].y()),
        std::size_t(offsets[i].x()),
        0}, src.size()));
}

/* Reflect the updated image range to the actual GPU-side texture */
cache.flushImage(*range);
/* [AbstractGlyphCache-filling-glyphs] */
}

{
struct: Text::AbstractGlyphCache {
    using Text::AbstractGlyphCache::AbstractGlyphCache;

    Text::GlyphCacheFeatures doFeatures() const override { return {}; }
} cacheInstance{PixelFormat::R8Unorm, Vector2i{256}};
/* [AbstractGlyphCache-querying] */
Containers::Pointer<Text::AbstractFont> font = DOXYGEN_ELLIPSIS({});
Text::AbstractGlyphCache& cache = DOXYGEN_ELLIPSIS(cacheInstance);

Containers::ArrayView<const UnsignedInt> fontGlyphIds = DOXYGEN_ELLIPSIS({});

Containers::Optional<UnsignedInt> fontId = cache.findFont(*font);
DOXYGEN_ELLIPSIS()
for(std::size_t i = 0; i != fontGlyphIds.size(); ++i) {
    Containers::Triple<Vector2i, Int, Range2Di> glyph =
        cache.glyph(*fontId, fontGlyphIds[i]);
    DOXYGEN_ELLIPSIS(static_cast<void>(glyph);)
}
/* [AbstractGlyphCache-querying] */

/* [AbstractGlyphCache-querying-batch] */
Containers::Array<UnsignedInt> glyphIds{NoInit, fontGlyphIds.size()};
cache.glyphIdsInto(*fontId, fontGlyphIds, glyphIds);

Containers::StridedArrayView1D<const Vector2i> offsets = cache.glyphOffsets();
Containers::StridedArrayView1D<const Range2Di> rects = cache.glyphRectangles();
for(std::size_t i = 0; i != fontGlyphIds.size(); ++i) {
    Vector2i offset = offsets[glyphIds[i]];
    Range2Di rectangle = rects[glyphIds[i]];
    DOXYGEN_ELLIPSIS(static_cast<void>(offset); static_cast<void>(rectangle);)
}
/* [AbstractGlyphCache-querying-batch] */
}

{
/* -Wnonnull in GCC 11+  "helpfully" says "this is null" if I don't initialize
   the font pointer. I don't care, I just want you to check compilation errors,
   not more! */
PluginManager::Manager<Text::AbstractFont> manager;
/* [AbstractShaper-shape] */
Containers::Pointer<Text::AbstractFont> font = DOXYGEN_ELLIPSIS(manager.loadAndInstantiate("SomethingWhatever"));
Containers::Pointer<Text::AbstractShaper> shaper = font->createShaper();

/* Set text properties and shape it */
shaper->setScript(Text::Script::Latin);
shaper->setDirection(Text::ShapeDirection::LeftToRight);
shaper->setLanguage("en");
shaper->shape("Hello, world!");

/* Get the glyph info back */
struct GlyphInfo {
    UnsignedInt id;
    Vector2 offset;
    Vector2 advance;
};
Containers::Array<GlyphInfo> glyphs{NoInit, shaper->glyphCount()};
shaper->glyphIdsInto(
    stridedArrayView(glyphs).slice(&GlyphInfo::id));
shaper->glyphOffsetsAdvancesInto(
    stridedArrayView(glyphs).slice(&GlyphInfo::offset),
    stridedArrayView(glyphs).slice(&GlyphInfo::advance));
/* [AbstractShaper-shape] */
}

{
/* -Wnonnull in GCC 11+  "helpfully" says "this is null" if I don't initialize
   the font pointer. I don't care, I just want you to check compilation errors,
   not more! */
PluginManager::Manager<Text::AbstractFont> manager;
Containers::Pointer<Text::AbstractFont> font = manager.loadAndInstantiate("SomethingWhatever");
Containers::Pointer<Text::AbstractShaper> shaper = font->createShaper();
/* [AbstractShaper-shape-features] */
shaper->shape("Hello, world!", {
    {Text::Feature::SmallCapitals, 7, 12}
});
/* [AbstractShaper-shape-features] */
}

{
struct GlyphInfo {
    UnsignedInt id;
    Vector2 offset;
    Vector2 advance;
};
/* -Wnonnull in GCC 11+  "helpfully" says "this is null" if I don't initialize
   the font pointer. I don't care, I just want you to check compilation errors,
   not more! */
PluginManager::Manager<Text::AbstractFont> manager;
/* [AbstractShaper-shape-multiple] */
Containers::Pointer<Text::AbstractFont> font = DOXYGEN_ELLIPSIS(manager.loadAndInstantiate("SomethingWhatever"));
Containers::Pointer<Text::AbstractFont> boldFont = DOXYGEN_ELLIPSIS(manager.loadAndInstantiate("SomethingWhatever"));
Containers::Pointer<Text::AbstractShaper> shaper = font->createShaper();
Containers::Pointer<Text::AbstractShaper> boldShaper = boldFont->createShaper();
DOXYGEN_ELLIPSIS()

Containers::Array<GlyphInfo> glyphs;

/* Shape "Hello, " with a regular font */
shaper->shape("Hello, world!", 0, 7);
Containers::StridedArrayView1D<GlyphInfo> glyphs1 =
    arrayAppend(glyphs, NoInit, shaper->glyphCount());
shaper->glyphIdsInto(
    glyphs1.slice(&GlyphInfo::id));
shaper->glyphOffsetsAdvancesInto(
    glyphs1.slice(&GlyphInfo::offset),
    glyphs1.slice(&GlyphInfo::advance));

/* Append "world" shaped with a bold font */
boldShaper->shape("Hello, world!", 7, 12);
Containers::StridedArrayView1D<GlyphInfo> glyphs2 =
    arrayAppend(glyphs, NoInit, boldShaper->glyphCount());
shaper->glyphIdsInto(
    glyphs2.slice(&GlyphInfo::id));
shaper->glyphOffsetsAdvancesInto(
    glyphs2.slice(&GlyphInfo::offset),
    glyphs2.slice(&GlyphInfo::advance));

/* Finally shape "!" with a regular font again */
shaper->shape("Hello, world!", 12, 13);
Containers::StridedArrayView1D<GlyphInfo> glyphs3 =
    arrayAppend(glyphs, NoInit, shaper->glyphCount());
shaper->glyphIdsInto(
    glyphs3.slice(&GlyphInfo::id));
shaper->glyphOffsetsAdvancesInto(
    glyphs3.slice(&GlyphInfo::offset),
    glyphs3.slice(&GlyphInfo::advance));
/* [AbstractShaper-shape-multiple] */
}

{
/* -Wnonnull in GCC 11+  "helpfully" says "this is null" if I don't initialize
   the font pointer. I don't care, I just want you to check compilation errors,
   not more! */
PluginManager::Manager<Text::AbstractFont> manager;
Containers::Pointer<Text::AbstractFont> font = manager.loadAndInstantiate("SomethingWhatever");
Containers::Pointer<Text::AbstractShaper> shaper = font->createShaper();
/* [AbstractShaper-shape-clusters] */
Containers::StringView text = DOXYGEN_ELLIPSIS({});

shaper->shape(text);
DOXYGEN_ELLIPSIS()

Containers::Array<UnsignedInt> clusters{NoInit, shaper->glyphCount()};
shaper->glyphClustersInto(clusters);

Containers::StringView selection = text.slice(clusters[2], clusters[5]);
/* [AbstractShaper-shape-clusters] */
static_cast<void>(selection);
}

}
