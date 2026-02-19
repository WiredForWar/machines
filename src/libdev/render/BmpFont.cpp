/*
 * B M P F O N T . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "render/BmpFont.hpp"
#include "render/Painter.hpp"
#include "render/internal/BmpFontCore.hpp"
#include "render/surface.hpp"
#include "base/diag.hpp"
#include "base/persist.hpp"

PER_DEFINE_PERSISTENT(BmpFontCoreCharData);

BmpFontCore::BmpFontCore(const SysPathName& fontPath)
{
    fontBmp_ = RenSurface::createSharedSurface(fontPath.pathname());
    fontBmp_.enableColourKeying();
    fontPath_ = fontPath;
    coreCount_ = 1;
    charHeight_ = fontBmp_.height() - 1 /*bitmap has extra undisplayable line at bottom*/;

    SysPathName persistFontPath = fontPath;
    persistFontPath.extension("bin");
    if (persistFontPath.existsAsFile())
    {
        createFromBinaryFile(persistFontPath);
    }
    else
    {
        calculateProportionalFontWidthData(persistFontPath);
    }
}

void BmpFontCore::createFromBinaryFile(const SysPathName& persistFontPath)
{
    DEBUG_STREAM(DIAG_NEIL, "BmpFontCore::createFromBinaryFile " << persistFontPath << std::endl);
    std::ifstream str(persistFontPath.c_str(), std::ios::binary);
    PerIstream istr(str);

    istr >> charData_;
    istr >> maxCharWidth_;
    DEBUG_STREAM(DIAG_NEIL, "BmpFontCore::createFromBinaryFile exit maxCharWidth_ " << maxCharWidth_ << std::endl);
}

void BmpFontCore::calculateProportionalFontWidthData(const SysPathName& persistFontPath)
{
    int bmpXPos = 0;
    maxCharWidth_ = 0;

    while (bmpXPos < fontBmp_.width())
    {
        int startXPos = bmpXPos;

        while (bmpXPos < fontBmp_.width() && !endOfChar(bmpXPos))
        {
            ++bmpXPos;
        }

        BmpFontCoreCharData newCharData;
        newCharData.width_ = bmpXPos - startXPos;
        // If this is widest character then update maxCharWidth_
        maxCharWidth_ = std::max(maxCharWidth_, newCharData.width_);
        newCharData.offset_ = startXPos;
        charData_.push_back(newCharData);

        ++bmpXPos;
    }

    std::ofstream str(persistFontPath.c_str(), std::ios::binary);
    PerOstream ostr(str);

    ostr << charData_;
    ostr << maxCharWidth_;

    DEBUG_STREAM(
        DIAG_NEIL,
        "BmpFontCore::calculateProportionalFontWidthData ( num chars : " << charData_.size() << " ) maxCharWidth_ "
                                                                         << maxCharWidth_ << std::endl);
}

bool BmpFontCore::endOfChar(int x)
{
    RenColour colour;
    static const RenColour white = RenColour::white();;
    int height = fontBmp_.height();

    fontBmp_.getPixel(x, height - 1, &colour);

    return colour.operator==(white);
}

namespace Ren
{

const SysPathName& BmpFont::fontPath() const
{
    return pFontCore_->fontPath_;
}

char BmpFont::arrowUpIndex()
{
    return 0x8D;
}

char BmpFont::arrowDownIndex()
{
    return 0x8E;
}

char BmpFont::arrowLeftIndex()
{
    return 0x8F;
}

char BmpFont::arrowRightIndex()
{
    return 0x90;
}

char BmpFont::healthPointsIndex()
{
    return 0x9D;
}

char BmpFont::armorPointsIndex()
{
    return 0x80;
}

char BmpFont::bmuPointsIndex()
{
    return 0x81;
}

char BmpFont::bmuMinedPointsIndex()
{
    return 0x99;
}

char BmpFont::researchPointsIndex()
{
    return 0x9E;
}

char BmpFont::redCharIndex()
{
    return 0xA9;
}

char BmpFont::greenCharIndex()
{
    return 0xAA;
}

char BmpFont::blueCharIndex()
{
    return 0xAB;
}

char BmpFont::yellowCharIndex()
{
    return 0xAC;
}

BmpFont::BmpFont()
{
}

BmpFont::BmpFont(const SysPathName& fontPath)
{
    pFontCore_ = new BmpFontCore(fontPath);

    TEST_INVARIANT;
}

BmpFont::~BmpFont()
{
    TEST_INVARIANT;

    if (!pFontCore_)
        return;

    --pFontCore_->coreCount_;
    if (pFontCore_->coreCount_ == 0)
    {
        delete pFontCore_;
    }
}

BmpFont::BmpFont(const BmpFont& copy)
{
    pFontCore_ = copy.pFontCore_;
    fontType_ = copy.fontType_;
    spaceCharWidth_ = copy.spaceCharWidth_;
    spacing_ = copy.spacing_;
    underlineColour_ = copy.underlineColour_;
    underline_ = copy.underline_;

    if (pFontCore_)
    {
        ++pFontCore_->coreCount_;
    }
}

BmpFont& BmpFont::operator=(const BmpFont& rhs)
{
    if (this != &rhs)
    {
        pFontCore_ = rhs.pFontCore_;
        fontType_ = rhs.fontType_;
        spaceCharWidth_ = rhs.spaceCharWidth_;
        spacing_ = rhs.spacing_;
        underlineColour_ = rhs.underlineColour_;
        underline_ = rhs.underline_;

        if (pFontCore_)
        {
            ++pFontCore_->coreCount_;
        }
    }

    return *this;
}

void BmpFont::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const BmpFont& t)
{

    o << "BmpFont " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "BmpFont " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

size_t BmpFont::height() const
{
    return pFontCore_->charHeight_;
}

size_t BmpFont::charWidth(char c) const
{
    if (c == ' ')
    {
        return spaceCharWidth_;
    }
    unsigned int index = (unsigned char)c;
    if (pFontCore_->charData_.size() > index)
    {
        return pFontCore_->charData_[index].width_;
    }
    return 0;
}

size_t BmpFont::maxCharWidth() const
{
    return pFontCore_->maxCharWidth_;
}

const RenSurface& BmpFont::fontBitmap() const
{
    return pFontCore_->fontBmp_;
}

size_t BmpFont::charOffset(unsigned char c) const
{
    if (c < pFontCore_->charData_.size())
        return pFontCore_->charData_[c].offset_;
    return 0;
}

const RenColour& BmpFont::underlineColour() const
{
    return underlineColour_;
}

BmpFont::FontType BmpFont::fontType() const
{
    return fontType_;
}

void BmpFont::fontType(BmpFont::FontType fontType)
{
    fontType_ = fontType;
}

size_t BmpFont::spaceCharWidth() const
{
    return spaceCharWidth_;
}

void BmpFont::spaceCharWidth(size_t spaceCharWidth)
{
    spaceCharWidth_ = spaceCharWidth;
}

size_t BmpFont::spacing() const
{
    return spacing_;
}

void BmpFont::spacing(size_t spacing)
{
    spacing_ = spacing;
}

void BmpFont::underline(bool underl)
{
    underline_ = underl;
}

bool BmpFont::underline() const
{
    return underline_;
}

void BmpFont::underlineColour(RenColour colour)
{
    underlineColour_ = colour;
}

int BmpFont::horizontalAdvance(const std::string_view& text) const
{
    int xPos = 0;

    for (int i = 0; i < text.length(); ++i)
    {
        if (text[i] == ' ') // Special handling for space character
        {
            xPos += spaceCharWidth_ + spacing_;
        }
        else if (charWidth(text[i])) // Check that character is supported by bitmap
        {
            // Add character width and spacing width
            if (fontType_ == PROPORTIONAL)
                xPos += charWidth(text[i]) + spacing_;
            else
                xPos += maxCharWidth() + spacing_;
        }
    }

    return xPos;
}

} // namespace Ren

void perWrite(PerOstream& ostr, const BmpFontCoreCharData& data)
{
    ostr << data.offset_;
    ostr << data.width_;
}

void perRead(PerIstream& istr, BmpFontCoreCharData& data)
{
    istr >> data.offset_;
    istr >> data.width_;
}

/* End BMPFONT.CPP *****************************************************/
